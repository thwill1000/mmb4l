#include "../common/version.h"

void cmd_files(void) {
    char command[STRINGSIZE];

    skipspace(cmdline);
    if (*cmdline != '\0' && *cmdline != '\\') {
        sprintf(command, "ls \"%s\"", getCstring(cmdline));
    } else {
        sprintf(command, "ls");
    }

    int result = system(command);
    // if (result != 0) error("System command failed");
}
