#include <unistd.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"

void cmd_seek(void) {
    getargs(&cmdline, 3, ",");
    if (argc != 3) ERROR_SYNTAX;

    int fnbr = parse_file_number(argv[0], false);
    if (fnbr == -1) ERROR_INVALID_FILE_NUMBER;

    int idx = getinteger(argv[2]);
    if (idx < 1) ERROR_INVALID("seek position");

    file_seek(fnbr, idx);
}
