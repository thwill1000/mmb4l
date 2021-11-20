#include "../common/error.h"
#include "../common/file.h"
#include "../common/version.h"

void fun_eof(void) {
    getargs(&ep, 1, ",");
    if (argc == 0) ERROR_SYNTAX;
    targ = T_INT;
    if (*argv[0] == '#') argv[0]++;
    iret = file_eof(getinteger(argv[0]));
}
