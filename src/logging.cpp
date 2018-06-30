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
#include <iostream>

#include "logging.h"

using namespace std;

static oemros::logging *log_singleton = NULL;

namespace oemros {

void logging_bootstrap(void) {
    assert(log_singleton == NULL);
    log_singleton = new logging();
}

void logging_cleanup(void) {
    assert(log_singleton != NULL);
    delete log_singleton;
}

static logging * logging_get_engine(void) {
    assert(log_singleton != NULL);
    return log_singleton;
}

loglevel logging_get_level(void) {
    return logging_get_engine()->current_level();
}

loglevel logging_set_level(loglevel new_level) {
    return logging_get_engine()->current_level(new_level);
}

bool logging_should_log(loglevel level) {
    auto logging = logging_get_engine();
    return(level >= logging->current_level());
}

void logging_add_destination(shared_ptr<logdest> destination) {
    logging_get_engine()->add_destination(destination);
}

void logging_input_event(const logevent& event) {
    logging_get_engine()->input_event(event);
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
        return "UNKNOWNLEVEL";
    }

    log_fatal("switch() failed for loglevel enum: ", (int)level);
}

const char * logging_source_name(logsource source) {
    switch (source) {
    case logsource::unknown:
        return "UNKNOWNSOURCE";
    case logsource::oemros:
        return "oemros";
    }

    log_fatal("switch() failed for logsource enum: ", (int)source);
}

logevent::logevent(logsource source, loglevel level, const char *function, const char *path, const int line, string message)
: source(source), level(level), function(function), path(path), line(line), message(message)
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
    assert(event.level >= logging_get_level());

    for (auto i : this->destinations) {
        i->event(event);
    }
}

void logging::add_destination(shared_ptr<logdest> destination) {
    this->destinations.push_back(destination);
}

void logdest::event(const logevent& event) { }

string logdest::format_event(const logevent& event) {
    stringstream buffer;

    buffer << logging_source_name(event.source) << " ";
    buffer << logging_level_name(event.level) << " ";
    buffer << event.path << ":" << event.line << " ";
    buffer << event.function << ": ";
    buffer << event.message;

    return buffer.str();
}

void logstdio::event(const logevent& event) {
    logdest::event(event);

    string formatted = this->format_event(event);

    if (event.level >= loglevel::warn) {
        cerr << formatted << endl;
    } else {
        cout << formatted << endl;
    }
}

}
