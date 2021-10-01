#include "../common/version.h"

void cmd_system(void) {
    int result = system(getCstring(cmdline));
    if (result != 0) {
        MMerrno = result;
        error("System command failed, exit code [%]", result);
    }
}
