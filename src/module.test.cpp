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
#include "module.h"
#include "module.test.h"

OBJECT(test_module_info, public oemros::module_info) {
    OBJSTUFF(test_module_info);

    public:
        test_module_info(const std::string& name)
        : oemros::module_info(name) { }
        virtual void do_bootstrap() override { }
        virtual void do_cleanup() override { }
        virtual oemros::module_s do_create_module();
};

OBJECT(test_module, public oemros::module) {
    OBJSTUFF(test_module);

    protected:
        virtual void will_start(void) override;
        virtual void did_start(void) override;

    public:
        test_module();
};

extern "C" oemros::module_info_s module__test_load(void) {
    log_trace("returning the info for the test module");
    return test_module_info::create("test_module");
}

//struct test_module_info : public oemros::module_info {
//    test_module_info(const std::string& name)
//	: oemros::module_info(name) { }
//
//    void bootstrap(void) const override {
//	log_trace("bootstrapping the test module");
//    }
//
//    oemros::module_s create(void) const override {
//	log_trace("creating an instance of a test module");
//	return std::dynamic_pointer_cast<oemros::module>(test::create());
//    }
//
//};

oemros::module_s test_module_info::do_create_module() {
    return std::dynamic_pointer_cast<oemros::module>(test_module::create());
}

test_module::test_module(void) { }

void test_module::will_start(void) {
    log_trace("will_start was called");
}

void test_module::did_start(void) {
    log_trace("did_start was called");
    oemros->runloop->create_item<oemros::rlonce>([]{
        log_info("This is the test module");
    })->start();
}
