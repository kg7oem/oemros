/*
 * system.h
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

#ifndef SRC_SYSTEM_H_
#define SRC_SYSTEM_H_

#include <condition_variable>
#include <exception>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>

// g++ 6.3.0 as it comes in debian/stretch does not support maybe_unused
#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED [[ maybe_unused ]]
#endif

namespace oemros {

using string = std::string;

template <typename T> using strong_ptr = std::shared_ptr<T>;
template <typename T> using weak_ptr = std::weak_ptr<T>;

enum class exitvalue {
    ok = 0,
    fatal = 1,
    panic = 101,
};

class generic_error : public std::runtime_error {
    public:
        generic_error(const char*);
};

std::exception_ptr make_error(const char*);

template <class T>
class classname_t {
    public:
        static string get() {
            string gcc_pretty = __PRETTY_FUNCTION__;

            // FIXME this is not good enough
            size_t bracket_pos = gcc_pretty.find_first_of('[');
            if (bracket_pos == string::npos) {
                return gcc_pretty;
            }

            size_t semicolon_pos = gcc_pretty.find_first_of(';', bracket_pos);
            if (bracket_pos == string::npos) {
                return gcc_pretty;
            }

            size_t name_start = bracket_pos + 10;
            return gcc_pretty.substr(name_start, semicolon_pos - name_start);
        }
};

template <class T>
string classname(UNUSED const T* _this = NULL) {
    return classname_t<T>::get();
}

// the value for errno must be passed in so that it isn't possibly invalidated
// by some other system call that happens before errnostr() is invoked when
// it is in an argument list and it must be pass by value so it is copied
const char* errnostr(int);

[[ noreturn ]] void system_exit(exitvalue);
[[ noreturn ]] void system_panic(const char *);

}

#endif /* SRC_SYSTEM_H_ */
