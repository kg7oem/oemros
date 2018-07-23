/*
 * system.cxx
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

#include "system.h"

namespace oemros {

static std::string enum_to_str_error_what(const char* enum_name_in) {
    std::string buf;

    buf += "enum '";
    buf += enum_name_in;
    buf += "' string conversion failure";

    return buf;
}

enum_to_str_error::enum_to_str_error(const char* enum_name_in)
: fatal_error(enum_to_str_error_what(enum_name_in)) { }

const char* enum_to_str(const exit_code& code_in) {
    switch(code_in) {
        case exit_code::ok: return "ok";
        case exit_code::failed: return "failed";
        case exit_code::doublefault: return "doublefault";
    }

    throw enum_to_str_error("exit_code");
}

}
