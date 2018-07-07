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
    int result = uv_run(&this->uv_loop, UV_RUN_DEFAULT);
    log_debug("got control back from uv_run()");

    assert(result == 0);
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
    assert(this->loop != NULL);

    log_trace("constructed a new runloop item #", this->item_id, " ", this->description());
}

rlitem::~rlitem() {
    log_trace("deconstructing a runloop item");

    if (this->state != rlitemstate::closed) {
        log_fatal("attempt to deconstruct a runloop item that was not stopped: state = ", (int)this->state);
    }
}

rlitem_s rlitem::get_shared(void) {
    return this->get_shared__child();
}

uv_loop_t* rlitem::get_uv_loop(void) {
    return this->loop->get_uv_loop();
}

void rlitem::start(void) {
    log_trace("starting runloop item #", this->item_id);

    this->will_start();
    this->uv_start();
    this->loop->add_item(this->get_shared());
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
    this->will_close();

    this->state = rlitemstate::closing;
    this->uv_close();

    // the rest of the closing workflow
    // happens via a callback from libuv
    // that invokes this->close_resume()
}

void rlitem::close_resume(void) {
    log_trace("continuing on with the close workflow");
    this->state = rlitemstate::closed;
    this->loop->remove_item(this->get_shared());
    this->did_close();
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
    log_trace("adding myself to the uv runloop");
    this->get_uv_handle()->data = this;
    assert(uv_timer_init(this->get_uv_loop(), &this->uv_timer) == 0);

    log_trace("starting the libuv timer; initial=", this->initial, " repeat=", this->repeat);

    uv_timer_start(&this->uv_timer, [](uv_timer_t* uv_timer) {
        log_trace("inside the lambda");
        auto us = static_cast<rltimer*>(uv_timer->data);
        log_trace("going to execute timer callback for #", us->item_id);
        us->execute();

        if (us->repeat == 0) {
            us->stop();
        }
    }, this->initial, this->repeat);
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
