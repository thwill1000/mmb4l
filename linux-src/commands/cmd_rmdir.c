#include <unistd.h>

#include "../common/error.h"
#include "../common/version.h"

void cmd_rmdir(void) {
    char *p;

    p = getCstring(cmdline);  // get the directory name and convert to a standard C string
    errno = 0;
    rmdir(p);
    error_check();
}
