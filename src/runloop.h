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

typedef uint64_t runloop_item_id_t;
typedef void (*runloop_cb_t)(void);

class runloopitem;

class runloop {
    friend class runloopitem;

private:
    uv_loop_t uv_loop;
    runloop_item_id_t prev_item_id = 0;

    runloop(const runloop&);
    std::list<runloopitem*> get_items(void);

public:
    runloop();
    ~runloop();

    void enter(void);
};

class runloopitem : public std::enable_shared_from_this<runloopitem> {
private:
    runloopitem(const runloopitem&);
    bool stopped = true;

protected:
    runloop* loop = NULL;

    runloopitem(runloop*);
    virtual void init(void);
    virtual uv_handle_t* uv_handle(void);
    uv_loop_t* get_uvloop(void);

public:
    const runloop_item_id_t id = 0;
    virtual void start(void);
    virtual void stop(void);
};

class runlooptimer : public runloopitem {
private:
    uv_timer_t uv_timer;
    runloop_cb_t cb;

    runlooptimer(const runlooptimer&);

protected:
    virtual void init(void);
    virtual uv_handle_t* uv_handle(void);
    virtual void execute(void);

public:
    uint64_t initial = 0;
    uint64_t repeat = 0;

    runlooptimer(runloop*, uint64_t);
    runlooptimer(runloop*, uint64_t, uint64_t);
    runlooptimer(runloop*, uint64_t, runloop_cb_t);
    runlooptimer(runloop*, uint64_t, uint64_t, runloop_cb_t);

    virtual void start(void);
    virtual void stop(void);
};

oemros::runloop* runloop_get(void);
void runloop_enter(void);

}

template<typename T, typename... Args>
std::shared_ptr<T> runloop_item(Args... args) {
    auto loop = oemros::runloop_get();
    std::shared_ptr<T> thing = std::make_shared<T>(loop, args...);
    return thing;
}

template<typename T, typename... Args>
std::shared_ptr<T> runloop_make(Args... args) {
    std::shared_ptr<T> item = runloop_item<T>(args...);
    item->start();
    return item;
}

#endif /* SRC_RUNLOOP_H_ */
