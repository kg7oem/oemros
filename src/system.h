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

#include <iostream>
#include <memory>
#include <ostream>
#include <thread>

namespace oemros {

enum class exitvalue {
    ok = 0,
    fatal = 1,
    panic = 101,
};

template<class T>
class refcounted : public std::enable_shared_from_this<T> {
    private:
        // copy constructor
        refcounted(const refcounted&) = delete;
        // move constructor
        refcounted(const refcounted&&) = delete;
        // assignment operator
        refcounted& operator=(const refcounted&) = delete;

    public:
        refcounted() = default;
        template<typename... Args>
        static std::shared_ptr<T> create(Args&&...args) {
            // https://stackoverflow.com/questions/7257144/when-to-use-stdforward-to-forward-arguments
            return std::make_shared<T>(std::forward<Args>(args)...);
        };
};

class errstream_t {
public:
    friend std::ostream& operator<<(std::ostream& os, const errstream_t&);
};

extern errstream_t errstream;

[[ noreturn ]] void system_exit(exitvalue);
void system_panic(const char *);

}

#endif /* SRC_SYSTEM_H_ */
