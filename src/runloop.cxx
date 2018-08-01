/*
 * runloop.cxx
 *
 *  Created on: Jul 31, 2018
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

#include <boost/bind.hpp>

#include "logging.h"
#include "runloop.h"
#include "system.h"

namespace oemros {

void runloop::enter() {
    io.run();
}

void runloop_item::start() {
    start__child();
}

boost::asio::io_service* runloop_item::get_loop_ioptr() {
    return &loop->io;
}

void oneshot_timer::handler(const boost::system::error_code& error) {
    if (error) {
        system_fault("timer failed");
    }

    log_debug("Handler ran");
}

void oneshot_timer::start__child() {
    // do the time calculation as soon as possible
    asio_timer.expires_from_now(boost::posix_time::millisec(initial.count()));
    auto bound = boost::bind(&oneshot_timer::handler, this, boost::asio::placeholders::error);
    asio_timer.async_wait(bound);
}

}
