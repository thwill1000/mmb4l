#include <unistd.h>

#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"
#include "../common/version.h"

void cmd_seek(void) {
    getargs(&cmdline, 3, ",");
    if (argc != 3) ERROR_SYNTAX;

    int fnbr = parse_file_number(argv[0], false);
    if (fnbr == -1) ERROR_INVALID_FILE_NUMBER;

    int idx = getinteger(argv[2]) - 1;
    if (idx < 0) ERROR_INVALID("seek position");

    fnbr--;
    if (file_table[fnbr].type == fet_closed) ERROR_NOT_OPEN;
    FILE *f = file_table[fnbr].file_ptr;

    fflush(f);
    fsync(fileno(f));
    fseek(f, idx, SEEK_SET);
    error_check();
}
