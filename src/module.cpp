/*
 * module.cpp
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

#include "logging.h"
#include "module.h"
#include "thread.h"

namespace oemros {

static mutex module_mutex;
static map<string, const module_info*> loaded_modules;

static lock get_lock() {
    log_trace("trying to acquire the module info mutex");
    auto new_lock = make_lock(module_mutex);
    log_trace("got the module info mutex");
    return new_lock;
}

static void load_module(modinfo_func info_function) {
    auto info = info_function();

    if (info->name.empty()) {
        log_fatal("name of module passsed for initializtion was NULL");
    }

    log_trace("request to initialize module: ", info->name);

    auto exclusive = get_lock();

    if (loaded_modules.find(info->name) != loaded_modules.end()) {
        log_fatal("attempt to load module with duplicate name: ", info->name);
    }

    log_trace("running bootstrap for module ", info->name);
    info->bootstrap();
    log_trace("done running bootstrap for module ", info->name);

    log_trace("adding module info to the loaded module list");
    loaded_modules[info->name] = info;
}

void module_bootstrap() {
    load_module(module__test_load);
}

module_s module_create(const string& module_name) {
    log_trace("request to create an instance of a module with name: ", module_name);

    auto found = loaded_modules.find(module_name);
    if (found == loaded_modules.end()) {
        log_fatal("could not find a module with name of ", module_name);
    }
    auto info = found->second;
    log_trace("calling the registered create function for module ", module_name);
    module_s new_module = info->make_module();
    log_trace("got control back from create function for module", module_name);

    return new_module;
}

thread* module_spawn(const string& name) {
    auto module_thread = new thread([name]{
        log_trace("new module thread has just started");
        auto module_instance = module_create(name);
        module_instance->start();
        module_instance->oemros->runloop->enter();
    });

    return module_thread;
}

void module_info::bootstrap() const {
    do_bootstrap();
}

void module_info::cleanup() const {
    do_cleanup();
}

module_s module_info::make_module() const {
    return do_create_module();
}

void module::start() {
    log_trace("notifying subclass that module is going to start");
    will_start();

    // FIXME check to see if the module decided to stop before continuing
    log_trace("scheduling a runloop job to deliver the did_start() notification");
    oemros->runloop->make_item<rljob>([this] {
        log_trace("got control inside start notifier runloop job");

        log_trace("invoking the did_start() method");
        did_start();
        log_trace("done invoking the did_start method");
    })->start();
}

}
