/*
 * object.h
 *
 *  Created on: Jul 30, 2018
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

#include <algorithm>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <vector>

namespace oemros {

uint32_t get_next_subscription_num();

struct baseobj : public std::enable_shared_from_this<baseobj> {
    baseobj& operator=(const baseobj&) = delete;
    baseobj(const baseobj&) = delete;
    baseobj(const baseobj&&) = delete;
    baseobj() = default;
    virtual ~baseobj() = default;
};

template <typename... Args>
class event_source : public baseobj {
    public:
        using sink_type = std::function<void (Args...)>;
        struct subscription {
            const uint32_t number = get_next_subscription_num();
            const bool repeat;
            const sink_type cb;
            subscription(const sink_type& cb_in, const bool& repeat_in = true)
            : repeat(repeat_in), cb(cb_in) { }
        };

    private:
        std::vector<std::shared_ptr<subscription>> subscribers;

    public:
        std::shared_ptr<subscription> subscribe(const sink_type& handler_in, const bool& repeat_in = true) {
            auto ticket = std::make_shared<subscription>(handler_in, repeat_in);
            subscribers.push_back(ticket);
            return ticket;
        }

        void deliver(Args&... args) {
            std::list<std::shared_ptr<subscription>> to_remove;

            for(auto&& i : subscribers) {
                i->cb(args...);
                if (! i->repeat) {
                    to_remove.push_back(i);
                }
            }

            for(auto&& i : to_remove) {
                auto found = std::find(subscribers.begin(), subscribers.end(), i);
                assert(found != subscribers.end());
                subscribers.erase(found);
            }
        }
};

template <typename T>
class value_source : public baseobj {
    using value_type = T;
    // the sink gets const values so it can not modify the stored value
    using source_type = event_source<const value_source<T>&>;
    using sink_type = typename source_type::sink_type;
    using subscription_type = typename source_type::subscription;
    using timestamp_type = std::chrono::time_point<std::chrono::system_clock>;

    private:
        value_type value;
        source_type source;
        timestamp_type last_update;
        void deliver() { source.deliver(*this); }

    public:
        value_source(const T& value_in) : value(value_in) { }
        value_source& operator=(const T& value_in) { set(value_in); return *this; }
        operator T() { return get(); }
        T get() { return value; }
        T set(const T& value_in) {
            auto now = std::chrono::system_clock::now();
            auto old = value;
            value = value_in;
            last_update = now;
            deliver();
            return old;
        }
        template <typename... Args>
        std::shared_ptr<subscription_type> subscribe(const Args&&... args) {
            return source.subscribe(args...);
        }
};

}

