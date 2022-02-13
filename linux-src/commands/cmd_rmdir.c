#include <unistd.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/utility.h"

void cmd_rmdir(void) {
    char *path = GetTempStrMemory();
    munge_path(getCstring(cmdline), path, STRINGSIZE);
    error_check();
    rmdir(path);
    error_check();
}
