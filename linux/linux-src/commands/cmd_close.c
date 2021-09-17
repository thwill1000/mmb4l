#include "../common/version.h"

void cmd_close(void) {
    int i;
    // getargs() macro must be first executable statement in a block.
    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",");
    if ((argc & 0x01) == 0) error("Invalid syntax");

    for (i = 0; i < argc; i += 2) {
        if (*argv[i] == '#') argv[i]++;
        MMfclose(getinteger(argv[i]));
    }
}
