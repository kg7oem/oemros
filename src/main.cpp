#include "event.h"
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

void some_func(int blah, UNUSED bool foo) {
    log_info("Something else: ", blah);
}

void run() {

    auto foo_strong = promise<int>::make();
    log_info("count: ", foo_strong.use_count());
    promise<int>* foo_p = foo_strong.get();
    promise_s<int> something = foo_p->strong_ref();
    // Do want this
//    promise_s<int> something_else = foo_p;
    log_info("count: ", foo_strong.use_count());

    auto sink = foo_strong->sink();
    sink(5);
    log_info("Got from promise: ", foo_strong->get());

    event_source<int, const char*> another_source;

    another_source.subscribe([](int value, const char* foo) {
        log_info("got value: ", value);
        log_info("got foo: ", foo);
    });

    another_source.subscribe(some_func);

    another_source.deliver(1, "hello");

    auto new_task = task_spawn("some task", "test_module");
    auto main_loop = runloop::make();

    auto signal_handler = main_loop->make_started<rlsignal>(signame::INT, sigint_handler);

    signal_handler->events.will_execute.subscribe([] () { log_info("wow!!!!!!!"); });

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
