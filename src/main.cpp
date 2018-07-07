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
    radio_bootstrap();
}

int main(int argc, char **argv) {
    logging_add_destination(make_shared<logstdio>());
    logging_set_level(loglevel::trace);

    bootstrap();

    radio_s thing = radio::create(1);

    threadpool_schedule([]{ log_trace("I ran in the threadpool!"); });

    while(1) { std::this_thread::sleep_for(std::chrono::milliseconds(1000)); }

    return 0;
}
