#include <iostream>
#include <memory.h>

#include "logging.h"
#include "runloop.h"
#include "system.h"

using namespace oemros;
using namespace std;

int main(int argc, char **argv) {
    int someint = 10;

    logging_add_destination(make_shared<logstdio>());
    logging_set_level(loglevel::trace);

    log_info("At the start someint is ", someint);

    auto loop = runloop::create();
    auto thing = loop->create_item<rlonce>([&someint] {
        log_trace("I should run once");
        log_info("I set the value to: ", ++someint);
    });

    thing->start();

    loop->enter();

    log_debug("going to exit main; someint = ", someint);
    return 0;
}
