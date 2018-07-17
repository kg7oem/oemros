#include "logging.h"
#include "module.h"
#include "runloop.h"
#include "radio.h"
#include "system.h"

using namespace oemros;

void sigint_handler(UNUSED signame signum) {
    assert(signum == signame::INT);
    log_info("Got SIGINT");
    system_exit(exitvalue::ok);
}

void run() {
    auto main_loop = runloop::make();

    main_loop->make_started<rlsignal>(signame::INT, sigint_handler);

    log_info("going into main runloop");
    main_loop->enter();
    log_info("out of main runloop");
}

void bootstrap() {
    logging_start();

    thread_bootstrap();
    radio_bootstrap();

    module_bootstrap();
}

int main(UNUSED int argc, UNUSED char ** argv) {
    bootstrap();
    run();
    return 0;
}
