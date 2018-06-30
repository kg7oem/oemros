#include <iostream>
#include <memory.h>

#include "logging.h"

using namespace oemros;
using namespace std;

static void bootstrap(void) {
    logging_bootstrap();
}

static void setup_logging(void) {
    logging_add_destination(make_shared<logstdio>());
//    logging_add_destination(make_shared<logfile>("/home/tyler/test.log"));

    logging_set_level(loglevel::error);
}

int main(int argc, char **argv) {
    bootstrap();
    setup_logging();

    log_error("testing: ", errstream);

    return 0;
}
