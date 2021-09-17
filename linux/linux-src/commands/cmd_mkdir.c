#include <sys/stat.h>

#include "../common/error.h"
#include "../common/utility.h"
#include "../common/version.h"

void cmd_mkdir(void) {
    // Get the directory name and convert to a C-string.
    char *p = getCstring(cmdline);
    char path[STRINGSIZE];
    if (!munge_path(p, path, STRINGSIZE)) {
        if (error_check()) return;
    }
    errno = 0;
    // TODO: check/validate mode/permissions.
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    error_check();
}
