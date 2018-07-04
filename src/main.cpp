#include <iostream>
#include <memory.h>

#include "logging.h"
#include "runloop.h"
#include "system.h"

using namespace oemros;
using namespace std;

REFCOUNTED(mctesterson) {
    public:
        mctesterson() = default;
};

int main(int argc, char **argv) {

    logging_add_destination(make_shared<logstdio>());
    logging_set_level(loglevel::trace);

    auto loop = runloop::create();
    auto item = loop->create_item<rlitem>();
    auto welp = mctesterson::create();

    log_debug("This thing --> ", welp);

    log_debug("going to exit main");

    return 0;
}
