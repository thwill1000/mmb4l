#include "../common/version.h"

void cmd_exitmmb(void) {
    checkend(cmdline);
    ExitMMBasicFlag = true;  // signal that we want out of here
    longjmp(mark, 1);        // jump back to the input prompt
}
