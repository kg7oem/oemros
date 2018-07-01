#include <iostream>
#include <memory.h>

#include "logging.h"

using namespace oemros;
using namespace std;

int main(int argc, char **argv) {

    logging_add_destination(make_shared<logstdio>());
    log_error("testing: ", errstream);

    return 0;
}
