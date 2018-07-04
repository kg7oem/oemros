#include <iostream>
#include <memory.h>

#include "logging.h"
#include "runloop.h"

using namespace oemros;
using namespace std;

int main(int argc, char **argv) {

    logging_add_destination(make_shared<logstdio>());
    logging_set_level(loglevel::trace);

    auto loop = runloop::create();
    auto item = loop->create_item<rlitem>();

    log_debug("going to exit main");

    return 0;
}
