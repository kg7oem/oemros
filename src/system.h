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
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>

#define REFCOUNTED(name, ...) class name; typedef std::shared_ptr<name> name##_s; typedef std::weak_ptr<name> name##_w; class name : public oemros::refcounted<name> ,##__VA_ARGS__
#define REFLEAF(name, ...) class name; typedef std::shared_ptr<name> name##_s; typedef std::weak_ptr<name> name##_w; class name : public oemros::refcounted<name>, public std::enable_shared_from_this<name> ,##__VA_ARGS__

#define UNUSED __attribute__((unused))

namespace oemros {

enum class exitvalue {
    ok = 0,
    fatal = 1,
    panic = 101,
};

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
class refcounted {
    friend std::ostream& operator<<(std::ostream& os, const refcounted& obj) {
        os << obj.description();
        return os;
    }

    friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<refcounted>& obj) {
        os << "shared_ptr(use=" << obj.use_count();
        os << " " << *obj.get() << ")";
        return os;
    }

    private:
        // disable copy constructor
        refcounted(const refcounted&) = delete;
        // disable move constructor
        refcounted(const refcounted&&) = delete;
        // disable assignment operator
        refcounted& operator=(const refcounted&) = delete;

    public:
        refcounted() = default;
        virtual std::string description(void) const {
            std::stringstream ss;
            ss << "refcounted(" << classname<T>() << ")";
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

using threadpool_cb = std::function<void (void)>;

class threadpool {
    private:
        threadpool(const threadpool&) = delete;
        threadpool(const threadpool&&) = delete;
        threadpool& operator=(const threadpool&) = delete;
        std::list<std::thread*> thread_list;
        std::list<threadpool_cb> work_queue;
        std::mutex pool_mutex;
        std::condition_variable pool_cond;

    public:
        const size_t size = 0;
        threadpool(size_t);
        void schedule(threadpool_cb);
};

void threadpool_schedule(threadpool_cb);

[[ noreturn ]] void system_exit(exitvalue);
void system_panic(const char *);

}

#endif /* SRC_SYSTEM_H_ */
