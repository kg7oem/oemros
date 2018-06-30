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

#include <iostream>

#include "logging.h"

using namespace std;

static oemros::logging *log_singleton = NULL;

namespace oemros {

void logging_bootstrap(void) {
    if (log_singleton != NULL) {
        system_panic("log_singleton was not NULL; attempt to double init logging system?");
    }

    log_singleton = new logging();
}

logging * logging_engine(void) {
    if (log_singleton == NULL) {
        system_panic("log_singleton was NULL; attempt to get logging engine before bootstrap?");
    }

    return log_singleton;
}

bool logging_should_log(loglevel level) {
    auto logging = logging_engine();
    return(level >= logging->current_level());
}

const char * logging_level_name(loglevel level) {
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
        return "UNKNOWN";
    }

    return NULL;
}

logevent::logevent(logsource source, loglevel level, const char *function, const char *path, const int linenum, string message)
: source(source), level(level), function(function), path(path), linenum(linenum), message(message)
{ };

loglevel logging::current_level(void) const {
    return this->log_threshold;
}

loglevel logging::current_level(loglevel new_level) {
    loglevel old_level = this->log_threshold;
    this->log_threshold = new_level;
    return old_level;
}

void logging::input_event(const logevent& event) {
    if (event.level < this->log_threshold) {
        return;
    }

    cout << "yep" << endl;
}

}
