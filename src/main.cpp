#include <chrono>
#include <iostream>
#include <memory.h>
#include <thread>

#include "logging.h"
#include "runloop.h"
#include "radio.h"
#include "system.h"

using namespace oemros;
using namespace std;

void bootstrap(void) {
    threadpool_bootstrap();
    radio_bootstrap();
}

int main(int argc, char **argv) {
    logging_add_destination(make_shared<logstdio>());
    logging_set_level(loglevel::trace);

    bootstrap();

    auto thing = radio::create(1);

    thing->frequency(100)->wait();

    auto freq = thing->frequency();

    log_fatal("got freq: ", freq->get());

    return 0;
}
