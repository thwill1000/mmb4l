#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/path.h"

void cmd_kill(void) {
    char *path = GetTempStrMemory();
    path_munge(getCstring(cmdline), path, STRINGSIZE);
    error_check();
    remove(path);
    error_check();
}
