/*
 * thread.h
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

#pragma once

#include <boost/thread.hpp>
#include <functional>
#include <memory>

#include "object.h"

namespace oemros {

uint64_t thread_next_jobid();

class thread_queue : public baseobj {
    public:
        struct job;
        using cb_type = std::function<void (std::shared_ptr<job> job_in)>;
        using jobid_type = uint64_t;
        struct job {
            const jobid_type id = thread_next_jobid();
            const cb_type cb;
            job(cb_type cb_in) : cb(cb_in) { };
        };

    private:
        boost::condition_variable condition;
        boost::mutex mutex;
        const int num_workers = 8;
        std::list<boost::thread> workers;
        std::list<std::shared_ptr<job>> job_queue;
        boost::unique_lock<boost::mutex> get_lock();
        void be_worker();
        std::shared_ptr<job> add__priv(const cb_type& cb_in);

    public:
        thread_queue();
        static std::shared_ptr<job> add(const cb_type& cb_in);
};

}
