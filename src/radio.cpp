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

using namespace hl;

namespace oemros {

void radio_bootstrap(void) {
//    rig_set_debug_callback([](enum rig_debug_level_e debug_level, UNUSED rig_ptr_t user_data, UNUSED const char *fmt, UNUSED va_list arg) -> int {
//        return true;
//    }, NULL);
}

radiomode::radiomode(const modulation_t& modulation_arg, const data_mode_t& data_mode_arg)
: modulation_mem(modulation_arg), data_mode_mem(data_mode_arg) {

}

modulation_t radiomode::modulation(void) {
    return this->modulation_mem;
}

modulation_t radiomode::modulation(modulation_t new_modulation) {
    modulation_t old_modulation = this->modulation_mem;
    this->modulation_mem = new_modulation;
    return old_modulation;
}

data_mode_t radiomode::data(void) {
    return this->data_mode_mem;
}

data_mode_t radiomode::data(data_mode_t new_data_mode) {
    data_mode_t old_data_mode = this->data_mode_mem;
    this->data_mode_mem = new_data_mode;
    return old_data_mode;
}

hamlib::hamlib(rig_model_t model_arg)
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

std::shared_ptr<oemros::promise<freq_t>> hamlib::hl_get_freq(vfo_t vfo) {
    log_trace("Going to read the frequency from hamlib");
    auto promise = make_promise<freq_t>([this, vfo] {
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

std::shared_ptr<oemros::promise<bool>> hamlib::hl_set_freq(vfo_t vfo, freq_t freq) {
    log_trace("going to set the frequency with hamlib");

    auto promise = make_promise<bool>([this, vfo, freq] {
        int result = rig_set_freq(this->hl_rig, (int)vfo, freq);
        if (result != RIG_OK) {
            log_fatal("rig_set_freq() failed");
        }
        log_trace("back from hamlib");
        return true;
    });

    return promise;
}

std::shared_ptr<oemros::promise<bool>> hamlib::hl_get_ptt(vfo_t vfo) {
    log_trace("going to get the PTT state from hamlib");

    auto promise = make_promise<bool>([this, vfo] {
        hl::ptt_t ptt;
        int result = rig_get_ptt(this->hl_rig, (int)vfo, &ptt);
        if (result != RIG_OK) {
            log_fatal("rig_get_ptt() failed");
        }
        log_trace("back from rig_get_ptt()");
        return ptt == RIG_PTT_ON;
    });

    return promise;
}

std::shared_ptr<oemros::promise<bool>> hamlib::hl_set_ptt(vfo_t vfo, bool ptt_active) {
    log_trace("going to set the PTT state with hamlib");
    hl::ptt_t hl_ptt;

    if (ptt_active) {
        hl_ptt = RIG_PTT_ON;
    } else {
        hl_ptt = RIG_PTT_OFF;
    }

    auto promise = make_promise<bool>([this, vfo, hl_ptt] {
        int result = rig_set_ptt(this->hl_rig, (int)vfo, hl_ptt);
        if (result != RIG_OK) {
            log_fatal("rig_set_ptt() failed");
        }
        return true;
    });

    return promise;
}

std::shared_ptr<oemros::promise<radiomode_s>> hamlib::hl_get_mode(vfo_t vfo) {
    log_trace("going to get the radio mode with hamlib");

    auto promise = make_promise<radiomode_s>([this, vfo]{
        hl::rmode_t rmode;
        hl::pbwidth_t width;
        int result;

        result = rig_get_mode(this->hl_rig, (int)vfo, &rmode, &width);
        if (result != RIG_OK) {
            log_fatal("rig_get_mode() failed");
        }

        log_trace("back from hamlib");

        modulation_t modulation;
        data_mode_t datamode;

        switch (rmode) {
	default:
	    modulation = modulation_t::unsupported;
            datamode = false;
	    break;
        case RIG_MODE_NONE:
            modulation = modulation_t::none;
            datamode = false;
            break;
        case RIG_MODE_AM:
            modulation = modulation_t::am;
            datamode = false;
            break;
        case RIG_MODE_CW:
            modulation = modulation_t::cw;
            datamode = false;
            break;
        case RIG_MODE_USB:
            modulation = modulation_t::usb;
            datamode = false;
            break;
        case RIG_MODE_LSB:
            modulation = modulation_t::lsb;
            datamode = false;
            break;
        case RIG_MODE_RTTY:
            modulation = modulation_t::rtty;
            break;
        case RIG_MODE_FM:
            modulation = modulation_t::fm;
            break;
        case RIG_MODE_WFM:
            modulation = modulation_t::wfm;
            datamode = false;
            break;
        case RIG_MODE_CWR:
            modulation = modulation_t::cwr;
            datamode = false;
            break;
        case RIG_MODE_RTTYR:
            modulation = modulation_t::rttyr;
            datamode = false;
            break;
        case RIG_MODE_PKTLSB:
            modulation = modulation_t::lsb;
            break;
        case RIG_MODE_PKTUSB:
            modulation = modulation_t::usb;
            break;
        case RIG_MODE_PKTFM:
            modulation = modulation_t::fm;
            break;
        }
        return radiomode::create(modulation, datamode);
    });

    return promise;
}

std::shared_ptr<oemros::promise<bool>> hamlib::hl_set_mode(vfo_t vfo, radiomode_s mode) {
    log_trace("going to set the radio mode with hamlib");

    auto promise = make_promise<bool>([this, vfo, mode]{
        hl::rmode_t rmode;

        switch (mode->modulation()) {
        case modulation_t::unspecified:
            log_fatal("can't yet use an unspecified modulation when setting a mode");
            break;
        case modulation_t::unsupported:
            log_fatal("attempt to set modulation type to 'unsupported'");
            break;
        case modulation_t::none:
            if (mode->data()) {
                log_fatal("can not set datamode with a modulation of 'none' with hamlib");
            }
            rmode = RIG_MODE_NONE;
            break;
        case modulation_t::cw:
            if (mode->data()) {
                log_fatal("can not set datamode with CW with hamlib");
            }
            rmode = RIG_MODE_CW;
            break;
        case modulation_t::cwr:
            if (mode->data()) {
                log_fatal("can not set datamode with CWR with hamlib");
            }
            rmode = RIG_MODE_CWR;
            break;
        case modulation_t::rtty:
            if (mode->data()) {
                log_fatal("can not set datamode with RTTY with hamlib");
            }
            rmode = RIG_MODE_RTTY;
            break;
        case modulation_t::rttyr:
            if (mode->data()) {
                log_fatal("can not set datamode with RTTYR with hamlib");
            }
            rmode = RIG_MODE_RTTYR;
            break;
        case modulation_t::usb:
            if (mode->data()) {
                rmode = RIG_MODE_PKTUSB;
            } else {
                rmode = RIG_MODE_USB;
            }
            break;
        case modulation_t::lsb:
            if (mode->data()) {
                rmode = RIG_MODE_PKTLSB;
            } else {
                rmode = RIG_MODE_LSB;
            }
            break;
        case modulation_t::am:
            if (mode->data()) {
                log_fatal("can not set datamode with AM with hamlib");
            }
            rmode = RIG_MODE_AM;
            break;
        case modulation_t::fm:
            if (mode->data()) {
                rmode = RIG_MODE_PKTFM;
            } else {
                rmode = RIG_MODE_FM;
            }
            break;
        case modulation_t::wfm:
            if (mode->data()) {
                log_fatal("can not set datamode with WFM with hamlib");
            }
            rmode = RIG_MODE_WFM;
            break;
        }

        hl::pbwidth_t width;
        hl::rmode_t old_rmode;
        int result;

        result = rig_get_mode(this->hl_rig, (int)vfo, &old_rmode, &width);
        if (result != RIG_OK) {
            log_fatal("rig_get_mode() failed: ", rigerror(result));
        }

        result = rig_set_mode(this->hl_rig, (int)vfo, rmode, width);
        if (result != RIG_OK) {
            log_fatal("rig_set_mode() failed");
        }

        return true;
    });

    return promise;
}

std::shared_ptr<oemros::promise<freq_t>> hamlib::frequency(vfo_t vfo) {
    return this->hl_get_freq(vfo);
}

std::shared_ptr<oemros::promise<freq_t>> hamlib::frequency(void) {
    return this->hl_get_freq(vfo_t::CURR);
}

std::shared_ptr<oemros::promise<bool>> hamlib::frequency(freq_t freq) {
    return this->hl_set_freq(vfo_t::CURR, freq);
}

std::shared_ptr<oemros::promise<bool>> hamlib::frequency(vfo_t vfo, freq_t freq) {
    return this->hl_set_freq(vfo, freq);
}

std::shared_ptr<oemros::promise<bool>> hamlib::ptt(void) {
    return this->hl_get_ptt(vfo_t::CURR);
}

std::shared_ptr<oemros::promise<bool>> hamlib::ptt(vfo_t vfo) {
    return this->hl_get_ptt(vfo);
}

std::shared_ptr<oemros::promise<bool>> hamlib::ptt(bool ptt_active) {
    return this->hl_set_ptt(vfo_t::CURR, ptt_active);
}

std::shared_ptr<oemros::promise<bool>> hamlib::ptt(vfo_t vfo, bool ptt_active) {
    return this->hl_set_ptt(vfo, ptt_active);
}

std::shared_ptr<oemros::promise<radiomode_s>> hamlib::mode(void) {
    return this->hl_get_mode(vfo_t::CURR);
}

std::shared_ptr<oemros::promise<bool>> hamlib::mode(radiomode_s mode) {
    return this->hl_set_mode(vfo_t::CURR, mode);
}

std::shared_ptr<oemros::promise<bool>> hamlib::mode(modulation_t mod_arg, data_mode_t data_arg) {
    return this->hl_set_mode(vfo_t::CURR, radiomode::create(mod_arg, data_arg));
}
}
