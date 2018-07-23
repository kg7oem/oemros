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
#include "system.h"

using std::make_shared;

void run() {
    auto logging = logjam::logengine::get_engine();
    auto test_dest = make_shared<oemros::log_console>(logjam::loglevel::debug);

    logging->add_destination(test_dest);
    logging->start();
}

int main() {
    try {
        run();
    } catch (oemros::fault& fault) {
        log_error("OEMROS faulted: ", fault.what());
    }

    oemros::exit_fault_state();
}

