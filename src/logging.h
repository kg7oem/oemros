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

#include <iostream>
#include <sstream>

#include "system.h"

namespace oemros {

enum class loglevel {
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

enum class logsource {
    unknown = 0,
    oemros = 1,
};

class logevent {
public:
    const logsource source = logsource::unknown;
    const loglevel level = loglevel::unknown;
    const char *function = NULL;
    const char *path = NULL;
    const int linenum = 0;
    const std::string message;

    logevent(logsource, loglevel, const char *, const char *, const int, const std::string);
};

#define log_fatal(...) oemros::log__level_t(true, oemros::logsource::oemros, oemros::loglevel::fatal, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) oemros::log__level_t(false, oemros::logsource::oemros, oemros::loglevel::debug, __func__, __FILE__, __LINE__, __VA_ARGS__)

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
void log__level_t(bool fatal, logsource source, loglevel level, const char *function, const char *path, int line, T t) {
    std::stringstream sstream;
    log__accumulate_value(sstream, t);

    logevent event(source, level, function, path, line, sstream.str());
    std::cout << function << "():" << line << " " << event.message << std::endl;

    if (fatal) {
        system_exit(exitvalue::fatal);
    }
}

template<typename T, typename... Args>
void log__level_t(bool fatal, logsource source, loglevel level, const char *function, const char *path, int line, T t, Args... args) {
    std::stringstream sstream;
    log__accumulate_value(sstream, t);
    log__accumulate_value(sstream, args...);

    logevent event(source, level, function, path, line, sstream.str());
    std::cout << function << "():" << line << " " << event.message << std::endl;

    if (fatal) {
        system_exit(exitvalue::fatal);
    }
}

}

#endif /* SRC_LOGGING_H_ */
