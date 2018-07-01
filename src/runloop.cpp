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

//static runloop* get_loop(void) {
//    static thread_local runloop* thread_singleton = NULL;
//
//    if (thread_singleton == NULL) {
//        thread_singleton = new runloop();
//    }
//
//    return thread_singleton;
//}

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

}


