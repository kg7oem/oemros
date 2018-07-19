/*
 * event.h
 *
 *  Created on: Jul 18, 2018
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

#ifndef SRC_EVENT_H_
#define SRC_EVENT_H_

#include "system.h"

namespace oemros {

uint64_t event_next_subscription_id(void);

template <class... T>
class event_source {
    // forward declare some things
    public:
        using sink = std::function<void (T...)>;
        struct subscription;

    private:
        vector<subscription> subscribers;
        event_source(const event_source&) = delete;
        event_source(const event_source&&) = delete;
        event_source& operator=(const event_source&) = delete;

    public:
        event_source() = default;
        subscription subscribe(sink cb) {
            subscription new_subscription(cb);
            subscribers.push_back(new_subscription);
            return new_subscription;
        }
        void unsubscribe(subscription sub_in);
        template <typename... Args>
        void deliver(Args... args) {
            for (auto&& i : subscribers) {
                // FIXME should this use std::forward?
                i.handler(args...);
            }
        }

        struct subscription {
            // FIXME how can this be moved into a member of event_source?
            const uint64_t id = event_next_subscription_id();
            const sink handler;

            subscription(sink& handler_in) : handler(handler_in) { }
        };
};

}


#endif /* SRC_EVENT_H_ */
