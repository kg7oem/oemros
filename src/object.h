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
        virtual const char* type() const override { return #name; }; \
 \
    private: \

#define OBJECT(name, ...) \
    class name; \
    typedef std::shared_ptr<name> name##_s; \
    typedef std::weak_ptr<name> name##_w; \
    class name final : public oemros::object<name>, public std::enable_shared_from_this<name> ,##__VA_ARGS__

#define TOBJECT(name, template_spec, ...) \
    template <typename T> \
    class name; \
    template <typename T> \
    using name##_s = std::shared_ptr<name<T>>; \
    template <typename T> \
    using name##_w = std::weak_ptr<name<T>>; \
    template template_spec \
    class name final : public oemros::object<name<T>>, public std::enable_shared_from_this<name<T>> ,##__VA_ARGS__ \

// private members come last so it is the default when the macro ends
//
// FIXME this needs to have tests to make sure this macro doesn't ever
// accidently flip the default to public
#define OBJSTUFF(name) \
    public: \
        virtual const char* type() const override { return #name; }; \
        virtual const std::string description() const override { \
            std::stringstream ss; \
            ss << "refcounted(" << type() << ")"; \
            return ss.str(); \
        } \
 \
    private: \
        virtual void ____has_boilerplate() override { }; \
        name(const name&) = delete; \
        name(const name&&) = delete; \
        name& operator=(const name&) = delete; \

namespace oemros {

class object_interface {
    public:
        virtual ~object_interface() = default;
        virtual const char* type() const = 0;
        virtual const std::string description() const = 0;
};

class abstract : public object_interface {
    public:
        virtual ~abstract() = default;
        virtual const char* type() const override = 0;
        virtual const std::string description() const override = 0;
};

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
        // CLEANUP rename create() to make()
        static std::shared_ptr<T> create(Args&&...args) {
            return std::make_shared<T>(args...);
        }
};

}

#endif /* SRC_OBJECT_H_ */
