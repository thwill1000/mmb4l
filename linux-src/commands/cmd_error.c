#include "../common/mmb4l.h"
#include "../common/error.h"

void cmd_error(void) {
    getargs(&cmdline, 3, ",");

    switch (argc) {
        case 0:
            ERROR_UNKNOWN_USER_ERROR;
            break;
        case 1: {
            char *s = getCstring(argv[0]);
            error_throw_ex(kError, s);
            break;
        }
        case 3: {
            char *s = getCstring(argv[0]);
            int32_t i = getint(argv[2], INT32_MIN, INT32_MAX);
            if (i == 0) ERROR_INVALID("error code");
            error_throw_ex(i, s);
            break;
        }
        default:
            ERROR_SYNTAX;
            break;
    }
}
