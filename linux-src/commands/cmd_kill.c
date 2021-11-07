#include "../common/error.h"
#include "../common/utility.h"
#include "../common/version.h"

void cmd_kill(void) {
    char *path = GetTempStrMemory();
    munge_path(getCstring(cmdline), path, STRINGSIZE);
    error_check();
    remove(path);
    error_check();
}
