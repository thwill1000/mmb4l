#include "../common/mmb4l.h"
#include "../common/console.h"
#include "../common/error.h"

void cmd_settitle(void) {
    getargs(&cmdline, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    console_set_title(getCstring(argv[0]));
}
