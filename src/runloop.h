/*
 * runloop.h
 *
 *  Created on: Jul 3, 2018
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

#ifndef SRC_RUNLOOP_H_
#define SRC_RUNLOOP_H_

#include <uv.h>

#include "system.h"

namespace oemros {

class rlitem : public refcounted<rlitem> {
    public:
        rlitem();
        ~rlitem();
};

class runloop : public refcounted<runloop> {
    private:
        uv_loop_t uv_loop;

    public:
        runloop();
        ~runloop();
};

}

#endif /* SRC_RUNLOOP_H_ */
