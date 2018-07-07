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

#include <typeinfo>

#include <cassert>
#include "logging.h"
#include "runloop.h"

using namespace libuv;

namespace oemros {

runloop::runloop() {
    log_trace("constructing a runloop");

    int result = uv_loop_init(&this->uv_loop);

    if (result) {
        system_panic("uv_loop_init() failed");
    }
}

runloop::~runloop() {
    log_trace("deconstructing a runloop");

    if (size_t size = this->active_items.size() != 0) {
        log_fatal("attempt stop a runloop with items in the active items set: size = ", size);
    }

    int result = uv_loop_close(&this->uv_loop);

    if (result) {
        if (result == UV_EBUSY) {
            log_fatal("uv_loop_close() failed because the runloop was not empty");
        } else {
            log_fatal("uv_loop_close() failed; result=", result);
        }
    }
}

uv_loop_t* runloop::get_uv_loop(void) {
    return &this->uv_loop;
}

void runloop::enter(void) {
    log_debug("giving uv_run() control of the thread");
    this->inside_runloop = true;
    int result = uv_run(&this->uv_loop, UV_RUN_DEFAULT);
    this->inside_runloop = false;
    log_debug("got control back from uv_run()");

    assert(result == 0);
}

void runloop::shutdown(void) {
    log_debug("shutting down the runloop");

    for(auto&& i : this->active_items) {
        log_trace("stopping ", i->description());
        i->stop();

        if (! i->autoclose) {
            log_trace("autoclose is not enabled so closing explicitly");
            i->close();
        }
    }

    if (! this->inside_runloop) {
        // give the libuv close callbacks a chance to run
        this->enter();
    }
}

std::list<uv_handle_t*> runloop::get_handles(void) {
    log_trace("generating a list of handles in the libuv runloop");
    std::list<uv_handle_t*> list;

    uv_walk(&this->uv_loop, [](uv_handle_t* handle, void* arg) -> void {
        auto list = static_cast<std::list<uv_handle_t*>*>(arg);
        list->push_front(handle);
    }, &list);

    log_trace("found ", list.size(), " handles in the libuv runloop");
    return list;
}

void runloop::add_item(rlitem_s item) {
    log_trace("adding runloop item to the active list: #", item->item_id);
    this->active_items.insert(item);
}

void runloop::remove_item(rlitem_s item) {
    log_trace("removing runloop item from the active list: #", item->item_id);
    assert(this->active_items.erase(item) == 1);
}

rlitem::rlitem(runloop_s loop_arg)
: loop(loop_arg), item_id(++loop_arg->prev_item_id) {
    assert(this->loop.expired() == false);

    log_trace("constructed a new runloop item #", this->item_id, " ", this->description());
}

rlitem::~rlitem() {
    log_trace("deconstructing a runloop item");

    if (this->state != rlitemstate::closed) {
        log_fatal("attempt to deconstruct a runloop item that was not stopped: state = ", (int)this->state);
    }
}

runloop_s rlitem::get_loop(void) {
    assert(this->loop.expired() == false);
    return this->loop.lock();
}

rlitem_s rlitem::get_shared(void) {
    return this->get_shared__child();
}

uv_loop_t* rlitem::get_uv_loop(void) {
    return this->get_loop()->get_uv_loop();
}

void rlitem::start(void) {
    log_trace("starting runloop item #", this->item_id);

    this->will_start();
    this->uv_start();
    this->get_loop()->add_item(this->get_shared());
    this->state = rlitemstate::started;
    this->did_start();
}

void rlitem::stop(void) {
    log_trace("stopping runloop item #", this->item_id);

    this->will_stop();
    this->uv_stop();
    this->state = rlitemstate::stopped;
    this->did_stop();

    if (this->autoclose) {
        this->close();
    }
}

void rlitem::close(void) {
    assert(this->state == rlitemstate::stopped);

    this->will_close();

    this->state = rlitemstate::closing;
    this->uv_close();

    // the rest of the closing workflow
    // happens via a callback from libuv
    // that invokes this->close_resume()
}

void rlitem::close_resume(void) {
    log_trace("continuing on with the close workflow");
    // hold a local copy so the object is guranteed to be alive
    // after it is removed from the active list
    rlitem_s us = this->get_shared();
    us->state = rlitemstate::closed;
    us->get_loop()->remove_item(us);
    us->did_close();
}

rlonce::rlonce(runloop_s loop_arg, runloopcb_f cb_arg)
: rlitem(loop_arg), cb(cb_arg) {
    if (this->cb == NULL) {
        log_fatal("callback pointer was NULL");
    }
}

rlitem_s rlonce::get_shared__child(void) {
    return this->shared_from_this();
}

uv_handle_t* rlonce::get_uv_handle(void) {
    return (uv_handle_t*)&this->uv_idle;
}

void rlonce::uv_start(void) {
    log_trace("adding myself to the uv runloop");
    this->get_uv_handle()->data = this;
    assert(uv_idle_init(this->get_uv_loop(), &this->uv_idle) == 0);

    log_trace("starting the libuv handle");
    uv_idle_start(&this->uv_idle, [](uv_idle_t* uv_idle){
        log_trace("inside the lambda function");

        rlonce* item = static_cast<rlonce*>(uv_idle->data);
        log_trace("invoking execute() on object");
        item->execute();
        item->stop();
    });
}

void rlonce::uv_stop(void) {
    log_trace("stopping libuv handle for item #", this->item_id);
    uv_idle_stop(&this->uv_idle);
}

void rlonce::uv_close(void) {
    log_trace("closing libuv handle for item #", this->item_id);

    libuv::uv_close(this->get_uv_handle(), [](uv_handle_t* handle) -> void {
        log_trace("inside the lambda");
        rlonce* item = static_cast<rlonce*>(handle->data);
        item->close_resume();
    });
}

void rlonce::execute(void) {
    log_trace("inside the execute handler for #", this->item_id);
    this->cb();
}

rltimer::rltimer(runloop_s loop_arg, uint64_t initial_arg, runloopcb_f cb_arg)
: rlitem(loop_arg), cb(cb_arg), initial(initial_arg) {
    if (! this->check_intervals()) {
        log_fatal("invalid intervals given to the timer");
    }
}

rltimer::rltimer(runloop_s loop_arg, uint64_t initial_arg, uint64_t repeat_arg, runloopcb_f cb_arg)
: rlitem(loop_arg), cb(cb_arg), initial(initial_arg), repeat(repeat_arg) {
    if (! this->check_intervals()) {
        log_fatal("invalid intervals given to the timer");
    }
}

bool rltimer::check_intervals(void) const {
    if (this->initial == 0 && this->repeat == 0) {
        return false;
    }

    return true;
}

rlitem_s rltimer::get_shared__child(void) {
    return this->shared_from_this();
}

uv_handle_t* rltimer::get_uv_handle(void) {
    return (uv_handle_t*)&this->uv_timer;
}

void rltimer::execute(void) {
    log_trace("inside the execute handler for #", this->item_id);
    this->cb();
}

void rltimer::uv_start(void) {
    uint64_t uv_initial, uv_repeat = 0;

    if (this->initial == 0) {
        uv_initial = uv_repeat = this->repeat;
    } else {
        uv_initial = this->initial;
        uv_repeat = this->repeat;
    }

    log_trace("adding myself to the uv runloop");
    this->get_uv_handle()->data = this;
    assert(uv_timer_init(this->get_uv_loop(), &this->uv_timer) == 0);

    log_trace("starting the libuv timer; initial=", this->initial, " repeat=", this->repeat);
    log_trace("uv timer values; initial = ", uv_initial, " repeat = ", uv_repeat);

    uv_timer_start(&this->uv_timer, [](uv_timer_t* uv_timer) {
        log_trace("inside the lambda");
        auto us = static_cast<rltimer*>(uv_timer->data);
        log_trace("going to execute timer callback for #", us->item_id);
        us->execute();

        if (us->repeat == 0) {
            us->stop();
        }
    }, uv_initial, uv_repeat);
}

void rltimer::uv_stop(void) {
    log_trace("stopping libuv timer");
    uv_timer_stop(&this->uv_timer);
}

void rltimer::uv_close(void) {
    log_trace("closing libuv timer");

    libuv::uv_close(this->get_uv_handle(), [](uv_handle_t* handle) -> void {
        log_trace("inside the lambda");
        static_cast<rltimer*>(handle->data)->close_resume();
    });
}

}
