/*
 * module.h
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

#ifndef SRC_MODULE_H_
#define SRC_MODULE_H_

#include "runloop.h"
#include "system.h"
#include "thread.h"

namespace oemros {

OBJECT(module_components) {
    OBJSTUFF(module_components);

    protected:

    public:
        runloop_s runloop = runloop::make();
        module_components() = default;
};

ABSTRACT(module) {
    ABSSTUFF(module);

    friend module_s module_create(const char*);

    protected:
        // only the module system is allowed to construct
        // and initialize module objects
        module() = default;
        virtual void will_start() = 0;
        virtual void did_start() = 0;

    public:
//        EVENT_SOURCE(something, bool, int);
        module_components_s oemros = module_components::make();
        void start();
};

ABSTRACT(module_info) {
    ABSSTUFF(module_info);

    private:
        module_info& operator=(const module_info&) = delete;
        module_info(const module_info&) = delete;
        module_info(const module_info&&) = delete;

    protected:
        virtual void do_bootstrap() const = 0;
        virtual void do_cleanup() const = 0;
        virtual module_s do_create_module() const = 0;

    public:
        const string name;

        module_info(string in_name) : name(in_name) { }

        void bootstrap() const;
        void cleanup() const;
        module_s make_module() const;
};

OBJECT(task) {
    OBJSTUFF(task);

    private:
        const module_s module;
        // FIXME this needs to be handled in the destructor
        thread* mod_thread = NULL;

    public:
        const int task_num;
        const string title;
        task(const string title_in, module_s module_in, thread* mod_thread_in);
};

void module_bootstrap();
task_s task_spawn(const string& title, const char* module_name);

using modinfo_func = const module_info* (*)();

// per module functions to get module data if the module
// is linked in
extern "C" {
    const module_info* module__test_load();
}

}

#endif /* SRC_MODULE_H_ */
