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

#include <functional>
#include <set>
#include <list>

namespace libuv {
#include <uv.h>
}

#include "system.h"

namespace oemros {

enum class rlitemstate {
    unstarted = 0,
    started,
    stopped,
    closing,
    closed,
};

typedef void(*runloopcb_t)(void);
typedef std::function<void (void)> runloopcb_f;

class rlitem;

REFCOUNTED(runloop, public std::enable_shared_from_this<runloop>) {
    friend class rlitem;

    private:
        libuv::uv_loop_t uv_loop;
        uint64_t prev_item_id = 0;
        libuv::uv_loop_t* get_uv_loop(void);
        std::set<std::shared_ptr<rlitem>> active_items;
        void add_item(std::shared_ptr<rlitem>);
        void remove_item(std::shared_ptr<rlitem>);

    public:
        runloop();
        ~runloop();
        template<typename... Args>
        static runloop_s create(Args... args) {
            return std::make_shared<runloop>(args...);
        }
        template<typename T, typename... Args>
        std::shared_ptr<T> create_item(Args... args) {
            return T::create(this->shared_from_this(), args...);
        }
        void enter(void);
};

// runloop items (rlitem) are things that are managed
// by the runloop
REFCOUNTED(rlitem) {
    friend class runloop;

    private:
        rlitemstate state = rlitemstate::unstarted;
        bool autoclose = true;

    protected:
        const runloop_s loop = NULL;
        const uint64_t item_id = 0;
        libuv::uv_loop_t* get_uv_loop(void);
        virtual libuv::uv_handle_t* get_uv_handle(void) = 0;
        virtual void will_start(void) { };
        virtual void uv_start(void) = 0;
        virtual void did_start(void) { };
        virtual void will_stop(void) { };
        virtual void uv_stop(void) = 0;
        virtual void did_stop(void) { };
        virtual void will_close(void) { };
        virtual void uv_close(void) = 0;
        virtual void did_close(void) { };
//        virtual uv_handle_t* uv_handle(void) = 0;

    public:
        // FIXME if this is made protected then it can't be
        // run by the allocator - how does that get fixed?
        rlitem() = default;
        rlitem(runloop_s);
        ~rlitem();
        rlitem_s get_shared(void);
        virtual rlitem_s get_shared__child(void) = 0;
        void start(void);
        void stop(void);
        void close(void);
        void close_resume(void);
};

REFCOUNTED(rlonce, public rlitem, public std::enable_shared_from_this<rlonce>) {
    private:
        libuv::uv_idle_t uv_idle;

    protected:
        runloopcb_f cb = NULL;

    public:
        rlonce(runloop_s, runloopcb_f);
        template<typename... Args>
        static rlonce_s create(Args... args) {
            return std::make_shared<rlonce>(args...);
        }
        virtual rlitem_s get_shared__child(void);
        libuv::uv_handle_t* get_uv_handle(void);
        virtual void uv_start(void);
        virtual void uv_stop(void);
        virtual void uv_close(void);
        void execute(void);
};

}

#endif /* SRC_RUNLOOP_H_ */
