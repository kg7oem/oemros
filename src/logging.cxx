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

void logjam::handlers::fatal(const logevent& event_in) {
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
    auto log_level_env = std::getenv("OEMROS_PRELOG_LEVEL");
    auto log_output_env = std::getenv("OEMROS_PRELOG_OUTPUT");

    if (log_level_env == nullptr) {
        min_log_level = logjam::loglevel::none;
    } else {
        min_log_level = logjam::level_from_name(log_level_env);
    }

    if (log_output_env != nullptr) {
        auto console_output = std::make_shared<logjam::logconsole>(min_log_level);
        add_destination(console_output);
    }
}
