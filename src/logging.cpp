/*
 * logging.cpp
 *
 *  Created on: Jun 29, 2018
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

#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include "logging.h"

using namespace std;

namespace oemros {

#ifndef LOGGING_ENGINE_CREATE
#define LOGGING_ENGINE_CREATE new logging()
#endif

static logging * get_engine(void) {
    // C++ gurantees thread safe static initialization
    static oemros::logging *log_singleton = LOGGING_ENGINE_CREATE;
    assert(log_singleton != NULL);
    return log_singleton;
}

void logging_start(void) {
    get_engine()->start();
}

void logging_cleanup(void) {
    auto logging = get_engine();
    assert(logging != NULL);
    delete logging;
}

loglevel logging_get_level(void) {
    return get_engine()->current_level();
}

loglevel logging_set_level(loglevel new_level) {
    return get_engine()->current_level(new_level);
}

bool logging_should_log(loglevel level) {
    return(level >= get_engine()->current_level());
}

void logging_add_destination(shared_ptr<logdest> destination) {
    get_engine()->add_destination(destination);
}

void logging_input_event(const logevent& event) {
    get_engine()->input_event(event);
}

const char * logging_level_name(loglevel level) {
    return get_engine()->level_name(level);
}

const char * logging_source_name(logsource source) {
    return get_engine()->source_name(source);
}

logevent::logevent(logsource source, loglevel level, const struct timeval timestamp, std::thread::id tid, const char *function, const char *path, const int line, string message)
: source(source), level(level), timestamp(timestamp), tid(tid), function(function), path(path), line(line), message(message)
{ };

logging::logging(void) {
    const char* env_value = getenv("OEMROS_TRACE");
    loglevel_t level = loglevel_t::info;

    if (env_value != NULL) {
        level = loglevel_t::trace;
    }

    log_threshold = level;
}

std::unique_lock<std::shared_timed_mutex> logging::get_lockex(void) {
    return std::unique_lock<std::shared_timed_mutex>(log_mutex);
}

std::shared_lock<std::shared_timed_mutex> logging::get_locksh(void) {
    return std::shared_lock<std::shared_timed_mutex>(log_mutex);
}

// THREAD this entire method needs exclusive access
void logging::start(void) {
    auto lock = get_lockex();
    size_t delivered = 0;

    // if start was called and no destination was ever specified
    // then a default one is setup
    if (destinations.size() == 0) {
        destinations.push_back(std::make_shared<logconsole>());
    }

    if (event_buffer.size() > 0) {
        for(auto&& i : event_buffer) {
            deliver_event(i);
            delivered++;
        }
    }

    deliver_events = true;

    assert(event_buffer.size() == delivered);
    event_buffer.clear();
}

// THREAD this method relies on atomic variables
loglevel logging::current_level(void) {
    return log_threshold;
}

// THREAD this method relies on atomic variables
loglevel logging::current_level(loglevel new_level) {
    loglevel old_level = log_threshold;
    // FIXME this could probably use a compare-and-swap since it is not locked
    log_threshold = new_level;
    return old_level;
}

// THREAD this method is inherently thread safe
const char * logging::level_name(loglevel level) {
    switch (level) {
    case loglevel::fatal:
        return "FATAL";
    case loglevel::error:
        return "ERROR";
    case loglevel::warn:
        return "WARN";
    case loglevel::notice:
        return "NOTICE";
    case loglevel::info:
        return "INFO";
    case loglevel::verbose:
        return "VERBOSE";
    case loglevel::debug:
        return "DEBUG";
    case loglevel::lots:
        return "LOTS";
    case loglevel::trace:
        return "TRACE";
    case loglevel::unknown:
        return "UNKNOWNLEVEL";
    }

    log_fatal("switch() failed for loglevel enum: ", (int)level);
}


// THREAD this method is inherently thread safe
const char * logging::source_name(logsource source) {
    switch (source) {
    case logsource::unknown:
        return "UNKNOWNSOURCE";
    case logsource::oemros:
        return "oemros";
    case logsource::hamlib:
        return "hamlib";
    }

    log_fatal("switch() failed for logsource enum: ", (int)source);
}

// THREAD private method only called by methods already holding a lock
void logging::deliver_event(const logevent& event) {
    for (auto&& i : destinations) {
        i->event(event);
    }
}

// THREAD starts off with no locking then acquires a shared lock and
// THREAD might restart with an exclusive lock
void logging::input_event(const logevent& event) {
    if (event.level == loglevel::unknown) {
        system_panic("attempt to log an event with a level of unknown");
    }

    // atomic data type means this does not need to be locked
    if (log_threshold == loglevel::unknown) {
        return;
    } else if(event.level < log_threshold) {
        return;
    }

    // this function starts off with a shared lock then restarts
    // with an exclusive lock if needed
    bool shared = true;
    std::shared_lock<std::shared_timed_mutex> shared_lock;
    std::unique_lock<std::shared_timed_mutex> exclusive_lock;

restart:

    if (shared) {
        shared_lock = get_locksh();
    } else {
        shared_lock.unlock();
        exclusive_lock = get_lockex();
    }

    // event delivery and checking for buffering are ok with the shared lock
    // and with the exclusive lock though the preferred case is to handle
    // delivery with the shared lock - delivering on an exclusive lock
    // will be limited to edge cases where this restarts immediately after
    // moving from buffering events to delivery of events otherwise the
    // exclusive lock is used to put events into the buffer
    if(deliver_events) {
        deliver_event(event);
    } else if (buffer_events) {
        if (shared) {
            shared = false;
            goto restart;
        }
        // FIXME if this was a lockless queue then the locking requirements
        // for everything else would be satisfied with only a shared (not exclusive)
        // lock and this logic could be much more simple and the edge case removed
        event_buffer.push_back(event);
    }
}

// THREAD this method needs exclusive access
void logging::add_destination(shared_ptr<logdest> destination) {
    auto lock = get_lockex();
    destinations.push_back(destination);
}

void logdest::event(const logevent& event) {
    string formatted = format_event(event);
    output(event, formatted);
}

string logdest::format_time(const struct timeval& when) {
    struct tm tmbuf;
    struct tm* timevals = gmtime_r(&when.tv_sec, &tmbuf);
    char buf[LOGGING_TIMESTR_BUFLEN];

    if (timevals == NULL) {
        // FIXME ignores errno
        system_panic("gmtime_r() failed");
    }

    int result = snprintf(
            buf, LOGGING_TIMESTR_BUFLEN,
            "%02d:%02d:%02d.%06ld",
            timevals->tm_hour, timevals->tm_min,
            timevals->tm_sec, (long)when.tv_usec);

    if (result >= LOGGING_TIMESTR_BUFLEN) {
        // FIXME this ignores errno
        system_panic("formatted time string was truncated");
    }

    return string(buf);
}

string logdest::format_event(const logevent& event) {

    stringstream buffer;
    buffer << format_time(event.timestamp) << " ";
    buffer << event.tid << " ";
    buffer << logging_source_name(event.source) << " ";
    buffer << logging_level_name(event.level) << " ";
    buffer << event.path << ":" << event.line << " ";
    buffer << event.function << ": " << endl;
    buffer << "  " << event.message;

    return buffer.str();
}

void logdest::output(const logevent& event, const string formatted) {
    std::lock_guard<std::mutex> lock(output_mutex);
    output__child(event, formatted);
}

void logstdio::output__child(const logevent& event, const string formatted) {
    if (event.level >= loglevel::warn) {
        cerr << formatted << endl;
    } else {
        cout << formatted << endl;
    }
}

std::string logconsole::format_event(const logevent& event) {
    stringstream buffer;

    if (event.level <= loglevel::debug) {
        return logdest::format_event(event);
    } else if ((event.level >= loglevel::verbose) && (event.level <= loglevel::warn)) {
        buffer << logging_level_name(event.level) << " " << event.message;
    } else {
        return logdest::format_event(event);
    }

    return buffer.str();
}

logfile::logfile(const char *path) {
    outfile.open(path, ofstream::app);

    if (outfile.fail()) {
        log_fatal("could not open ", path, " for write: ", errstream);
    }
}

void logfile::output__child(const logevent& /* event */, const string formatted) {
    outfile << formatted << endl;
}

}
