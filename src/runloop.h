/*
 * runloop.h
 *
 *  Created on: Jul 3, 2018
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

#ifndef SRC_RUNLOOP_H_
#define SRC_RUNLOOP_H_

#include <list>
#include <uv.h>

#include "system.h"

namespace oemros {

class rlitem;

REFCOUNTED(runloop) {
    friend class rlitem;

    private:
        uv_loop_t uv_loop;
        uint64_t prev_item_id = 0;

    public:
        runloop();
        ~runloop();
        template<typename... Args>
        static runloop_s create(Args... args) {
            return std::make_shared<runloop>(args...);
        }
        template<typename T, typename... Args>
        std::shared_ptr<T> create_item(Args... args) { return T::create(this->shared_from_this(), args...); }
};

// runloop items (rlitem) are things that are managed
// by the runloop
REFCOUNTED(rlitem) {
    friend class runloop;

    private:
        const uint64_t item_id = 0;

    protected:
        const runloop_s loop = NULL;
//        virtual uv_handle_t* uv_handle(void) = 0;

    public:
        // FIXME if this is made protected then it can't be
        // run by the allocator - how does that get fixed?
        rlitem() = default;
        rlitem(runloop_s);
        ~rlitem();
};

REFCOUNTED(rlonce, public rlitem) {
    private:
        uv_idle_t* uv_idle = NULL;

    protected:
//        virtual uv_handle_t* uv_handle(void);

    public:
        rlonce(runloop_s);
        template<typename... Args>
        static rlonce_s create(Args... args) {
            return std::make_shared<rlonce>(args...);
        }
};

}

#endif /* SRC_RUNLOOP_H_ */
