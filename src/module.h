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

#include "runloop.h"
#include "system.h"

namespace oemros {

OBJECT(module_components) {
    OBJSTUFF(module_components);

    protected:
        oemros::runloop_s runloop;

    public:
        module_components(void);
};

ABSTRACT(module) {
    friend module_s module_create(const char*);

    protected:
        // only the module system is allowed to construct
        // and initialize module objects
        module();
        virtual bool init(void) = 0;
        oemros::module_components oemros;

    public:
        virtual ~module() = default;
};

using module_bootstrap_t = void(*)(void);
using module_factory_t = module_s (*)(void);

struct module_info_st {
    const char* name = NULL;
    module_bootstrap_t bootstrap = NULL;
    module_factory_t create = NULL;
};

using module_info_t = struct module_info_st;
using module_loader_t = const module_info_t* (*)(void);

void module_bootstrap(void);
module_s module_create(const char*);

// per module functions to get module data if the module
// is linked in
extern "C" {
    const oemros::module_info_t* module__test_load(void);
}

}

