#include "../common/error.h"
#include "../common/version.h"

void cmd_system(void) {
    int result = system(getCstring(cmdline));
    if (result != 0) {
        error_code(result, "System command failed, exit code [%]", result);
    }
}
