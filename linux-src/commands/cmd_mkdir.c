#include <sys/stat.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/path.h"
#include "../common/utility.h"

void cmd_mkdir(void) {
    char *path = GetTempStrMemory();
    if (!path_munge(getCstring(cmdline), path, STRINGSIZE)) error_throw(errno);
    // TODO: check/validate mode/permissions.
    errno = 0;
    if FAILED(mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) error_throw(errno);
}
