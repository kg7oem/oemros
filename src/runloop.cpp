/*
 * runloop.cpp
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

#include "logging.h"
#include "runloop.h"
#include "system.h"

using namespace libuv;

namespace oemros {

runloop::runloop() {
    log_trace("constructing a runloop");

    int result = uv_loop_init(&uv_loop);

    if (result) {
        system_panic("uv_loop_init() failed");
    }
}

runloop::~runloop() {
    log_trace("deconstructing a runloop");

    if (size_t size = active_items.size() != 0) {
        log_fatal("attempt stop a runloop with items in the active items set: size = ", size);
    }

    int result = uv_loop_close(&uv_loop);

    if (result) {
        if (result == UV_EBUSY) {
            log_fatal("uv_loop_close() failed because the runloop was not empty");
        } else {
            log_fatal("uv_loop_close() failed; result=", result);
        }
    }
}

uv_loop_t* runloop::get_uv_loop() {
    return &uv_loop;
}

void runloop::enter() {
    log_debug("giving uv_run() control of the thread");
    inside_runloop = true;
    int result = uv_run(&uv_loop, UV_RUN_DEFAULT);
    inside_runloop = false;
    log_debug("got control back from uv_run()");

    assert(result == 0);
}

void runloop::shutdown() {
    log_debug("shutting down the runloop");

    for(auto&& i : active_items) {
        log_trace("stopping ", i->description());
        i->stop();

        if (! i->autoclose) {
            log_trace("autoclose is not enabled so closing explicitly");
            i->close();
        }
    }

    if (! inside_runloop) {
        // give the libuv close callbacks a chance to run
        enter();
    }
}

list<uv_handle_t*> runloop::get_handles() {
    log_trace("generating a list of handles in the libuv runloop");
    list<uv_handle_t*> uv_handles;

    uv_walk(&uv_loop, [](uv_handle_t* handle, void* arg) -> void {
        auto list_arg = static_cast<list<uv_handle_t*>*>(arg);
        list_arg->push_front(handle);
    }, &uv_handles);

    log_trace("found ", uv_handles.size(), " handles in the libuv runloop");
    return uv_handles;
}

void runloop::add_item(rlitem_s item) {
    log_trace("adding runloop item to the active list: #", item->item_id);
    active_items.insert(item);
}

void runloop::remove_item(rlitem_s item) {
    log_trace("removing runloop item from the active list: #", item->item_id);
    assert(active_items.erase(item) == 1);
}

rlitem::rlitem(runloop_s loop_arg)
: loop(loop_arg), item_id(++loop_arg->prev_item_id) {
    assert(loop.expired() == false);

    log_trace("constructed a new runloop item #", item_id, " with type of ", type());
}

rlitem::~rlitem() {
    log_trace("deconstructing a runloop item");

    if (state != rlitemstate::closed) {
        log_fatal("attempt to deconstruct a runloop item that was not stopped: state = ", (int)state);
    }
}

runloop_s rlitem::get_loop() {
    assert(loop.expired() == false);
    return loop.lock();
}

rlitem_s rlitem::get_shared() {
    return get_shared__child();
}

uv_loop_t* rlitem::get_uv_loop() {
    return get_loop()->get_uv_loop();
}

void rlitem::start() {
    log_trace("starting runloop item #", item_id);

    will_start();
    uv_start();
    get_loop()->add_item(get_shared());
    state = rlitemstate::started;
    did_start();
}

void rlitem::stop() {
    log_trace("stopping runloop item #", item_id);

    will_stop();
    uv_stop();
    state = rlitemstate::stopped;
    did_stop();

    if (autoclose) {
        close();
    }
}

void rlitem::close() {
    assert(state == rlitemstate::stopped);

    will_close();

    state = rlitemstate::closing;
    uv_close();

    // the rest of the closing workflow
    // happens via a callback from libuv
    // that invokes close_resume()
}

void rlitem::close_resume() {
    log_trace("continuing on with the close workflow");
    // hold a local copy so the object is guranteed to be alive
    // after it is removed from the active list
    rlitem_s us = get_shared();
    us->state = rlitemstate::closed;
    us->get_loop()->remove_item(us);
    us->did_close();
}

rlonce::rlonce(runloop_s loop_arg, runloopcb_f cb_arg)
: rlitem(loop_arg), cb(cb_arg) {
    if (cb == NULL) {
        log_fatal("callback pointer was NULL");
    }
}

rlitem_s rlonce::get_shared__child() {
    return strong_ref();
}

uv_handle_t* rlonce::get_uv_handle() {
    return (uv_handle_t*)&uv_idle;
}

void rlonce::uv_start() {
    log_trace("adding myself to the uv runloop");
    get_uv_handle()->data = this;
    assert(uv_idle_init(get_uv_loop(), &uv_idle) == 0);

    log_trace("starting the libuv handle");
    uv_idle_start(&uv_idle, [](uv_idle_t* uv_idle_arg){
        log_trace("inside the lambda function");

        rlonce* item = static_cast<rlonce*>(uv_idle_arg->data);
        log_trace("invoking execute() on object");
        item->execute();
        item->stop();
    });
}

void rlonce::uv_stop() {
    log_trace("stopping libuv handle for item #", item_id);
    uv_idle_stop(&uv_idle);
}

void rlonce::uv_close() {
    log_trace("closing libuv handle for item #", item_id);

    libuv::uv_close(get_uv_handle(), [](uv_handle_t* handle) -> void {
        log_trace("inside the lambda");
        rlonce* item = static_cast<rlonce*>(handle->data);
        item->close_resume();
    });
}

void rlonce::execute() {
    log_trace("inside the execute handler for #", item_id);
    cb();
}

rltimer::rltimer(runloop_s loop_arg, uint64_t initial_arg, runloopcb_f cb_arg)
: rlitem(loop_arg), cb(cb_arg), initial(initial_arg) {
    if (! check_intervals()) {
        log_fatal("invalid intervals given to the timer");
    }
}

rltimer::rltimer(runloop_s loop_arg, uint64_t initial_arg, uint64_t repeat_arg, runloopcb_f cb_arg)
: rlitem(loop_arg), cb(cb_arg), initial(initial_arg), repeat(repeat_arg) {
    if (! check_intervals()) {
        log_fatal("invalid intervals given to the timer");
    }
}

bool rltimer::check_intervals() const {
    if (initial == 0 && repeat == 0) {
        return false;
    }

    return true;
}

rlitem_s rltimer::get_shared__child() {
    return strong_ref();
}

uv_handle_t* rltimer::get_uv_handle() {
    return (uv_handle_t*)&uv_timer;
}

void rltimer::execute() {
    log_trace("inside the execute handler for #", item_id);
    cb();
}

void rltimer::uv_start() {
    uint64_t uv_initial, uv_repeat = 0;

    if (initial == 0) {
        uv_initial = uv_repeat = repeat;
    } else {
        uv_initial = initial;
        uv_repeat = repeat;
    }

    log_trace("adding myself to the uv runloop");
    get_uv_handle()->data = this;
    assert(uv_timer_init(get_uv_loop(), &uv_timer) == 0);

    log_trace("starting the libuv timer; initial=", initial, " repeat=", repeat);
    log_trace("uv timer values; initial = ", uv_initial, " repeat = ", uv_repeat);

    uv_timer_start(&uv_timer, [](uv_timer_t* uv_timer_arg) {
        log_trace("inside the lambda");
        auto us = static_cast<rltimer*>(uv_timer_arg->data);
        log_trace("going to execute timer callback for #", us->item_id);
        us->execute();

        if (us->repeat == 0) {
            us->stop();
        }
    }, uv_initial, uv_repeat);
}

void rltimer::uv_stop() {
    log_trace("stopping libuv timer");
    uv_timer_stop(&uv_timer);
}

void rltimer::uv_close() {
    log_trace("closing libuv timer");

    libuv::uv_close(get_uv_handle(), [](uv_handle_t* handle) -> void {
        log_trace("inside the lambda");
        static_cast<rltimer*>(handle->data)->close_resume();
    });
}

}
