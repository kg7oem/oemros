#include <iostream>

#include "logging.h"

int main(int argc, char **argv) {

    log_debug("test ", 1);
    log_debug("foo");

    log_fatal("this should ", "totally abort");

    return 0;
}
