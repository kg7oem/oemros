/*
 * thread.cxx
 *
 *  Created on: Aug 8, 2018
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

#include "thread.h"

namespace oemros {

static thread_queue global_thread_queue;

uint64_t thread_next_jobid() {
    static uint64_t last_jobid = 0;
    return ++last_jobid;
}

thread_queue::thread_queue() {
    for(int i = 0; i < num_workers; i++) {
        auto bound = boost::bind(&thread_queue::be_worker, this);
        auto new_worker = boost::thread(bound);
    }
}

boost::unique_lock<boost::mutex> thread_queue::get_lock() {
    return boost::unique_lock<boost::mutex>(mutex);
}

std::shared_ptr<thread_queue::job> thread_queue::add(const thread_queue::cb_type& cb_in) {
    return global_thread_queue.add__priv(cb_in);
}

std::shared_ptr<thread_queue::job> thread_queue::add__priv(const thread_queue::cb_type& cb_in) {
    auto lock = get_lock();
    auto ticket = std::make_shared<job>(cb_in);
    job_queue.push_back(ticket);
    condition.notify_one();
    return ticket;
}

void thread_queue::be_worker() {
    while(1) {
        auto lock = get_lock();
        while(job_queue.size() == 0) {
            condition.wait(lock);
        }
        assert(job_queue.size() != 0);
        auto our_job = job_queue.front();
        job_queue.pop_front();

        // after this the queue no longer needs to be locked
        lock.unlock();
        our_job->cb(our_job);
    }
}

}
