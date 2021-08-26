#include "../common/version.h"

void cmd_colour(void) {
    int fc, bc;
    getargs(&cmdline, 3,
            ",");  // getargs macro must be the first executable stmt in a block
    if (argc != 3) error("Syntax");
    fc = getint(argv[0], 0, 0x0f);
    bc = getint(argv[2], 0, 0x0f);
    DOSColour(fc, bc);
}
