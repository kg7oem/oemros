/*
 * runloop.cpp
 *
 *  Created on: Jul 1, 2018
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

#include <thread>

#include "logging.h"
#include "runloop.h"

namespace oemros {

// shorter function name for internal use, does not need
// to follow naming guidelines
static runloop* get_loop(void) {
    static thread_local runloop* thread_singleton = NULL;

    if (thread_singleton == NULL) {
        thread_singleton = new runloop();
    }

    return thread_singleton;
}

// public function that follows naming convention
runloop* runloop_get(void) {
    return get_loop();
}

void runloop_enter(void) {
    get_loop()->enter();
}

runloop::runloop() {
    if (uv_loop_init(&this->uv_loop) != 0) {
        log_fatal("could not initialize uv runloop");
    }

    log_trace("built a runloop");
}

runloop::~runloop() {
    if (uv_loop_close(&this->uv_loop) == UV_EBUSY) {
        log_fatal("could not cleanup runloop because it was not empty");
    }

    log_trace("destroyed a runloop");
}

void runloop::enter(void) {
    auto loop = get_loop();

    log_debug("giving control to libuv");
    bool retval = uv_run(&loop->uv_loop, UV_RUN_DEFAULT);
    log_debug("got control back from libuv; reval=", retval);
}

std::list<runloopitem*> runloop::get_items(void) {
    log_trace("starting to enumerate runloop items in uv");
    std::list<runloopitem*> item_list;

    uv_walk(&this->uv_loop, [](uv_handle_t* handle, void* arg){
        auto&& item_list = static_cast<std::list<runloopitem*>*>(arg);
        runloopitem* item = static_cast<runloopitem*>(handle->data);
        item_list->push_back(item);
    }, &item_list);

    log_trace("found ", item_list.size(), " items in the runloop");

    return item_list;
}

runloopitem::runloopitem(runloop* loop)
: loop(loop), id(++loop->prev_item_id) {
    log_trace("finished constructing runloop item #", this->id);
}

std::ostream& operator<<(std::ostream& os, const runloopitem& item) {
    os << "THIS------> " << typeid(item).name() << " <---- THAT";
    return os;
}

void runloopitem::init(void) {
    // FIXME this needs to undergo automatic memory management
    // and be refcounted as it goes in and out of the uv_handle
    uv_handle()->data = this;
}

uv_handle_t* runloopitem::uv_handle(void) {
    log_fatal("this method must be overloaded");
}

uv_loop_t* runloopitem::get_uvloop(void) {
    return &this->loop->uv_loop;
}

void runloopitem::start(void) {
    log_trace("setting stopped to be false");
    this->stopped = false;
}

void runloopitem::stop(void) {
    log_trace("setting stopped to be true");
    this->stopped = true;
}

runlooptimer::runlooptimer(runloop* loop, uint64_t initial)
: runloopitem(loop), initial(initial)
{
    log_trace("constructing with initial=", initial);
    this->init();
}
runlooptimer::runlooptimer(runloop* loop, uint64_t initial, uint64_t repeat)
: runloopitem(loop), initial(initial), repeat(repeat)
{
    log_trace("constructing with initial=", initial, "; repeat=", repeat);
    this->init();
}

runlooptimer::runlooptimer(runloop* loop, uint64_t initial, runloop_cb_t cb)
: runloopitem(loop), cb(cb), initial(initial) {
    log_trace("constructing with initial=", initial);
    this->init();
}

uv_handle_t* runlooptimer::uv_handle(void) {
    return (uv_handle_t*)&this->uv_timer;
}

void runlooptimer::init(void) {
    runloopitem::init();

    log_trace("initializing libuv parts of timer");
    uv_timer_init(this->get_uvloop(), &this->uv_timer);
}

void runlooptimer::start(void) {
    log_trace("starting timer");

    runloopitem::start();

    uv_timer_start(
            &this->uv_timer,
            [](uv_timer_t *arg) {
                runlooptimer* timer = static_cast<runlooptimer*>(arg->data);
                timer->execute(); },
            this->initial, this->repeat);

}

void runlooptimer::stop(void) {
    runloopitem::stop();
}

void runlooptimer::execute(void) {
    log_trace("executing a timer callback");

    if (this->cb == NULL) {
        log_trace("callback was null, not doing anything");
        return;
    }

    this->cb();
}

}

