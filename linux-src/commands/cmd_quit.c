#include "../common/version.h"

void cmd_quit(void) {
    checkend(cmdline);
    longjmp(mark, JMP_QUIT);
}
