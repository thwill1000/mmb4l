#include <stdio.h>
#include <stdlib.h>

#include "../common/mmb4l.h"
#include "../common/memory.h"

void cmd_files_internal(char *p) {
    char *command = GetTempStrMemory();

    skipspace(p);
    if (*p != '\0' && *p != '\'') {
        snprintf(command, STRINGSIZE, "ls %s", getCstring(p));
    } else {
        snprintf(command, STRINGSIZE, "ls");
    }

    int result = system(command);
    // if (result != 0) ERROR_SYSTEM_COMMAND_FAILED;

    MMPrintString("\r\n");
}

void cmd_files(void) {
    cmd_files_internal(cmdline);
}
