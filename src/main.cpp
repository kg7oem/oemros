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

    log_trace("tracing something");
    log_lots("lots of logging");
    log_debug("debugging it");
    log_verbose("verbosity = high");
    log_info("info: you have been told");
    log_notice("notice notice notice");
    log_warn("uhm that looks possibly not good");
    log_error("yeah that was actually ", "bad");

    log_fatal("this should ", "totally abort");

    return 0;
}
