#include "../common/error.h"
#include "../common/file.h"
#include "../common/version.h"

void cmd_close(void) {
    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",");
    if ((argc & 0x01) == 0) ERROR_SYNTAX;

    for (int i = 0; i < argc; i += 2) {
        if (*argv[i] == '#') argv[i]++;
        file_close(getinteger(argv[i]));
    }
}
