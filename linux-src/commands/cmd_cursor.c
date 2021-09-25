#include "../common/console.h"
#include "../common/error.h"
#include "../common/version.h"

void cmd_cursor(void) {
    getargs(&cmdline, 3, ",");
    if (argc != 3) ERROR_ARGUMENT_COUNT;
    int x = getint(argv[0], 0, 255);
    int y = getint(argv[2], 0, 255);
    console_set_cursor_pos(x, y);
}
