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

struct test_module_info : public oemros::module_info {
    test_module_info(const std::string& name)
	: oemros::module_info(name) { }

    void bootstrap(void) const override {
	log_trace("bootstrapping the test module");
    }

    oemros::module_s create(void) const override {
	log_trace("creating an instance of a test module");
	return std::dynamic_pointer_cast<oemros::module>(test::create());
    }

};

static const test_module_info test_module{"test"};

const oemros::module_info* oemros::module__test_load(void) {
    log_trace("returning the info for the test module");

    return &test_module;
}

test::test(void) { }

void test::will_start(void) {
    log_trace("will_start was called");
}

void test::did_start(void) {
    log_trace("did_start was called");
    this->oemros->runloop->create_item<oemros::rlonce>([]{
        log_info("This is the test module");
    })->start();
}
