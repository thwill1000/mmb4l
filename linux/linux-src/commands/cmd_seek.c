#include <unistd.h>

#include "../common/error.h"
#include "../common/file.h"

void cmd_seek(void) {
    int fnbr, idx;
    getargs(&cmdline, 5, ",");
    if (argc != 3) error("Invalid syntax");
    if (*argv[0] == '#') argv[0]++;
    fnbr = getinteger(argv[0]) - 1;
    if (fnbr < 0 || fnbr >= 10) error("Invalid file number");
    if (MMFilePtr[fnbr] == NULL) error("File number is not open");
    idx = getinteger(argv[2]) - 1;
    if (idx < 0) error("Invalid seek position");
    fflush(MMFilePtr[fnbr]);
    fsync(fileno(MMFilePtr[fnbr]));
    fseek(MMFilePtr[fnbr], idx, SEEK_SET);
    error_check();
}
