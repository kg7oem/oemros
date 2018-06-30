#include <iostream>

#include "logging.h"

using namespace oemros;
using namespace std;

static void bootstrap(void) {
    logging_bootstrap();
}

int main(int argc, char **argv) {

    bootstrap();

    logging_set_level(loglevel::trace);

    log_debug("test ", 1);
    log_debug("foo");

    log_fatal("this should ", "totally abort");

    return 0;
}
