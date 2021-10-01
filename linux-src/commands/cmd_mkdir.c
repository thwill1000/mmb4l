#include <sys/stat.h>

#include "../common/error.h"
#include "../common/utility.h"
#include "../common/version.h"

void cmd_mkdir(void) {
    char *path = GetTempStrMemory();
    munge_path(getCstring(cmdline), path, STRINGSIZE);
    error_check();
    // TODO: check/validate mode/permissions.
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    error_check();
}
