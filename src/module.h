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

    public:
        runloop_s runloop = runloop::create();
        module_components(void);
};

ABSTRACT(module) {
    friend module_s module_create(const char*);

    protected:
        // only the module system is allowed to construct
        // and initialize module objects
        module();
        virtual void will_start(void) = 0;
        virtual void did_start(void) = 0;

    public:
        module_components_s oemros = module_components::create();
        void start(void);
        virtual ~module() = default;
};

struct module_info {
    const std::string name;

    module_info(std::string in_name) 
	: name(in_name)
    {
    }
    virtual void bootstrap() const = 0;
    virtual module_s create() const = 0;

    private:
	module_info& operator=(const module_info&) = delete;
	module_info(const module_info&) = delete;
	module_info(const module_info&&) = delete;
};

void module_bootstrap(void);
module_s module_create(const std::string&);
std::thread* module_spawn(const std::string&);

// per module functions to get module data if the module
// is linked in
extern "C" {
    const oemros::module_info* module__test_load(void);
}

}

