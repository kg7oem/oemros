/*
 * thread.h
 *
 *  Created on: Jul 7, 2018
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

#ifndef SRC_THREAD_H_
#define SRC_THREAD_H_

#include <condition_variable>
#include <exception>
#include <future>
#include <memory>
#include <thread>

namespace oemros {

using mutex_t = std::mutex;
using lock_t = std::unique_lock<mutex_t>;

MIXIN(lockable) {
    protected:
        mutex_t lock_mutex;
        lock_t lock();
};

using threadpool_cb = std::function<void (void)>;

class threadpool : public lockable {
    friend void threadpool_be_worker(threadpool*);

    private:
        bool should_run = true;
        std::condition_variable pool_cond;
        std::list<std::thread*> thread_list;
        std::list<threadpool_cb> work_queue;
        threadpool(const threadpool&) = delete;
        threadpool(const threadpool&&) = delete;
        threadpool& operator=(const threadpool&) = delete;

    public:
        const size_t size = 0;
        threadpool(size_t);
        void shutdown();
        void schedule(threadpool_cb);
};

void thread_bootstrap();
void thread_cleanup();
void threadpool_schedule(threadpool_cb);

template <class T>
class promise : public lockable {
    private:
        promise(const promise&) = delete;
        promise(const promise&&) = delete;
        promise& operator=(const promise&) = delete;
        bool pending = true;
        bool cancelled = false;
        std::promise<T> promobj;
        const std::function<T (void)> cb;

    public:
        promise() = default;
        promise(std::function<T (void)> cb_arg) : cb(cb_arg)
        {
            threadpool_schedule([&]{
                T result = cb();
                log_trace("setting the result in the promise");
                set(result);
            });
        }
        // FIXME this doesn't work - because of the shared_ptr that
        // always wraps it?
//        operator T() const {
//            log_debug("automatically getting future value");
//            return get;
//        }
        void cancel() {
            if (! pending) {
                log_fatal("attempt to cancel  a promise that is not pending");
            }
            pending = false;
            cancelled = true;
            auto exception = make_error("this promise has been cancelled");
            promobj.set_exception(exception);
        }
        void set(T value) {
            auto lock = promise::lock();
            if (! pending) {
                log_fatal("attempt to set the value for a promise that is not pending");
            }
            pending = false;
            promobj.set_value(value);
        }
        T get() { return promobj.get_future().get(); }
        void merge() { promobj.get_future().wait(); }
};

template <typename T, typename... Args>
std::shared_ptr<oemros::promise<T>> make_promise(Args&&...args) {
    return std::make_shared<oemros::promise<T>>(args...);
}

lock_t make_lock(mutex_t&);

}

#endif /* SRC_THREAD_H_ */
