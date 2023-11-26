#include <stdio.h>
#include "constants.h"
#include "webserver.h"
#include "board.h"

int main() {
    board_init();
    webserver_run();

    return 0;
}

