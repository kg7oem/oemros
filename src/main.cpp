#include "logging.h"
#include "module.h"
#include "runloop.h"
#include "radio.h"
#include "system.h"

using namespace oemros;
using namespace std;

void sigint_handler(void) {
    log_info("Got SIGINT");
    system_exit(exitvalue::ok);
}

void run() {
    auto main_loop = runloop::make();
    auto int_handler = main_loop->make_started<rlsignal>(SIGINT, sigint_handler);

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
