#include <stdlib.h>

#include "../common/version.h"

void cmd_files_internal(char *p) {
    char *command = GetTempStrMemory();

    skipspace(p);
    if (*p != '\0' && *p != '\'') {
        snprintf(command, STRINGSIZE, "ls %s", getCstring(p));
    } else {
        snprintf(command, STRINGSIZE, "ls");
    }

    int result = system(command);
    // if (result != 0) error("System command failed");

    MMPrintString("\r\n");
}

void cmd_files(void) {
    cmd_files_internal(cmdline);
}
