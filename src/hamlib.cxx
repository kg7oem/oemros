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

#include "hamlib.h"

namespace oemros {

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

}
