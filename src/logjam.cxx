/*
 * logjam.cxx
 *
 *  Created on: Jul 21, 2018
 *      Author: tyler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <atomic>
#include <cassert>
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>

#include "logjam.h"

namespace logjam {

static std::atomic<loglevel> log_level_min = ATOMIC_VAR_INIT(loglevel::none);

const char* level_name(const loglevel& level_in) {
    switch (level_in) {
        case loglevel::uninit: return "(uninitialized)";
        case loglevel::none: return "(none)";
        case loglevel::unknown: return "unknown";
        case loglevel::trace: return "trace";
        case loglevel::debug: return "debug";
        case loglevel::verbose: return "verbose";
        case loglevel::info: return "info";
        case loglevel::error: return "error";
        case loglevel::fatal: return "fatal";
    }

    throw std::runtime_error("switch() failure");
}

void mutex::lock() {
    std::mutex::lock();
    // FIXME how do I check for no value?
    // assert(owned_by == ??????);
    owned_by = std::this_thread::get_id();
}

// FIXME is this right?
void mutex::unlock() {
    assert(owned_by == std::this_thread::get_id());
    owned_by = std::thread::id();
    std::mutex::unlock();
}

bool mutex::caller_has_exclusive() {
    return owned_by == std::this_thread::get_id();
}

// THREAD this is not yet thread safe
static loglevel get_min_level() {
    // FIXME this should use memory ordering probably
    return log_level_min.load();
}

// THREAD this is not yet thread safe
static loglevel set_min_level(const loglevel& level_in) {
    loglevel old = get_min_level();
    // FIXME this needs a compare and swap
    log_level_min = level_in;
    return old;
}

static bool logsource_compare(const char* rhs, const char* lhs) {
    // two pointers to the same string must match so check for that
    if (rhs == lhs) {
        return true;
    }

    // otherwise do a normal string compare
    return std::strcmp(rhs, lhs) == 0;
}

logsource::logsource(const char* c_str_in) : c_str(c_str_in) {
    assert(c_str != nullptr);
}

bool logsource::operator==(const char* rhs) const {
    return logsource_compare(c_str, rhs);
}

bool logsource::operator==(const logsource& rhs) const {
    return logsource_compare(c_str, rhs.c_str);
}

logevent::logevent(const logsource& source_in, const loglevel& level_in, const timestamp& when_in, const std::thread::id& tid_in, const char* function_in, const char *file_in, const int& line_in, const std::string& message_in)
: category(source_in.c_str), level(level_in), when(when_in), tid(tid_in), function(function_in), file(file_in), line(line_in), message(message_in) {
    assert(level >= loglevel::unknown);
}

// THREAD this function is inherently thread safe
logengine* logengine::get_engine() {
    auto user_engine = handlers::get_engine();
    assert(user_engine != nullptr);
    return user_engine;
}

std::unique_lock<logjam::mutex> logengine::get_lockex() {
    return std::unique_lock<logjam::mutex>(mutex);
}

// attempts to add a destination more than once silently return
// THREAD this function needs exclusive locking
void logengine::add_destination(const std::shared_ptr<logdest>& destination_in) {
    auto lock = get_lockex();

    assert(mutex.caller_has_exclusive());

    for (auto&& i : destinations) {
        if (i->id == destination_in->id) {
            return;
        }
    }

    destination_in->engine = this;
    destinations.push_back(destination_in);

    update_min_level();
}

// THREAD this is not yet thread safe
void logengine::update_min_level() {
//    assert(mutex.caller_has_exclusive());

    int max_so_far = (int)loglevel::uninit;

    for (auto&& i : destinations) {
        if ((int)i->min_level > max_so_far) {
            max_so_far = (int)i->min_level;
        }
    }

    if (max_so_far == (int)loglevel::uninit) {
        return;
    } else if (max_so_far != (int)get_min_level()) {
        set_min_level(loglevel(max_so_far));
    }
}

// THREAD this function is inherently thread safe
bool logengine::should_log(const loglevel& level_in) {
    assert(log_level_min != loglevel::uninit);
    // always pass in log events if they are fatal so
    // the engine can terminate the process
    if (level_in == loglevel::fatal) return true;
    return level_in >= log_level_min;
}

// THREAD this is not yet thread safe
void logengine::start() {

}

// THREAD this function needs shared locking if the log
// event queue uses lockless insertion
// https://en.cppreference.com/w/cpp/atomic/atomic_compare_exchange
void logengine::deliver(const logevent& event_in) const {
    // this function must not return if a log event came in with
    // a level of fatal

//    assert(event_in.level != loglevel::uninit);

    if (should_log(event_in.level)) {
        for(auto&& i : destinations) {
            i->output(event_in);
        }
    }

    if (event_in.level == loglevel::fatal) {
        handlers::fatal(event_in);
        // should never get here - if so the user defined terminate
        // handler didn't work
        std::terminate();
    }

    // it is now ok to return after here
    return;
}

logdest::logdest(const loglevel& min_level_in) : min_level(min_level_in) { }

// THREAD this function is inherently thread safe
logdest::destid logdest::next_destination_id() {
    static std::atomic<logdest::destid> last_dest_id = ATOMIC_VAR_INIT(0);
    return ++last_dest_id;
}

// THREAD this is not yet thread safe
loglevel logdest::set_min_level(const loglevel& min_level_in) {
    loglevel old = min_level;
    min_level = min_level_in;

    if (engine != nullptr) {
        engine->update_min_level();
    }

    return old;
}

// THREAD this function is inherently thread safe
void logdest::output(const logevent& event_in) {
    handle_output(event_in);
}

std::string logconsole::format_event(const logevent& event_in) {
    std::stringstream buf;

    buf << "@" << event_in.category << "." << level_name(event_in.level) << " ";
    buf << event_in.function << ": " << event_in.message;

    return buf.str();
}

// THREAD this function needs exclusive access to STDIO so
// threads don't output on top of each other
void logconsole::handle_output(const logevent& event_in) {
    std::cout << format_event(event_in) << std::endl;
}

}
