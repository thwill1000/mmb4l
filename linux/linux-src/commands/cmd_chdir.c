#include <unistd.h>

#include "../common/error.h"
#include "../common/utility.h"
#include "../common/version.h"

void cmd_chdir(void) {
    // Get the directory name and convert to a standard C string.
    char *dir = getCstring(cmdline);
    char path[STRINGSIZE];
    if (!munge_path(dir, path, STRINGSIZE)) {
        if (error_check()) return;
    }
    errno = 0;
    int result = chdir(path);
    error_check();
}
