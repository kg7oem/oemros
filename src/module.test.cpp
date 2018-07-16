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
#include "radio.h"

OBJECT(test_module_info, public oemros::module_info) {
    OBJSTUFF(test_module_info);

    public:
        test_module_info(const std::string& in_name)
        : oemros::module_info(in_name) { }
        virtual void do_bootstrap() const override { }
        virtual void do_cleanup() const override { }
        virtual oemros::module_s do_create_module() const override;
};

OBJECT(test_module, public oemros::module) {
    OBJSTUFF(test_module);

    protected:
        virtual void will_start() override;
        virtual void did_start() override;

    public:
        test_module() = default;
};

extern "C" const oemros::module_info* module__test_load() {
    static const test_module_info info("test_module");
    log_trace("returning the info for the test module");
    return dynamic_cast<const oemros::module_info*>(&info);
}

oemros::module_s test_module_info::do_create_module() const {
    return std::dynamic_pointer_cast<oemros::module>(test_module::create());
}

void test_module::will_start() {
    log_trace("will_start was called");
}

void test_module::did_start() {
    log_trace("did_start was called");
    oemros->runloop->create_item<oemros::rlonce>([]{
        log_info("This is the test module");
        auto rig = oemros::hamlib::create(1);
        log_info("Freq: ", rig->frequency()->get());
    })->start();
}
