/*
 * hamlib.cxx
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

#include <cassert>

#include "hamlib.h"
#include "logging.h"

namespace oemros {

void hamlib_bootstrap() {
    hamlib::rig_set_debug_level(hamlib::RIG_DEBUG_NONE);
}

hamlib_error::hamlib_error(int error_num_in)
: exception(hamlib::rigerror(error_num_in)), error_num(error_num_in) { }

hamlib_rig::~hamlib_rig() {
    if (hl_rig != nullptr) {
        rig_close(hl_rig);
        rig_cleanup(hl_rig);

        hl_rig = nullptr;
    }
}

bool hamlib_rig::open() {
    hl_rig = hamlib::rig_init(model);

    if (hl_rig == nullptr) {
        return false;
    }

    auto retcode = hamlib::rig_open(hl_rig);
    return retcode;
}

hamlib_result<float> hamlib_rig::get_alc(hamlib_rig::vfo_type vfo_in) {
    assert(hl_rig != nullptr);

    hamlib::value_t buf;
    auto retval = hamlib::rig_get_level(hl_rig, vfo_in, hamlib::RIG_LEVEL_ALC, &buf);
    return make_hamlib_result(retval, buf.f);
}

hamlib_result<hamlib_rig::freq_type>
hamlib_rig::get_freq(hamlib_rig::vfo_type vfo_in) {
    assert(hl_rig != nullptr);

    hamlib_rig::freq_type buf;
    auto retval = hamlib::rig_get_freq(hl_rig, vfo_in, &buf);
    return make_hamlib_result(retval, buf);
}

hamlib_result<int> hamlib_rig::get_strength(hamlib_rig::vfo_type vfo_in) {
    assert(hl_rig != nullptr);

    hamlib::value_t buf;
    auto retval = hamlib::rig_get_level(hl_rig, vfo_in, hamlib::RIG_LEVEL_STRENGTH, &buf);
    return make_hamlib_result(retval, buf.i);
}

hamlib_result<float> hamlib_rig::get_swr(hamlib_rig::vfo_type vfo_in) {
    assert(hl_rig != nullptr);

    hamlib::value_t buf;
    auto retval = hamlib::rig_get_level(hl_rig, vfo_in, hamlib::RIG_LEVEL_SWR, &buf);
    return make_hamlib_result(retval, buf.f);
}

hamlib_radio::hamlib_radio(std::shared_ptr<runloop> loop_in, const hamlib::rig_model_t& model_in)
: radio(loop_in), rig(new hamlib_rig(model_in)) { }

hamlib_radio::~hamlib_radio() {
    if (rig != nullptr) {
        delete rig;
        rig = nullptr;
    }
}

bool hamlib_radio::open() {
    assert(rig != nullptr);
    return rig->open();
}

void hamlib_radio::update__alc() {
    assert(rig != nullptr);

    auto result = rig->get_alc();
    if (result) {
        meters.alc = result.value;
    } else {
        log_error("could not get ALC from hamlib: ", result.error_str());
    }
}

void hamlib_radio::update__power() {
    assert(rig != nullptr);

    log_error("can not update power meter from hamlib yet");
}

void hamlib_radio::update__swr() {
    assert(rig != nullptr);

    auto result = rig->get_swr();
    if (result) {
        meters.swr = result.value;
    } else {
        log_error("could not get SWR from hamlib: ", result.error_str());
    }
}

void hamlib_radio::update__tuner() {
    assert(rig != nullptr);

    auto result = rig->get_freq();
    if (result) {
        vfo.tuner = result.value;
    } else {
        log_error("could not get frequency from hamlib: ", result.error_str());
    }
}

}
