/*
 * runloop.h
 *
 *  Created on: Jul 1, 2018
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

#ifndef SRC_RUNLOOP_H_
#define SRC_RUNLOOP_H_

#include <memory>
#include <uv.h>

namespace oemros {

typedef void (*runloop_cb_t)(void);

class runloop {
    friend class runloopitem;

private:
    uv_loop_t uv_loop;

    runloop(const runloop&);

public:
    runloop();
    ~runloop();

    void enter(void);
};

class runloopitem {
private:
    runloopitem(const runloopitem&);

protected:
    runloop* loop = NULL;

    runloopitem(runloop*);
    uv_loop_t* get_uvloop(void);
};

class runlooptimer : protected runloopitem {
private:
    uv_timer_t uv_timer;
    runloop_cb_t cb;

    runlooptimer(const runlooptimer&);
    void init(void);

protected:
    virtual void execute(void);

public:
    uint64_t initial = 0;
    uint64_t repeat = 0;

    runlooptimer(runloop*, uint64_t);
    runlooptimer(runloop*, uint64_t, uint64_t);
    runlooptimer(runloop*, uint64_t, runloop_cb_t);
    runlooptimer(runloop*, uint64_t, uint64_t, runloop_cb_t);

    void start(void);
    void stop(void);
};

oemros::runloop* runloop_get(void);
void runloop_enter(void);

}

template<typename T>
std::shared_ptr<T> runloop_make(void) {
    std::shared_ptr<T> thing = std::make_shared<T>(oemros::runloop_get());

    return thing;
}

template<typename T, typename... Args>
std::shared_ptr<T> runloop_make(Args... args) {
    std::shared_ptr<T> thing = std::make_shared<T>(oemros::runloop_get(), args...);

    return thing;
}

#endif /* SRC_RUNLOOP_H_ */
