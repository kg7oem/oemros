/*
 * system.cpp
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
#include <cstdlib>
#include <cerrno>
#include <sstream>
#include <string>
#include <string.h>

#include "conf.h"
#include "logging.h"
#include "system.h"

namespace oemros {

errstream_t errstream;

generic_error::generic_error(const char* message_arg)
: std::runtime_error(message_arg) { }

std::exception_ptr make_error(const char* message) {
    auto error = generic_error(message);
    return std::make_exception_ptr(error);
};

// a panic must never use the logging system because the logging
// system uses panic if it can't function
[[ noreturn ]] void system_panic(const char *message) {
    std::cerr << "PANIC! " << message << std::endl;
    exit((int)exitvalue::panic);
}

[[ noreturn ]] void system_exit(exitvalue value) {
    log_debug("going to exit with value of ", (int)value);
    exit((int)value);
}

std::ostream& operator<<(std::ostream& os, const errstream_t& error) {
    char buf[CONF_ERRMSG_BUFLEN];
    char *message = strerror_r(errno, buf, CONF_ERRMSG_BUFLEN);
    os << message;
    return os;
}

}

