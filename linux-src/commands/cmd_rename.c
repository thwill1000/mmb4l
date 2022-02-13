#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/utility.h"

void cmd_rename(void) {
    char ss[2] = { tokenAS, 0 };
    getargs(&cmdline, 3, ss);  // must be first executable statement in block
    if (argc != 3) ERROR_SYNTAX;

    char *old_path = GetTempStrMemory();
    munge_path(getCstring(argv[0]), old_path, STRINGSIZE);
    error_check();

    char *new_path = GetTempStrMemory();
    munge_path(getCstring(argv[2]), new_path, STRINGSIZE);
    error_check();

    rename(old_path, new_path);
    error_check();
}
