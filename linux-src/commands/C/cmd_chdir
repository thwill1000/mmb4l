#include <unistd.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/path.h"

void cmd_chdir(void) {
    char *path = GetTempStrMemory();
    path_munge(getCstring(cmdline), path, STRINGSIZE);
    error_check();
    int result = chdir(path);
    error_check();
}
