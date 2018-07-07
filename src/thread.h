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

namespace oemros {

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

void threadpool_bootstrap(void);
void threadpool_schedule(threadpool_cb);

template <class T>
class promise {
    private:
        promise(const promise&) = delete;
        promise(const promise&&) = delete;
        promise& operator=(const promise&) = delete;
        std::promise<T> promobj;
        std::function<T (void)> cb;

    public:
        // FIXME this doesn't work - because of the shared_ptr that
        // always wraps it?
//        operator T() const {
//            log_debug("automatically getting future value");
//            return this->get;
//        }
        void set(T value) {
            this->promobj.set_value(value);
        }
        T get(void) {
            return this->promobj.get_future().get();
        }
        void wait(void) {
            this->promobj.get_future().wait();
        }
        promise(void) = default;
        promise(std::function<T (void)> cb_arg)
        : cb(cb_arg)
        {
            threadpool_schedule([&]{
                T result = cb();
                this->promobj.set_value(result);
            });
        }
};

template <typename T, typename... Args>
std::shared_ptr<oemros::promise<T>> make_promise(Args&&...args) {
    return std::make_shared<oemros::promise<T>>(args...);
}

}

#endif /* SRC_THREAD_H_ */
