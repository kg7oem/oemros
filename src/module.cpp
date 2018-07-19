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
static list<module_w> active_modules;

static int last_task_num = 0;

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

static module_s module_create(const string& module_name) {
    log_trace("request to create an instance of a module with name: ", module_name);

    auto exclusive = get_lock();
    auto found = loaded_modules.find(module_name);
    if (found == loaded_modules.end()) {
        log_fatal("could not find a module with name of ", module_name);
    }
    auto info = found->second;
    exclusive.unlock();

    log_trace("calling the registered create function for module ", module_name);
    module_s new_module = info->make_module();
    log_trace("got control back from create function for module", module_name);

    return new_module;
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

module::states module::get_state(void) {
    return state;
}

module::states module::set_state(module::states new_state) {
    switch (new_state) {
    case module::states::created:
        log_fatal("attempt to set module state to 'created'");
        break;
    case module::states::starting:
        if (state != module::states::created) {
            log_fatal("attempt to move to starting state when current state is not 'created'");
        }
        break;
    case module::states::started:
        if (state != module::states::starting) {
            log_fatal("attempt to move to started state when current state is not 'starting'");
        }
        break;
    case module::states::stopping:
        if (state != module::states::starting || state != module::states::started) {
            log_fatal("attempt to move to stopping state when current state is not starting or started");
        }
        break;
    case module::states::stopped:
        if (state != module::states::stopping) {
            log_fatal("attempt to move to stopped state when current state is not stopping");
        }
    }

    module::states old_state = state;
    state = new_state;
    events.state_changed.deliver(this, state);

    return old_state;
}

void module::start() {
    set_state(module::states::starting);

    log_trace("notifying subclass that module is going to start");
    will_start();

    // FIXME check to see if the module decided to stop before continuing
    log_trace("scheduling a runloop job to deliver the did_start() notification");
    oemros->runloop->make_item<rljob>([this] {
        log_trace("got control inside start notifier runloop job");

        set_state(module::states::started);
        log_trace("invoking the did_start() method");
        did_start();
        log_trace("done invoking the did_start method");
    })->start();
}

task::task(const string title_in, module_s module_in, thread* mod_thread_in)
: module(module_in), mod_thread(mod_thread_in), task_num(++last_task_num), title(title_in) {
    log_trace("constructed task #", task_num, ": ", title);
}

// create a new task and create a new instance of a module
// inside that task but return after the module has already
// begun the start process
task_s task_spawn(const string& title, const char* module_name) {
    log_trace("Spawning task: ", title);
    auto module_promise = promise<module_s>::make();

    auto task_thread = new thread([&module_name, module_promise] {
        log_trace("new task thread has just started");

        auto fresh_module = module_create(module_name);
        fresh_module->start();
        module_promise->set(fresh_module);
        log_trace("starting the module runloop");
        fresh_module->oemros->runloop->enter();
    });

    log_trace("waiting for the module to be constructed in the task thread");
    auto new_module = module_promise->get();

    log_trace("module is now ready");
    return task::make(title, new_module, task_thread);
}

}
