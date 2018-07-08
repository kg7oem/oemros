#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "logging.h"
#include "module.h"
#include "runloop.h"
#include "radio.h"
#include "system.h"

using namespace oemros;
using namespace std;

void bootstrap(void) {
    logging_add_destination(make_shared<logstdio>());

    thread_bootstrap();
    radio_bootstrap();

    module_bootstrap();
}

void cleanup(void) {
    thread_cleanup();
}

void run(void) {
    auto test_module = module_create("test");
}

int main(int argc, char **argv) {
    bootstrap();
    run();
    cleanup();

    return 0;
}
