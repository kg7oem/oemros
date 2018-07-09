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

#include <map>
#include <string>
#include <thread>

#include "logging.h"
#include "module.h"

namespace oemros {

static std::mutex module_mutex;
static std::map<std::string, const module_info_t*> loaded_modules;

static std::unique_lock<std::mutex> get_lock(void) {
    log_trace("trying to acquire the module info mutex");
    return std::unique_lock<std::mutex>(module_mutex);
    log_trace("got the module info mutex");
}

static void load_module(const module_info_t* info) {
    if (info->name == NULL) {
        log_fatal("name of module passsed for initializtion was NULL");
    }

    log_trace("request to initialize module: ", info->name);

    auto lock = get_lock();

    if (loaded_modules.find(info->name) != loaded_modules.end()) {
        log_fatal("attempt to load module with duplicate name: ", info->name);
    }

    if (info->bootstrap != NULL) {
        log_trace("running bootstrap for module ", info->name);
        info->bootstrap();
        log_trace("done running bootstrap for module ", info->name);
    } else {
        log_trace("no bootstrap specified for module ", info->name);
    }

    log_trace("adding module info to the loaded module list");
    loaded_modules[info->name] = info;
}

//static void unload_module(const char* name) {
//
//}

void module_bootstrap(void) {
    load_module(module__test_load());
}

module_s module_create(const char* module_name) {
    log_trace("request to create an instance of a module with name: ", module_name);

    auto found = loaded_modules.find(module_name);
    if (found == loaded_modules.end()) {
        log_fatal("could not find a module with name of ", module_name);
    }
    const module_info_t* info = found->second;
    log_trace("calling the registered create function for module ", module_name);
    module_s new_module = info->create();
    log_trace("got control back from create function for module", module_name);

    return new_module;
}

std::thread* module_spawn(const char* name) {
    auto module_thread = new std::thread([name]{
        log_trace("new module thread has just started");
        auto module_instance = module_create(name);
        module_instance->start();
        module_instance->oemros->runloop->enter();
    });

    return module_thread;
}

module_components::module_components(void) { }

module::module(void) { }

void module::start(void) {
    log_trace("notifying subclass that module is going to start");
    this->will_start();

    // FIXME check to see if the module decided to stop before continuing
    log_trace("scheduling a runloop job to deliver the did_start() notification");
    this->oemros->runloop->create_item<rlonce>([this] {
        log_trace("got control inside start notifier runloop job");

        log_trace("invoking the did_start() method");
        this->did_start();
        log_trace("done invoking the did_start method");
    })->start();
}

}
