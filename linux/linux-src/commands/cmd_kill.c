#include "../common/error.h"
#include "../common/version.h"

void cmd_kill(void) {
    char *p = getCstring(cmdline);  // get the file name and convert to a standard C string

    errno = 0;
    remove(p);
    error_check();
}
