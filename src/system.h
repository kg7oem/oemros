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

#define MIXIN(name, ...) class name; class name ,##__VA_ARGS__
#define ABSTRACT(name, ...) class name; typedef std::shared_ptr<name> name##_s; typedef std::weak_ptr<name> name##_w; class name : public oemros::object<name> ,##__VA_ARGS__
#define OBJECT(name, ...) class name; typedef std::shared_ptr<name> name##_s; typedef std::weak_ptr<name> name##_w; class name final : public oemros::object<name>, public std::enable_shared_from_this<name> ,##__VA_ARGS__
// private members come last so it is the default when the macro ends
#define OBJSTUFF(name) \
    public:\
        template<typename... Args> \
        static name##_s create(Args&&...args) { \
            return std::make_shared<name>(args...); \
        } \
        virtual const char* type(void) const override { return #name; }; \
 \
    private: \
        virtual void ____has_boilerplate(void) override { }; \
        name(const name&) = delete; \
        name(const name&&) = delete; \
        name& operator=(const name&) = delete;

#define UNUSED __attribute__((unused))

namespace oemros {

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
        static std::string get(void) {
            std::string gcc_pretty = __PRETTY_FUNCTION__;

            // FIXME this is not good enough
            size_t bracket_pos = gcc_pretty.find_first_of('[');
            if (bracket_pos == std::string::npos) {
                return gcc_pretty;
            }

            size_t semicolon_pos = gcc_pretty.find_first_of(';', bracket_pos);
            if (bracket_pos == std::string::npos) {
                return gcc_pretty;
            }

            size_t name_start = bracket_pos + 10;
            return gcc_pretty.substr(name_start, semicolon_pos - name_start);
        }
};

template <class T>
std::string classname(const T* _this = NULL) {
    return classname_t<T>::get();
}

template<class T>
class object {
    friend std::ostream& operator<<(std::ostream& os, const object& obj) {
        os << obj.description();
        return os;
    }

    friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<object>& obj) {
        os << "shared_ptr(use=" << obj.use_count();
        os << " " << *obj.get() << ")";
        return os;
    }

    private:
        virtual void ____has_boilerplate(void) = 0;
        // disable copy constructor
        object(const object&) = delete;
        // disable move constructor
        object(const object&&) = delete;
        // disable assignment operator
        object& operator=(const object&) = delete;

    public:
        object() = default;
        virtual const char* type(void) const = 0;
        virtual std::string description(void) const {
            std::stringstream ss;
            ss << "refcounted(" << this->type() << ")";
            return ss.str();
        }
        template<typename... Args>
        static std::shared_ptr<T> create(Args&&...args) {
            // https://stackoverflow.com/questions/7257144/when-to-use-stdforward-to-forward-arguments
            return std::make_shared<T>(std::forward<Args>(args)...);
        };
};

class errstream_t {
    friend std::ostream& operator<<(std::ostream& os, const errstream_t&);
};

extern errstream_t errstream;

[[ noreturn ]] void system_exit(exitvalue);
[[ noreturn ]] void system_panic(const char *);

}

#endif /* SRC_SYSTEM_H_ */
