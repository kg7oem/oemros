#include "logging.h"
#include "module.h"
#include "runloop.h"
#include "radio.h"
#include "system.h"

using namespace oemros;
using namespace std;

void run(void) {
    auto test_thread = module_spawn("test_module");
    log_trace("joining to test module thread");
    test_thread->join();
    delete test_thread;
}

void bootstrap(void) {
//    logging_add_destination(make_shared<logstdio>());
    logging_start();

    thread_bootstrap();
    radio_bootstrap();

    module_bootstrap();
}

int main(int /* argc */, char ** /* argv */) {
    bootstrap();
    run();
    return 0;
}
