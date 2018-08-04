/*
 * hamlib.h
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

#pragma once

#include "object.h"

namespace hamlib {

#include <hamlib/rig.h>

}

namespace oemros {

class hamlib_rig : public baseobj {
    private:
        hamlib::RIG* hl_rig = nullptr;

    public:
        const hamlib::rig_model_t model;
        hamlib_rig(const hamlib::rig_model_t& model_in) : model(model_in) { }
        ~hamlib_rig();
        bool open();
};

}