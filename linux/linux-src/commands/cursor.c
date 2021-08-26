#include "../common/version.h"

void cmd_cursor(void) {
    getargs(&cmdline, 3,
            ",");  // getargs macro must be the first executable stmt in a block
    if (argc != 3) error("Syntax");
    DOSCursor(getinteger(argv[0]), getinteger(argv[2]));
}
