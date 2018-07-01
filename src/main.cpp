#include <iostream>
#include <memory.h>

#include "logging.h"
#include "runloop.h"

using namespace oemros;
using namespace std;

int main(int argc, char **argv) {
    logging_add_destination(make_shared<logstdio>());
    logging_set_level(loglevel::trace);

    runloop loop;

    log_error("the end");

    return 0;
}
