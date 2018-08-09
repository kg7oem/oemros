/*
 * runloop.h
 *
 *  Created on: Jul 31, 2018
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

#include <boost/asio.hpp>
#include <chrono>
#include <functional>
#include <memory>

#include "object.h"
#include "thread.h"

namespace oemros {

class runloop;

class runloop_item : public baseobj {
    friend runloop;

    private:
        std::shared_ptr<runloop> loop;

    protected:
        virtual void start__child() = 0;
        boost::asio::io_service* get_loop_ioptr();

    public:
        runloop_item(std::shared_ptr<runloop> loop_in) : loop(loop_in) { }
        void start();
};

class runloop : public baseobj {
    friend boost::asio::io_service* runloop_item::get_loop_ioptr();

    private:
        boost::asio::io_service io;

    public:
        template <class T, typename... Args>
        std::shared_ptr<T> make_item(const Args&... args) {
            auto shared_us = std::dynamic_pointer_cast<runloop>(shared_from_this());
            return std::make_shared<T>(shared_us, args...);
        }
        template <class T, typename... Args>
        std::shared_ptr<T> make_started(const Args&... args) {
            auto new_item = make_item<T>(args...);
            new_item->start();
            return new_item;
        }
        void enter();
        void post(const std::function<void ()>& post_in);
        template <class Class, class Instance>
        void post(Class&& class_in, Instance&& instance_in) {
            post(std::bind(class_in, instance_in));
        }
};

class oneshot_timer : public runloop_item {
    using milliseconds = std::chrono::milliseconds;

    private:
        boost::asio::deadline_timer asio_timer{*get_loop_ioptr()};
        const milliseconds initial;
        void handler(const boost::system::error_code& error);

    public:
        oneshot_timer(std::shared_ptr<runloop> loop_in, milliseconds initial_in)
        : runloop_item(loop_in), initial(initial_in) { }
        virtual void start__child() override;
};

}
