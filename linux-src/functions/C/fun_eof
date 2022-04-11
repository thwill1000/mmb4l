#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"

void fun_eof(void) {
    getargs(&ep, 1, ",");
    if (argc == 0) ERROR_SYNTAX;

    int fnbr = parse_file_number(argv[0], true);
    if (fnbr == -1) ERROR_INVALID_FILE_NUMBER;

    targ = T_INT;
    iret = file_eof(fnbr);
}
