/*
 * logging.h
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

#ifndef SRC_LOGGING_H_
#define SRC_LOGGING_H_

#include <fstream>
#include <list>
#include <iostream>
#include <memory>
#include <sstream>
#include <sys/time.h>
#include <thread>

#include "conf.h"
#include "system.h"

namespace oemros {

#ifndef LOGGING_TIMESTR_BUFLEN
#define LOGGING_TIMESTR_BUFLEN 128
#endif

#ifndef LOGGING_SOURCE_T
#define LOGGING_SOURCE_T logsource_t
enum class logsource_t {
    unknown = 0,
    oemros = 1,
    hamlib = 2,
};
#endif // LOGGING_SOURCE_T

#ifndef LOGGING_LEVEL_T
#define LOGGING_LEVEL_T loglevel_t
enum class loglevel_t {
    fatal = 100,  // execution will stop
    error = 8,
    warn = 7,     // output to stderr
    notice = 6,   // show to user even if quiet is on
    info = 5,     // show to user by default
    verbose = 4,  // opt-in here and down - must turn on the log level
    debug = 3,    // show values being used in logic
    lots = 2,     // lots of stuff
    trace = 1,    // log input and output of subsystems
    unknown = 0,
};
#endif // LOGGING_LEVEL_T

typedef LOGGING_SOURCE_T logsource;
typedef LOGGING_LEVEL_T loglevel;

#ifdef LOGGING_MACROS
#define log_fatal(...) oemros::log__level_tf(oemros::logsource::oemros, oemros::loglevel::fatal, __PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) oemros::log__level_t(oemros::logsource::oemros, oemros::loglevel::error, __PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) oemros::log__level_t(oemros::logsource::oemros, oemros::loglevel::warn, __PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#define log_notice(...) oemros::log__level_t(oemros::logsource::oemros, oemros::loglevel::notice, __PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) oemros::log__level_t(oemros::logsource::oemros, oemros::loglevel::info, __PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#define log_verbose(...) oemros::log__level_t(oemros::logsource::oemros, oemros::loglevel::verbose, __PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) oemros::log__level_t(oemros::logsource::oemros, oemros::loglevel::debug, __PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#define log_lots(...) oemros::log__level_t(oemros::logsource::oemros, oemros::loglevel::lots, __PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#define log_trace(...) oemros::log__level_t(oemros::logsource::oemros, oemros::loglevel::trace, __PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#endif

class logevent {
public:
    const logsource source = logsource::unknown;
    const loglevel level = loglevel::unknown;
    const struct timeval timestamp = { };
    const std::thread::id tid;
    const char *function = NULL;
    const char *path = NULL;
    const int line = 0;
    const std::string message;

    logevent(logsource, loglevel, const struct timeval when, std::thread::id, const char *, const char *, const int, const std::string);
};

class logdest {
private:
    std::mutex output_mutex;

public:
    virtual void event(const logevent&);
    std::string format_time(const struct timeval&);
    virtual std::string format_event(const logevent&);
    void output(const logevent&, const std::string);
    virtual void output__child(const logevent&, const std::string) = 0;
};

class logstdio : public logdest {
public:
    virtual void output__child(const logevent&, const std::string);
};

class logfile : public logdest {
private:
    std::ofstream outfile;

public:
    logfile(const char *);
    virtual void output__child(const logevent&, const std::string);
};

class logging {
private:
    loglevel log_threshold = loglevel::error;
    bool buffer_events = true;
    bool deliver_events = false;
    std::list<std::shared_ptr<logdest>> destinations;
    std::list<logevent> event_buffer;

    void deliver_event(const logevent&);

public:
    loglevel current_level(void) const;
    loglevel current_level(loglevel);
    const char * level_name(loglevel);
    const char * source_name(logsource);

    logging(void);
    void start(void);
    void input_event(const logevent&);
    void add_destination(std::shared_ptr<logdest>);
};

void logging_cleanup(void);
loglevel logging_get_level(void);
loglevel logging_set_level(loglevel);
const char * logging_level_name(loglevel);
bool logging_should_log(loglevel);
void logging_add_destination(std::shared_ptr<logdest>);
void logging_input_event(const logevent&);

template <typename T>
void log__accumulate_value(std::stringstream& sstream, T t) {
    sstream << t;
}

template<typename T, typename... Args>
void log__accumulate_value(std::stringstream& sstream, T t, Args... args) {
    log__accumulate_value(sstream, t);
    log__accumulate_value(sstream, args...);
}

template <typename T>
void log__level_t(logsource source, loglevel level, const char *function, const char *path, int line, T t) {
    if (logging_should_log(level)) {
        struct timeval when;
        int result = gettimeofday(&when, NULL);
        if (result != 0) {
            // FIXME this is eating the error reason, no errno usage
            system_panic("gettimeofday() failed");
        }

        std::stringstream sstream;
        log__accumulate_value(sstream, t);

        auto tid = std::this_thread::get_id();
        logevent event(source, level, when, tid, function, path, line, sstream.str());
        logging_input_event(event);
    }
}

template<typename T, typename... Args>
void log__level_t(logsource source, loglevel level, const char *function, const char *path, int line, T t, Args... args) {
    if (logging_should_log(level)) {
        struct timeval when;
        int result = gettimeofday(&when, NULL);
        if (result != 0) {
            // FIXME this is eating the error reason, no errno usage
            system_panic("gettimeofday() failed");
        }

        std::stringstream sstream;
        log__accumulate_value(sstream, t);
        log__accumulate_value(sstream, args...);

        auto tid = std::this_thread::get_id();
        logevent event(source, level, when, tid, function, path, line, sstream.str());
        logging_input_event(event);
    }
}

template <typename T>
[[ noreturn ]] void log__level_tf(logsource source, loglevel level, const char *function, const char *path, int line, T t) {
    log__level_t(source, level, function, path, line, t);
    system_exit(exitvalue::fatal);
}

template<typename T, typename... Args>
[[ noreturn ]] void log__level_tf(logsource source, loglevel level, const char *function, const char *path, int line, T t, Args... args) {
    log__level_t(source, level, function, path, line, t, args...);
    system_exit(exitvalue::fatal);
}

void logging_start(void);

}

#endif /* SRC_LOGGING_H_ */
