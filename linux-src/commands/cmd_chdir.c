#include <unistd.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/utility.h"

void cmd_chdir(void) {
    char *path = GetTempStrMemory();
    munge_path(getCstring(cmdline), path, STRINGSIZE);
    error_check();
    int result = chdir(path);
    error_check();
}
