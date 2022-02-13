#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/utility.h"

void cmd_kill(void) {
    char *path = GetTempStrMemory();
    munge_path(getCstring(cmdline), path, STRINGSIZE);
    error_check();
    remove(path);
    error_check();
}
