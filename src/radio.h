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

namespace hamlib {
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

using freq_t = hamlib::freq_t;

REFLEAF(radio) {
    private:
        hamlib::rig_model_t hl_model = 0;
        hamlib::rig* hl_rig = NULL;

    protected:
        std::shared_ptr<oemros::promise<freq_t>> hl_get_freq(vfo_t);
        std::shared_ptr<oemros::promise<bool>> hl_set_freq(vfo_t, freq_t);
        std::shared_ptr<oemros::promise<bool>> hl_get_ptt(vfo_t);
        std::shared_ptr<oemros::promise<bool>> hl_set_ptt(vfo_t, bool);

    public:
        radio(hamlib::rig_model_t);
        template<typename... Args>
        static radio_s create(Args&&...args) {
            return std::make_shared<radio>(args...);
        }
        std::shared_ptr<oemros::promise<freq_t>> frequency(void);
        std::shared_ptr<oemros::promise<freq_t>> frequency(vfo_t);
        std::shared_ptr<oemros::promise<bool>> frequency(freq_t);
        std::shared_ptr<oemros::promise<bool>> frequency(vfo_t, freq_t);
        std::shared_ptr<oemros::promise<bool>> ptt(void);
        std::shared_ptr<oemros::promise<bool>> ptt(vfo_t);
        std::shared_ptr<oemros::promise<bool>> ptt(bool);
        std::shared_ptr<oemros::promise<bool>> ptt(vfo_t, bool);
};

void radio_bootstrap(void);

}

#endif /* SRC_RADIO_H_ */
