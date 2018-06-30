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
    logging_add_destination(make_shared<logfile>("/home/tyler/test.log"));

    logging_set_level(loglevel::trace);
}

int main(int argc, char **argv) {
    bootstrap();
    setup_logging();

    log_trace("tracing something");
    log_lots("lots of logging");
    log_debug("debugging it");
    log_verbose("verbosity = high");
    log_info("info: you have been told");
    log_notice("notice notice notice");
    log_warn("uhm that looks possibly not good");
    log_error("yeah that was actually ", "bad");

    log_fatal("this should ", "totally abort");

    logging_cleanup();

    return 0;
}
