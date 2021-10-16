#include "../common/version.h"

void cmd_end(void) {
    checkend(cmdline);
    longjmp(mark, JMP_END);
}
