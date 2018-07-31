/*
 * object.cxx
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

#include <atomic>

#include "object.h"

namespace oemros {

uint32_t get_next_subscription_num() {
    static std::atomic<uint32_t> last_subscription_num = ATOMIC_VAR_INIT(0);
    return ++last_subscription_num;
}

}
