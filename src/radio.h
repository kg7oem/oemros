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
};

REFLEAF(hamlib, public radio) {
    private:
        hl::rig_model_t hl_model = 0;
        hl::rig* hl_rig = NULL;

    protected:
        std::shared_ptr<oemros::promise<freq_t>> hl_get_freq(vfo_t);
        std::shared_ptr<oemros::promise<bool>> hl_set_freq(vfo_t, freq_t);
        std::shared_ptr<oemros::promise<ptt_t>> hl_get_ptt(vfo_t);
        std::shared_ptr<oemros::promise<bool>> hl_set_ptt(vfo_t, ptt_t);

    public:
        hamlib(hl::rig_model_t);
        template<typename... Args>
        static hamlib_s create(Args&&...args) {
            return std::make_shared<hamlib>(args...);
        }
        virtual std::shared_ptr<oemros::promise<freq_t>> frequency(void) override;
        virtual std::shared_ptr<oemros::promise<freq_t>> frequency(vfo_t) override;
        virtual std::shared_ptr<oemros::promise<bool>> frequency(freq_t) override;
        virtual std::shared_ptr<oemros::promise<bool>> frequency(vfo_t, freq_t) override;
        virtual std::shared_ptr<oemros::promise<ptt_t>> ptt(void) override;
        virtual std::shared_ptr<oemros::promise<ptt_t>> ptt(vfo_t) override;
        virtual std::shared_ptr<oemros::promise<bool>> ptt(ptt_t) override;
        virtual std::shared_ptr<oemros::promise<bool>> ptt(vfo_t, ptt_t) override;
};

void radio_bootstrap(void);

}

#endif /* SRC_RADIO_H_ */
