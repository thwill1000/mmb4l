#include "../common/version.h"

void fun_eof(void) {
    getargs(&ep, 1, ",");
    targ = T_INT;
    if (argc == 0) error("Invalid syntax");
    if (*argv[0] == '#') argv[0]++;
    iret = MMfeof(getinteger(argv[0]));
}
