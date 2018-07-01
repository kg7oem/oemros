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
#include <iostream>

#include "logging.h"

using namespace std;

namespace oemros {

#ifndef LOGGING_ENGINE_CREATE
#define LOGGING_ENGINE_CREATE new logging()
#endif

static logging * get_engine(void) {
    static oemros::logging *log_singleton = NULL;

    if (log_singleton == NULL) {
        // FIXME this makes it so this can't be a shared library
        // and configurable by the user
        log_singleton = LOGGING_ENGINE_CREATE;

        if (log_singleton == NULL) {
            system_panic("got NULL from logging engine creator");
        }
    }

    return log_singleton;
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

logevent::logevent(logsource source, loglevel level, const struct timeval timestamp, const char *function, const char *path, const int line, string message)
: source(source), level(level), timestamp(timestamp), function(function), path(path), line(line), message(message)
{ };

loglevel logging::current_level(void) const {
    return this->log_threshold;
}

loglevel logging::current_level(loglevel new_level) {
    loglevel old_level = this->log_threshold;
    this->log_threshold = new_level;
    return old_level;
}

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


const char * logging::source_name(logsource source) {
    switch (source) {
    case logsource::unknown:
        return "UNKNOWNSOURCE";
    case logsource::oemros:
        return "oemros";
    }

    log_fatal("switch() failed for logsource enum: ", (int)source);
}

void logging::deliver_event(const logevent& event) {
    for (auto&& i : this->destinations) {
        i->event(event);
    }
}

void logging::input_event(const logevent& event) {
    assert(event.level >= logging_get_level());

    if (this->buffer_events && this->destinations.size() == 0) {
        this->event_buffer.push_back(event);
    } else {
        this->deliver_event(event);
    }
}

void logging::add_destination(shared_ptr<logdest> destination) {
    if (this->destinations.size() == 0) {
        // deliver the events that were stored before any
        // destination was available
        for (auto&& i : this->event_buffer) {
            destination->event(i);
        }

        this->event_buffer.clear();
    }

    this->destinations.push_back(destination);
}

void logdest::event(const logevent& event) {
    string formatted = this->format_event(event);
    this->output(event, formatted);
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
            "%02d:%02d:%02d", timevals->tm_hour, timevals->tm_min, timevals->tm_sec);

    if (result >= LOGGING_TIMESTR_BUFLEN) {
        // FIXME this ignores errno
        system_panic("formatted time string was truncated");
    }

    return string(buf);
}

string logdest::format_event(const logevent& event) {

    stringstream buffer;
    buffer << this->format_time(event.timestamp) << " ";
    buffer << logging_source_name(event.source) << " ";
    buffer << logging_level_name(event.level) << " ";
    buffer << event.path << ":" << event.line << " ";
    buffer << event.function << ": ";
    buffer << event.message;

    return buffer.str();
}

void logdest::output(const logevent& event, const string formatted) {
    system_panic("this method should be overloaded");
}

void logstdio::output(const logevent& event, const string formatted) {
    if (event.level >= loglevel::warn) {
        cerr << formatted << endl;
    } else {
        cout << formatted << endl;
    }
}

logfile::logfile(const char *path) {
    this->outfile.open(path, ofstream::app);

    if (this->outfile.fail()) {
        log_fatal("could not open ", path, " for write: ", errstream);
    }
}

void logfile::output(const logevent& event, const string formatted) {
    this->outfile << formatted << endl;
}

}
