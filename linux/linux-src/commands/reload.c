#include "../common/version.h"

int ErrorCheck(void);

void cmd_reload(void) {
    checkend(cmdline);
    if (CurrentLinePtr != NULL) error("Invalid in a program");
    char buf[STRINGSIZE + 2];
    sprintf(buf, "\"%s\"", CurrentFile);
    if (!FileLoadProgram(buf)) return;
    if (ErrorCheck()) return;
}
