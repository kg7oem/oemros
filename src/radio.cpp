/*
 * radio.cpp
 *
 *  Created on: Jul 6, 2018
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

#include <cassert>

#include "logging.h"
#include "radio.h"
#include "system.h"

using namespace hamlib;

namespace oemros {

void radio_bootstrap(void) {
    rig_set_debug_callback([](enum rig_debug_level_e debug_level, UNUSED rig_ptr_t user_data, UNUSED const char *fmt, UNUSED va_list arg) -> int {
        return true;
    }, NULL);
}

radio::radio(hamlib::rig_model_t model_arg)
: hl_model(model_arg) {
    assert(this->hl_rig == NULL);

    log_trace("constructing a new radio object with hamlib rig id = ", this->hl_model);
    this->hl_rig = rig_init(this->hl_model);
    if (! this->hl_rig) {
        log_fatal("could not call rig_init()");
    }

    int result = rig_open(this->hl_rig);
    if (result != RIG_OK) {
        log_fatal("could not call rig_open()");
    }

    log_trace("finished initializing hamlib");
}

std::shared_ptr<oemros::promise<freq_t>> radio::frequency(void) {
    log_trace("Going to read the frequency from hamlib");
    auto promise = make_promise<freq_t>([&] {
            freq_t cur_freq;
            int result = rig_get_freq(this->hl_rig, RIG_VFO_CURR, &cur_freq);
            if (result != RIG_OK) {
                log_fatal("rig_get_freq() failed (and this should really do an exception for the future");
            }
            log_trace("back from hamlib");
            return cur_freq;
    });

    return promise;
}

}
