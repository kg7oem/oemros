/*
 * system.h
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

#pragma once

#include <exception>
#include <stdexcept>

namespace oemros {

enum class exit_code {
    ok = 0,
    failed = 1,
    doublefault = 2,
};

const char* enum_to_str(const exit_code& code_in);

struct fatal_error : std::runtime_error {
    fatal_error(const std::string& what_in) : runtime_error(what_in) { }
};

struct enum_to_str_error : fatal_error {
        enum_to_str_error(const char* enum_name_in);
};

}
