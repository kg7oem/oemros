/*
 * module.test.cpp
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

#include <memory>

#include "logging.h"
#include "module.test.h"

static void bootstrap_test(void) {
    log_trace("bootstrapping the test module");
}

static oemros::module_s create_test(void) {
    log_info("creating an instance of a test module");
    return std::dynamic_pointer_cast<oemros::module>(test::create());
}

const oemros::module_info_t* oemros::module__test_load(void) {
    log_trace("returning the info for the test module");

    static const oemros::module_info_t info = {
            name: "test",
            bootstrap: bootstrap_test,
            create: create_test,
    };

    return &info;
}

test::test(void) { }

bool test::init(void) {
    log_fatal("well this is where we are");
}
