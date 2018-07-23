/*
 * logging.cxx
 *
 *  Created on: Jul 22, 2018
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

#include <cstdlib>
#include <iostream>

#include "logging.h"

const oemros::_log_sources oemros::log_sources;

#define OEMROS_PRELOG "OEMROS_PRELOG"
#define OEMROS_PRELOG_LEVEL "OEMROS_PRELOG_LEVEL"
#define OEMROS_PRELOG_OUTPUT "OEMROS_PRELOG_OUTPUT"

void logjam::handlers::fatal(const logevent& event_in) {
    if (std::getenv(OEMROS_PRELOG) != nullptr) {
        // FIXME do this
        std::cout << "OEMROS should output buffered log events but I don't yet";
    }

    // FIXME this doesn't work does it? The user could catch the
    // exception and then the logengine calling terminate() here
    // won't ever get a chance to call terminate() in case this
    // doesn't wind up killing the process
    //
    // or is it
    //
    // this works exactly the way the user wants it to if the user wants
    // to use exceptions so let them use it the way they want to?
    throw oemros::fatal_error(event_in.message);
}

logjam::logengine* logjam::handlers::get_engine() {
    static oemros::log_engine engine;
    return &engine;
}

oemros::log_engine::log_engine() : logjam::logengine() {
    // control auto prelog level
    auto prelog_env = std::getenv(OEMROS_PRELOG);
    // control initial log level
    auto log_level_env = std::getenv(OEMROS_PRELOG_LEVEL);
    // control initial log destination
    auto log_output_env = std::getenv(OEMROS_PRELOG_OUTPUT);

    if (prelog_env != nullptr) {
        // FIXME why aren't these in std?
        // these have no meaning when in auto prelog mode
        unsetenv(OEMROS_PRELOG_LEVEL);
        unsetenv(OEMROS_PRELOG_OUTPUT);

        // Events at the user specified level or greater will be
        // buffered until either start() is called during normal
        // initialization or a fatal error happens before start is called.
        // If a fatal error happens the contents of the buffer
        // will be sent to the logconsole destination before the
        // program exits. If start() is called then the buffered
        // events will be delivered at that time and the user will
        // see them then. This means that when auto prelog is on
        // log destinations will need to use the greater of their
        // normal configured log level or the prelog specified level
        // for the user to get behavior that makes sense: seeing
        // all the messages at the specified log level if the program
        // crashes or not.
        min_log_level = logjam::level_from_name(prelog_env);
        std::cout << "OEMROS prelog level: " << prelog_env << std::endl;
    } else if (log_level_env != nullptr) {
        // set the initial minimum log level to the user specified value
        min_log_level = logjam::level_from_name(log_level_env);
    } else {
        // by default no messages will be buffered
        min_log_level = logjam::loglevel::none;
    }

    if (log_output_env != nullptr) {
        auto console_output = std::make_shared<logjam::logconsole>(min_log_level);
        add_destination(console_output);
        started = true;
    }
}

namespace oemros {

using logjam::loglevel;

// if auto prelog is turned on then see if the auto prelog level
// is less than the level given to the constructor and return that
// instead so the user can specify they want more logging detail
// from the command line
static loglevel maybe_prelog_level(const loglevel& constructor_in) {

    auto prelog_env = std::getenv(OEMROS_PRELOG);
    if (prelog_env == nullptr) {
        return constructor_in;
    }

    auto prelog_level = logjam::level_from_name(prelog_env);

    if (prelog_level < constructor_in) {
        std::cout << "OEMROS forcing loglevel of " << prelog_env << " for log destination" << std::endl;
        return prelog_level;
    }

    // also handles the case where they are the same
    return constructor_in;
}

log_console::log_console(const loglevel& level_in)
: logjam::logconsole(maybe_prelog_level(level_in)) { }

}
