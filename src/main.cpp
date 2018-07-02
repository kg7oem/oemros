#include <iostream>
#include <memory.h>

#include "logging.h"
#include "runloop.h"

using namespace oemros;
using namespace std;

class sometimer : public runlooptimer {
public:
    sometimer(oemros::runloop* loop, uint64_t initial, uint64_t repeat)
        : runlooptimer(loop, initial, repeat) { };

    virtual void execute(void) {
        log_info("that wasn't so bad");
    }
};

int main(int argc, char **argv) {
    logging_add_destination(make_shared<logstdio>());
    logging_set_level(loglevel::trace);

    runloop_cb_t cb = [](){ log_debug("welp here I am!"); };
    __attribute__((unused)) auto timer = runloop_make<runlooptimer>(500, cb);

    __attribute__((unused)) auto timer2 = runloop_make<sometimer>(10, 0);

    runloop_enter();

    log_error("the end");

    return 0;
}
