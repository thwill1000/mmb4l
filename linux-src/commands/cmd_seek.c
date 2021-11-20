#include <unistd.h>

#include "../common/error.h"
#include "../common/file.h"
#include "../common/version.h"

void cmd_seek(void) {
    getargs(&cmdline, 5, ",");
    if (argc != 3) ERROR_SYNTAX;

    if (*argv[0] == '#') argv[0]++;
    int fnbr = getinteger(argv[0]) - 1;
    if (fnbr < 0 || fnbr >= 10) ERROR_INVALID("file number");

    int idx = getinteger(argv[2]) - 1;
    if (idx < 0) ERROR_INVALID("seek position");

    if (file_table[fnbr].type == fet_closed) ERROR_NOT_OPEN;
    FILE *f = file_table[fnbr].file_ptr;

    fflush(f);
    fsync(fileno(f));
    fseek(f, idx, SEEK_SET);
    error_check();
}
