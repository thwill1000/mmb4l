#include <sys/stat.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/utility.h"

void cmd_mkdir(void) {
    char *path = GetTempStrMemory();
    munge_path(getCstring(cmdline), path, STRINGSIZE);
    error_check();
    // TODO: check/validate mode/permissions.
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    error_check();
}
