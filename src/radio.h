/*
 * radio.h
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

#ifndef SRC_RADIO_H_
#define SRC_RADIO_H_

#include <future>
#include <iostream>

namespace hl {
#include <hamlib/rig.h>
}

#include "system.h"
#include "thread.h"

namespace oemros {

enum class vfo_t {
    CURR = RIG_VFO_CURR,
    A = RIG_VFO_A,
    B = RIG_VFO_B,
    C = RIG_VFO_C,
};

using freq_t = hl::freq_t;
using ptt_t = bool;
using data_mode_t = bool;

enum class modulation_t {
    unspecified = 0, unsupported, none,
    cw, cwr, rtty, rttyr,
    usb, lsb, am, fm, wfm,
};

REFLEAF(radiomode) {
    REFBOILER(radiomode);

    private:
        modulation_t modulation_mem = modulation_t::unspecified;
        data_mode_t data_mode_mem = false;

    public:
        radiomode(void) = default;
        radiomode(const modulation_t&, const data_mode_t&);
        modulation_t modulation(void);
        modulation_t modulation(modulation_t);
        data_mode_t data_mode(void);
        data_mode_t data_mode(data_mode_t);
};

class radio {
    private:
        radio(const radio&) = delete;
        radio(const radio&&) = delete;
        radio& operator=(const radio&) = delete;

    public:
        radio() = default;
        virtual ~radio() = default;
        virtual std::shared_ptr<oemros::promise<freq_t>> frequency(void) = 0;
        virtual std::shared_ptr<oemros::promise<freq_t>> frequency(vfo_t) = 0;
        virtual std::shared_ptr<oemros::promise<bool>> frequency(freq_t) = 0;
        virtual std::shared_ptr<oemros::promise<bool>> frequency(vfo_t, freq_t) = 0;
        virtual std::shared_ptr<oemros::promise<ptt_t>> ptt(void) = 0;
        virtual std::shared_ptr<oemros::promise<ptt_t>> ptt(vfo_t) = 0;
        virtual std::shared_ptr<oemros::promise<bool>> ptt(ptt_t) = 0;
        virtual std::shared_ptr<oemros::promise<bool>> ptt(vfo_t, ptt_t) = 0;
        virtual std::shared_ptr<oemros::promise<radiomode_s>> mode(void) = 0;
        virtual std::shared_ptr<oemros::promise<bool>> mode(radiomode_s) = 0;
        virtual std::shared_ptr<oemros::promise<bool>> mode(modulation_t, data_mode_t) = 0;
};

REFLEAF(hamlib, public radio) {
    REFBOILER(hamlib);

    private:
        hl::rig_model_t hl_model = 0;
        hl::rig* hl_rig = NULL;

    protected:
        std::shared_ptr<oemros::promise<freq_t>> hl_get_freq(vfo_t);
        std::shared_ptr<oemros::promise<bool>> hl_set_freq(vfo_t, freq_t);
        std::shared_ptr<oemros::promise<ptt_t>> hl_get_ptt(vfo_t);
        std::shared_ptr<oemros::promise<bool>> hl_set_ptt(vfo_t, ptt_t);
        std::shared_ptr<oemros::promise<radiomode_s>> hl_get_mode(vfo_t);
        std::shared_ptr<oemros::promise<bool>> hl_set_mode(vfo_t, radiomode_s);

    public:
        hamlib(hl::rig_model_t);
        virtual std::shared_ptr<oemros::promise<freq_t>> frequency(void) override;
        virtual std::shared_ptr<oemros::promise<freq_t>> frequency(vfo_t) override;
        virtual std::shared_ptr<oemros::promise<bool>> frequency(freq_t) override;
        virtual std::shared_ptr<oemros::promise<bool>> frequency(vfo_t, freq_t) override;
        virtual std::shared_ptr<oemros::promise<ptt_t>> ptt(void) override;
        virtual std::shared_ptr<oemros::promise<ptt_t>> ptt(vfo_t) override;
        virtual std::shared_ptr<oemros::promise<bool>> ptt(ptt_t) override;
        virtual std::shared_ptr<oemros::promise<bool>> ptt(vfo_t, ptt_t) override;
        virtual std::shared_ptr<oemros::promise<radiomode_s>> mode(void) override;
        virtual std::shared_ptr<oemros::promise<bool>> mode(modulation_t, data_mode_t) override;
        virtual std::shared_ptr<oemros::promise<bool>> mode(radiomode_s) override;
};

void radio_bootstrap(void);

}

#endif /* SRC_RADIO_H_ */
