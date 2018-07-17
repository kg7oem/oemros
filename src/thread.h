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
#include <future>
#include <thread>

#include "object.h"
#include "system.h"

namespace oemros {

using thread = std::thread;
using mutex = std::mutex;
using lock = std::unique_lock<mutex>;
// clang version 3.8.0-2ubuntu4 is not working with
// std::shared_mutex when in C++17 mode?
using shared_mutex = std::shared_timed_mutex;
using exclusive_lock = std::unique_lock<shared_mutex>;
using shared_lock = std::shared_lock<shared_mutex>;
using condition_variable = std::condition_variable;

using threadpool_cb = std::function<void (void)>;

lock make_lock(mutex&);
shared_lock make_shared_lock(shared_mutex&);
exclusive_lock make_exclusive_lock(shared_mutex&);

void threadpool_schedule(threadpool_cb);
void thread_bootstrap();
void thread_cleanup();

MIXIN(lockable) {
    protected:
        mutex lock_mutex;
        lock get_lock();
};

MIXIN(shareable) {
    protected:
        shared_mutex lock_mutex;
        shared_lock get_shared_lock();
        exclusive_lock get_exclusive_lock();
};

TOBJECT(<class T>, promise, public lockable) {
    OBJSTUFF(promise);

    private:
        bool pending = true;
        bool cancelled = false;
        std::promise<T> promobj;
        // CLEANUP this belongs in a thread_promise really
        // because the promise shouldn't have any code in it
        const std::function<T (void)> cb;

    public:
        promise() = default;
        // this should be a thread_promise constructor
        promise(std::function<T (void)> cb_arg) : cb(cb_arg)
        {
            threadpool_schedule([&]{
                T result = cb();
                log_trace("setting the result in the promise");
                set(result);
            });
        }
        virtual ~promise() = default;
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
            auto exclusive = get_lock();
            if (! pending) {
                log_fatal("attempt to set the value for a promise that is not pending");
            }
            pending = false;
            promobj.set_value(value);
        }
        T get() { return promobj.get_future().get(); }
        // it seems like it would be better if there was a promise like
        // alternate object that had no get() method and did have a merge
        // method() then the promise could extend it with the get() method
        void merge() { promobj.get_future().wait(); }
};

template <typename T, typename... Args>
promise_s<T> make_promise(Args&&...args) {
    return promise<T>::make(args...);
}

// CLEANUP this should probably be made into an OBJECT
class threadpool : public lockable {
    friend void threadpool_be_worker(threadpool*);

    private:
        bool should_run = true;
        condition_variable pool_cond;
        list<thread*> thread_list;
        // CLEANUP this should be called the job_queue instead of work_queue
        list<threadpool_cb> work_queue;
        threadpool(const threadpool&) = delete;
        threadpool(const threadpool&&) = delete;
        threadpool& operator=(const threadpool&) = delete;

    public:
        const size_t size = 0;
        threadpool(size_t);
        void shutdown();
        void schedule(threadpool_cb);
        template <typename T>
        // CLEANUP this should return a thread_promise
        strong_ptr<oemros::promise<T>> promise(std::function<T (void)> cb_in) {
            return make_promise<T>(cb_in);
        }
};

}

#endif /* SRC_THREAD_H_ */
