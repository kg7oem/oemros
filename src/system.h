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

#define MIXIN(name, ...) class name ,##__VA_ARGS__

#define ABSTRACT(name, ...) \
    class name; \
    typedef std::shared_ptr<name> name##_s; \
    typedef std::weak_ptr<name> name##_w; \
    class name : public abstract ,##__VA_ARGS__

// private members come last so it is the default when the macro ends
//
// FIXME this needs to have tests to make sure this macro doesn't ever
// accidently flip the default to public
#define ABSSTUFF(name) \
    public: \
        virtual const char* type(void) const override { return #name; }; \
 \
    private: \

#define OBJECT(name, ...) \
    class name; \
    typedef std::shared_ptr<name> name##_s; \
    typedef std::weak_ptr<name> name##_w; \
    class name final : public oemros::object<name>, public std::enable_shared_from_this<name> ,##__VA_ARGS__

// private members come last so it is the default when the macro ends
//
// FIXME this needs to have tests to make sure this macro doesn't ever
// accidently flip the default to public
//
// // FIXME if the description() method is in the object class then it won't
// work for satisfying the pure virtual description() method from the
// base classes - why?
#define OBJSTUFF(name) \
    public: \
        virtual const char* type(void) const override { return #name; }; \
        virtual const std::string description() const override { \
            std::stringstream ss; \
            ss << "refcounted(" << type() << ")"; \
            return ss.str(); \
        } \
 \
    private: \
        virtual void ____has_boilerplate(void) override { }; \
        name(const name&) = delete; \
        name(const name&&) = delete; \
        name& operator=(const name&) = delete; \

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
    (void)_this;
    return classname_t<T>::get();
}

class object_interface {
    public:
        virtual ~object_interface() = default;
        virtual const char* type() const = 0;
        virtual const std::string description() const = 0;
};

class abstract : public object_interface {
    public:
        virtual ~abstract() = default;
        virtual const char* type() const = 0;
        virtual const std::string description() const override = 0;
};

// FIXME if the template is removed this breaks - why?
template<class T>
class object : public object_interface {
    // FIXME does this need to be templated to work?
    friend std::ostream& operator<<(std::ostream& os, const T& obj) {
        os << obj.description();
        return os;
    }

    // FIXME does this need to be templated to work?
    friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<T>& obj) {
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
        template<typename... Args>
        // FIXME rename create() to make()
        static std::shared_ptr<T> create(Args&&...args) {
            return std::make_shared<T>(args...);
        }
};

const char* errnostr(int);

class errstream_t {
    friend std::ostream& operator<<(std::ostream& os, const errstream_t&);
};

extern errstream_t errstream;

[[ noreturn ]] void system_exit(exitvalue);
[[ noreturn ]] void system_panic(const char *);

}

#endif /* SRC_SYSTEM_H_ */
