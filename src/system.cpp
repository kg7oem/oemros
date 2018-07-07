/*
 * system.cpp
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

#include <cassert>
#include <cstdlib>
#include <cerrno>
#include <sstream>
#include <string>
#include <string.h>

#include "conf.h"
#include "logging.h"
#include "system.h"

namespace oemros {

errstream_t errstream;

// a panic must never use the logging system because the logging
// system uses panic if it can't function
[[ noreturn ]] void system_panic(const char *message) {
    std::cerr << "PANIC! " << message << std::endl;
    exit((int)exitvalue::panic);
}

[[ noreturn ]] void system_exit(exitvalue value) {
    log_debug("going to exit with value of ", (int)value);
    exit((int)value);
}

std::ostream& operator<<(std::ostream& os, const errstream_t& error) {
    char buf[CONF_ERRMSG_BUFLEN];
    char *message = strerror_r(errno, buf, CONF_ERRMSG_BUFLEN);
    os << message;
    return os;
}

static void threadpool_be_worker(std::mutex* pool_mutex, std::condition_variable* pool_cond, std::list<threadpool_cb>* work_queue) {
    log_trace("this is a brand new threadpool worker");
    std::unique_lock<std::mutex> lock(*pool_mutex);

    while(1) {
        log_trace("thread is about to wait on the condition variable; size = ", work_queue->size());
        pool_cond->wait(lock, [work_queue] { return work_queue->size() > 0; });
        log_trace("thread is done waiting for condition variable");

        threadpool_cb cb = work_queue->front();
        work_queue->pop_front();
        log_trace("will invoke callback from workqueue");
        cb();
        log_trace("done with callback from workqueue");
    }
}

threadpool::threadpool(size_t size_arg)
: size(size_arg) {
    log_trace("constructing a threadpool; size = ", this->size);
    assert(this->size > 0);

    for(size_t i = 0; i < this->size; i++) {
        log_trace("creating new thread; i = ", i);
        this->thread_list.push_back(new std::thread(threadpool_be_worker, &this->pool_mutex, &this->pool_cond, &this->work_queue));
    }
}

void threadpool::schedule(threadpool_cb cb) {
    log_trace("got a schedule request");

    {
        std::lock_guard<std::mutex> lock(this->pool_mutex);
        this->work_queue.push_back(cb);
    }

    this->pool_cond.notify_one();
}

static threadpool& threadpool_get(void) {
    static threadpool* pool_singleton = new threadpool(2);

    return *pool_singleton;
}

void threadpool_bootstrap(void) {
    log_trace("bootstrapping the threadpool");
    threadpool_get();
}

void threadpool_schedule(threadpool_cb cb) {
    threadpool_get().schedule(cb);
}

}

