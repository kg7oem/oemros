/*
 * object.h
 *
 *  Created on: Jul 15, 2018
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

#ifndef SRC_OBJECT_H_
#define SRC_OBJECT_H_

#include "system.h"

// FIXME how can this be turned into something like
//     using object_auto_cast = std::dynamic_pointer_cast;
// so the usage of a macro can be avoided?
#ifdef NDEBUG
#define OBJECT_AUTO_CAST(name, value) std::static_pointer_cast<name>(value)
#else
#define OBJECT_AUTO_CAST(name, value) std::dynamic_pointer_cast<name>(value)
#endif

namespace oemros {

class object;
using object_s = strong_ptr<object>;
using object_w = weak_ptr<object>;

class object : public std::enable_shared_from_this<object> {
    private:
        object(const object&) = delete;
        object(const object&&) = delete;
        object& operator=(const object&) = delete;

    public:
        object() = default;
        virtual ~object() = default;
        object_s strong_ref() { return object::shared_from_this(); }
        object_w weak_ref() { return object::shared_from_this(); }
};

}







/*
#define MIXIN(name, ...) class name ,##__VA_ARGS__

#define ABSTRACT(name, ...) \
    class name; \
    typedef oemros::strong_ptr<name> name##_s; \
    typedef oemros::weak_ptr<name> name##_w; \
    class name : public abstract ,##__VA_ARGS__

// private members come last so it is the default when the macro ends
//
// FIXME this needs to have tests to make sure this macro doesn't ever
// accidently flip the default to public
#define ABSSTUFF(name) \
    public: \
        oemros::strong_ptr<name> strong_ref() { return this->shared_from_this(); } \
        oemros::weak_ptr<name> weak_ref() { return this->shared_from_this(); } \
        virtual const char* type() const override { return #name; }; \
 \
    private:

#define OBJECT(name, ...) \
    class name; \
    typedef oemros::strong_ptr<name> name##_s; \
    typedef oemros::weak_ptr<name> name##_w; \
    class name final : public oemros::object<name> ,##__VA_ARGS__

#define TOBJECT(template_spec, name, ...) \
    template <typename T> class name; \
    template <typename T> using name##_s = oemros::strong_ptr<name<T>>; \
    template <typename T> using name##_w = oemros::weak_ptr<name<T>>; \
    template template_spec \
    class name final : public oemros::object<name<T>> ,##__VA_ARGS__

// private members come last so it is the default when the macro ends
//
// FIXME this needs to have tests to make sure this macro doesn't ever
// accidently flip the default to public
#define OBJSTUFF(name) \
    public: \
        oemros::strong_ptr<name> strong_ref() { return abstract::shared_from_this(); } \
        oemros::weak_ptr<name> weak_ref() { return this->abstract::shared_from_this(); } \
        virtual const char* type() const override { return #name; }; \
        virtual const oemros::string description() const override { \
            std::stringstream ss; \
            ss << "refcounted(" << type() << ")"; \
            return ss.str(); \
        } \
 \
    private: \
        virtual void ____has_boilerplate() override { }; \
        name(const name&) = delete; \
        name(const name&&) = delete; \
        name& operator=(const name&) = delete;

namespace oemros {


class root_object : public std::enabled_shared_from_this<root_object> {

};

class object_interface {
    public:
        virtual ~object_interface() = default;
        virtual const char* type() const = 0;
        virtual const string description() const = 0;
};

class abstract : public std::enable_shared_from_this<abstract> {
    public:
        virtual ~abstract() = default;
        virtual const char* type() const = 0;
        virtual const string description() const = 0;
};

template<class T>
class object : public abstract {
    // FIXME move to root of object hierarchy
    friend std::ostream& operator<<(std::ostream& os, const T& obj) {
        os << obj.description();
        return os;
    }

    // FIXME move to root of object hierarchy
    friend std::ostream& operator<<(std::ostream& os, const oemros::strong_ptr<T>& obj) {
        os << "shared_ptr(use=" << obj.use_count();
        os << " " << *obj.get() << ")";
        return os;
    }

    private:
        virtual void ____has_boilerplate() = 0;
        // disable copy constructor
        object(const object&) = delete;
        // disable move constructor
        object(const object&&) = delete;
        // disable assignment operator
        object& operator=(const object&) = delete;

    public:
        object() = default;
        template<typename... Args>
        static oemros::strong_ptr<T> make(Args&&...args) {
            return std::make_shared<T>(args...);
        }
};

}

*/

#endif /* SRC_OBJECT_H_ */
