/*
 * logjam.h
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

#pragma once

#include <chrono>
#include <ctime>
#include <memory>
#include <shared_mutex>
#include <sstream>
#include <thread>
#include <vector>

#ifdef LOGJAM_LOGSOURCE_MACRO
// TODO figure out why name(#name) doesn't work
#define LOGJAM_LOGSOURCE(name) const logjam::logsource name{#name}
#endif

namespace logjam {

struct logevent;
class logengine;

// functions that the user of the library needs to provide
struct handlers {
        static void fatal(const logevent& event_in);
        static logengine* get_engine();
};

class mutex : public std::mutex {
    private:
        std::thread::id owned_by;

    public:
        void lock();
        void unlock();
        bool caller_has_exclusive();
};

enum class loglevel {
    uninit = -2,
    none = -1,
    unknown = 0,
    trace = 10,
    debug = 30,
    verbose = 35,
    info = 40,
    error = 80,
    fatal = 100,
};

struct logsource {
    const char* c_str;
    logsource(const char* c_str_in);
    bool operator==(const char* rhs) const;
    bool operator==(const logsource& rhs) const;
};

struct baseobj {
    baseobj(const baseobj&) = delete;
    baseobj(const baseobj&&) = delete;
    baseobj& operator=(const baseobj&);

    baseobj() = default;
    ~baseobj() = default;
};

// all the members of a logevent are const for thread safety
struct logevent : public baseobj {
    using timestamp = std::chrono::time_point<std::chrono::system_clock>;

    const char* category = nullptr;
    const loglevel level = loglevel::uninit;
    const timestamp when;
    const std::thread::id tid;
    const char *function = nullptr;
    const char *file = nullptr;
    const int32_t line = -1;
    const std::string message;

    logevent(const logsource& source_in, const loglevel& level_in, const timestamp& when, const std::thread::id& tid_in, const char* function, const char *file, const int& line, const std::string& message_in);
    ~logevent() = default;
};

class logdest;

class logengine : public baseobj {
    friend logengine* handlers::get_engine();

    private:
        logjam::mutex mutex;
        std::vector<std::shared_ptr<logdest>> destinations;
        logengine() = default;
        std::unique_lock<logjam::mutex> get_lockex();

    public:
        // get the singleton instance
        static logengine* get_engine();
        void update_min_level(void);
        void add_destination(const std::shared_ptr<logdest>& destination_in);
        static bool should_log(const loglevel& level_in);
        void deliver(const logevent& event) const;
        void start();
};

class logdest : public baseobj {
    friend class logengine;
    using destid = uint32_t;

    private:
        loglevel min_level;
        static destid next_destination_id();

    protected:
        logengine* engine = nullptr;
        virtual void handle_output(const logevent& event) = 0;

    public:
        const destid id = logdest::next_destination_id();
        logdest(const loglevel& min_level_in);
        virtual ~logdest() = default;
        loglevel get_min_level();
        loglevel set_min_level(const loglevel& min_level_in);
        void output(const logevent& event_in);
};

class logconsole : public logdest {
    using lock = std::unique_lock<std::mutex>;

    private:
        std::mutex mutex;
        lock get_lock();
        virtual void handle_output(const logevent& event_in) override;

    public:
        logconsole(const loglevel& level_in = loglevel::debug)
            : logdest(level_in) { }
        virtual ~logconsole() = default;
        virtual std::string format_event(const logevent& event);
};

const char* level_name(const loglevel& level_in);

template <typename T>
void accumulate_log_arg(std::stringstream& sstream, T t) {
    sstream << t;
}

template<typename T, typename... Args>
void accumulate_log_arg(std::stringstream& sstream, T t, Args... args) {
    log__accumulate_value(sstream, t);
    log__accumulate_value(sstream, args...);
}

template <typename T>
void send_logevent(logsource source, loglevel level, const char *function, const char *path, int line, T t) {
    if (logengine::should_log(level)) {
        auto when = std::chrono::system_clock::now();

        std::stringstream sstream;
        accumulate_log_arg(sstream, t);

        auto tid = std::this_thread::get_id();
        logevent event(source, level, when, tid, function, path, line, sstream.str());
        logengine::get_engine()->deliver(event);
    }
}

template<typename T, typename... Args>
void send_logevent(logsource source, loglevel level, const char *function, const char *path, int line, T t, Args... args) {
    if (logengine::should_log(level)) {
        auto when = std::chrono::system_clock::now();

        std::stringstream sstream;
        accumulate_log_arg(sstream, t);
        accumulate_log_arg(sstream, args...);

        auto tid = std::this_thread::get_id();
        logevent event(source, level, when, tid, function, path, line, sstream.str());
        logengine::get_engine()->deliver(event);
    }
}


// fatal variants of the log functions are used because they need to be marked no return
template <typename T>
[[ noreturn ]] void send_logevent_fatal(const logsource& source, const loglevel& level, const char* function, const char* path, const int& line, T t) {
    send_logevent(source, level, function, path, line, t);
    // FIXME this is actually an error condition - the engine shouldn't return
    // when it gets a log event with fatal severity
    std::terminate();
}

template<typename T, typename... Args>
[[ noreturn ]] void send_logevent_fatal(const logsource& source, const loglevel& level, const char* function, const char* path, const int& line, T t, Args... args) {
    send_logevent(source, level, function, path, line, t, args...);
    // FIXME this is actually an error condition - the engine shouldn't return
    // when it gets a log event with fatal severity
   std::terminate();
}

}
