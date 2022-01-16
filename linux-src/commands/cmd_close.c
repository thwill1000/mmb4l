#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"
#include "../common/version.h"

void cmd_close(void) {
    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",");
    if ((argc & 0x01) == 0) ERROR_SYNTAX;

    for (int i = 0; i < argc; i += 2) {
        int fnbr = parse_file_number(argv[i], false);
        if (fnbr == -1) ERROR_INVALID_FILE_NUMBER;
        file_close(fnbr);
    }
}
