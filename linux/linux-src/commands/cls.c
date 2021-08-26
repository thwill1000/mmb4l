#include "../common/version.h"

void cmd_cls(void) {
    // int rc;

    checkend(cmdline);
    console_clear();
    // rc = system("CLS");
    // if (rc != 0) {
    //     error("Command could not be run");
    // }
}
