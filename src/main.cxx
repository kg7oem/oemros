/*
 * main.cxx
 *
 *  Created on: Jul 21, 2018
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

#include <memory>

#include "logging.h"
#include "object.h"
#include "radio.h"
#include "runloop.h"
#include "system.h"

using std::make_shared;
using namespace std::chrono_literals;

struct biddle {
    void ok() { log_debug("biddle ok!"); }
};

void run() {
    auto loop = make_shared<oemros::runloop>();
    auto repeat = loop->make_started<oemros::oneshot_timer>(2s);
    biddle abiddle;

    loop->post([] { log_debug("I am posted"); });
    loop->post(&biddle::ok, abiddle);

    loop->enter();
}

void bootstrap() {
    auto logging = logjam::logengine::get_engine();
    auto test_dest = make_shared<oemros::log_console>(logjam::loglevel::debug);

    logging->add_destination(test_dest);
    logging->start();
}

int main() {
    bootstrap();

    log_debug("Starting OEMROS");

    try {
        run();
    } catch (oemros::fault& fault) {
        log_error("OEMROS faulted: ", fault.what());
    }

    log_debug("Exiting OEMROS with fault state: ", (int)oemros::get_fault_state());
    oemros::exit_fault_state();
}

