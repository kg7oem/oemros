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

namespace oemros {

rlitem::rlitem(runloop_s loop_arg)
: loop(loop_arg), item_id(++loop_arg->prev_item_id) {
    assert(this->loop != NULL);

    log_trace("constructed a new runloop item #", this->item_id);
}

rlitem::~rlitem() {
    log_trace("deconstructed a runloop item");
}

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

}

