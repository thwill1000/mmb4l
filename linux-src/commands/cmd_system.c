#include "../common/version.h"

void cmd_system(void) {
    int rc;

    rc = system(getCstring(cmdline));
    if (rc != 0) {
        error("Command could not be run");
    }
}
