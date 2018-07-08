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

ABSTRACT(A) {

};

OBJECT(B, public A) {
    OBJSTUFF(B);
};

void bootstrap(void) {
    thread_bootstrap();
    radio_bootstrap();
}

void cleanup(void) {
    thread_cleanup();
}

void run(void) {
    auto radio = hamlib::create(1);

    log_info("start: ", radio->ptt()->get());
    radio->ptt(true)->wait();
    log_info("now: ", radio->ptt()->get());
    radio->ptt(false)->wait();
    log_info("and then: ", radio->ptt()->get());

    auto mode = radio->mode()->get();
    log_info("got mode back: modulation = ", (int)mode->modulation());
    if (mode->data_mode()) {
        log_info("data mode is on");
    } else {
        log_info("data mode is off");
    }

    mode->data_mode(false);
    mode->modulation(modulation_t::wfm);
    radio->mode(mode)->wait();
}

int main(int argc, char **argv) {
    logging_add_destination(make_shared<logstdio>());
    logging_set_level(loglevel::trace);

    bootstrap();
    run();
    cleanup();

    return 0;
}
