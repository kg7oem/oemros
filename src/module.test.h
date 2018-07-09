/*
 * module.test.h
 *
 *  Created on: Jul 8, 2018
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

#ifndef SRC_MODULE_TEST_H_
#define SRC_MODULE_TEST_H_

#include "module.h"

OBJECT(test, public oemros::module) {
    OBJSTUFF(test);

    protected:
        virtual void will_start(void) override;
        virtual void did_start(void) override;

    public:
        test();
};

#endif /* SRC_MODULE_TEST_H_ */