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

#include <csignal>
#include <functional>

namespace libuv {
#include <uv.h>
}

#include "object.h"
#include "system.h"

namespace oemros {

enum class rlitemstate {
    unstarted = 0,
    started,
    stopped,
    closing,
    closed,
};

typedef void(*runloopcb_t)();
typedef std::function<void (void)> runloop_cb;

class rlitem;

OBJECT(runloop) {
    friend class rlitem;

    OBJSTUFF(runloop);

    private:
        bool inside_runloop = false;
        libuv::uv_loop_t uv_loop;
        uint64_t prev_item_id = 0;
        libuv::uv_loop_t* get_uv_loop();
        set<strong_ptr<rlitem>> active_items;
        void add_item(strong_ptr<rlitem>);
        void remove_item(strong_ptr<rlitem>);
        list<libuv::uv_handle_t*> get_handles();

    public:
        runloop();
        ~runloop();
        template<typename T, typename... Args>
        strong_ptr<T> make_item(Args&&...args) {
            log_trace("creating a new item from inside the runloop");
            return T::make(strong_ref(), args...);
        }
        template<typename T, typename... Args>
        strong_ptr<T> make_started(Args&&...args) {
            auto new_item = make_item<T>(args...);
            log_trace("starting the new item");
            new_item->start();
            log_trace("done starting the new item");
            return new_item;
        }
        void enter();
        void shutdown();
};

// runloop items (rlitem) are things that are managed
// by the runloop
ABSTRACT(rlitem) {
    ABSSTUFF(rlitem);

    friend class runloop;

    private:
        const runloop_w loop;
        rlitemstate state = rlitemstate::unstarted;
        bool autoclose = true;

    protected:
        const uint64_t item_id = 0;
        runloop_s get_loop();
        libuv::uv_loop_t* get_uv_loop();
        virtual libuv::uv_handle_t* get_uv_handle() = 0;
        virtual void will_start() { };
        virtual void uv_start() = 0;
        virtual void did_start() { };
        virtual void will_stop() { };
        virtual void uv_stop() = 0;
        virtual void did_stop() { };
        virtual void will_close() { };
        virtual void uv_close() = 0;
        virtual void did_close() { };
//        virtual uv_handle_t* uv_handle() = 0;

    public:
        // FIXME if this is made protected then it can't be
        // run by the allocator - how does that get fixed?
        rlitem() = default;
        virtual ~rlitem();
        rlitem(runloop_s);
        rlitem_s get_shared();
        virtual rlitem_s get_shared__child() = 0;
        void start();
        void stop();
        void close();
        void close_resume();
};

// CLEANUP rename this to rljob
OBJECT(rlonce, public rlitem) {
    OBJSTUFF(rlonce);

    private:
        libuv::uv_idle_t uv_idle;

    protected:
        const runloop_cb cb = NULL;

    public:
        rlonce(runloop_s, runloop_cb);
        virtual rlitem_s get_shared__child() override;
        virtual libuv::uv_handle_t* get_uv_handle() override;
        virtual void uv_start() override;
        virtual void uv_stop() override;
        virtual void uv_close() override;
        void execute();
};

OBJECT(rltimer, public rlitem) {
    OBJSTUFF(rltimer);

    private:
        libuv::uv_timer_t uv_timer;

    protected:
        const runloop_cb cb = NULL;
        bool check_intervals() const;

    public:
        const uint64_t initial = 0;
        const uint64_t repeat = 0;
        rltimer(runloop_s, uint64_t, runloop_cb);
        rltimer(runloop_s, uint64_t, uint64_t, runloop_cb);
        virtual rlitem_s get_shared__child() override;
        virtual libuv::uv_handle_t* get_uv_handle() override;
        virtual void uv_start() override;
        virtual void uv_stop() override;
        virtual void uv_close() override;
        void execute();
};

OBJECT(rlsignal, public rlitem) {
    OBJSTUFF(rlsignal);

    private:
        libuv::uv_signal_t uv_signal;

    protected:
        const runloop_cb cb = NULL;

    public:
        const int signum;
        rlsignal(runloop_s, int, runloop_cb);
        virtual rlitem_s get_shared__child() override;
        virtual libuv::uv_handle_t* get_uv_handle() override;
        virtual void uv_start() override;
        virtual void uv_stop() override;
        virtual void uv_close() override;
        void execute();
};

}

#endif /* SRC_RUNLOOP_H_ */
