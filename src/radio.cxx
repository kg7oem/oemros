/*
 * radio.cxx
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

#include "radio.h"

namespace oemros {

void radio::update() {
    if (~ update_mask & (mask_type)radio::update::alc) update__alc();
    if (~ update_mask & (mask_type)radio::update::power) update__power();
    if (~ update_mask & (mask_type)radio::update::swr) update__swr();
    if (~ update_mask & (mask_type)radio::update::tuner) update__tuner();
}

}
