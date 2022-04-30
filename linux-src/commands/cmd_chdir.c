#include <unistd.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/path.h"
#include "../common/utility.h"

void cmd_chdir(void) {
    char *path = GetTempStrMemory();
    if (!path_munge(getCstring(cmdline), path, STRINGSIZE)) error_system(errno);
    errno = 0;
    if (FAILED(chdir(path))) error_system(errno);
}
