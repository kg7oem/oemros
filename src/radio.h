/*
 * radio.h
 *
 *  Created on: Jul 30, 2018
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

#pragma once

#include <cstdint>

#include "object.h"
#include "runloop.h"

namespace oemros {

// in Hz
using frequency = uint64_t;

class radio : public baseobj {
    public:
        using mask_type = uint64_t;
        enum class update : mask_type {
            alc = 1 << 0,
            power = 1 << 1,
            swr = 1 << 2,
            tuner = 1 << 3,
        };

    protected:
        std::shared_ptr<runloop> loop;
        virtual void update__alc() = 0;
        virtual void update__power() = 0;
        virtual void update__swr() = 0;
        virtual void update__tuner() = 0;

    public:
        struct vfo_type : public baseobj {
            value_source<frequency> tuner{0};
            // TODO splits, repeater offset, tone
        };

        struct meters_type : public baseobj {
            value_source<float> power{0};
            value_source<float> swr{0};
            value_source<float> alc{0};
        };

        mask_type update_mask = 0;
        vfo_type vfo;
        meters_type meters;

        radio(std::shared_ptr<runloop> loop_in) : loop(loop_in) { }
        void update();
};

}
