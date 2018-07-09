/*
 * thread.cpp
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

#include <cassert>
#include <cstdlib>

#include "logging.h"
#include "thread.h"

namespace oemros {

//static void threadpool_be_worker(bool* should_run, std::mutex* pool_mutex, std::condition_variable* pool_cond, std::list<threadpool_cb>* work_queue) {
void threadpool_be_worker(threadpool* pool) {
    log_trace("this is a brand new threadpool worker");
    auto lock = pool->lock();

    while(1) {
        log_trace("thread is about to wait on the condition variable; size = ", pool->work_queue.size());
        pool->pool_cond.wait(lock, [pool] {
            log_trace("condition variable check: should_run: ", pool->should_run, "; size: ", pool->work_queue.size());
            if (! pool->should_run) {
                return true;
            }
            return pool->work_queue.size() > 0;
        });

        log_trace("thread is done waiting for condition variable");
        if (! pool->should_run) {
            log_trace("breaking out of the dequeue loop");
            break;
        }

        threadpool_cb cb = pool->work_queue.front();
        pool->work_queue.pop_front();
        log_trace("will invoke callback from workqueue");
        cb();
        log_trace("done with callback from workqueue");
    }

    log_trace("thread is out of workqueue dequeue loop");
    lock.unlock();
}

lock_t lockable::lock(void) {
    log_trace("creating a new lock and acquiring the mutex");
    auto new_lock = make_lock(this->lock_mutex);
    log_trace("got the lock");
    return new_lock;
}

threadpool::threadpool(size_t size_arg)
: size(size_arg) {
    log_trace("constructing a threadpool; size = ", this->size);
    assert(this->size > 0);

    for(size_t i = 0; i < this->size; i++) {
        log_trace("creating new thread; i = ", i);
        this->thread_list.push_back(new std::thread(threadpool_be_worker, this));
    }
}

void threadpool::shutdown(void) {
    log_trace("starting threadpool shutdown");

    {
        log_trace("locking and setting should_run to false");
        auto lock = this->lock();
        this->should_run = false;
        log_trace("done with locked phase");
    }

    log_trace("waking up all threads");
    this->pool_cond.notify_all();

    // FIXME there should be some kind of limit on the amount of time
    // that it can take before the thread returns from join() or the
    // thread is cancelled
    for(auto&& i : this->thread_list) {
        log_trace("joining thread ", i->get_id());
        i->join();
        log_trace("deleting old thread");
        delete i;
    }

    log_trace("emptying the work queue; size = ", this->work_queue.size());
    this->work_queue.clear();
}

void threadpool::schedule(threadpool_cb cb) {
    log_trace("got a schedule request");

    {
        auto lock = this->lock();

        if (! this->should_run) {
            log_fatal("attempt to schedule a job on a threadqueue that is not running");
        }

        this->work_queue.push_back(cb);
    }

    this->pool_cond.notify_one();
}

lock_t make_lock(mutex_t& mutex) {
    log_trace("creating lock object and acquiring mutex");
    auto new_lock = lock_t(mutex);
    log_trace("acquired the mutex");
    return new_lock;
}

static threadpool& threadpool_get(void) {
    // TODO it would be nice if the thread pool size was dynamic
    static threadpool* pool_singleton = new threadpool(CONF_THREADPOOL_SIZE);
    return *pool_singleton;
}

void thread_bootstrap(void) {
    log_trace("bootstrapping the threadpool");
    threadpool_get();
    log_trace("setting thread cleanup to happen via atexit()");
    atexit(thread_cleanup);
}

void thread_cleanup(void) {
    log_trace("shutting down the threadpool");
    threadpool_get().shutdown();
}

void threadpool_schedule(threadpool_cb cb) {
    threadpool_get().schedule(cb);
}

}
