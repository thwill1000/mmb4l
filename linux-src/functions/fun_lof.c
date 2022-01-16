#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"
#include "../common/version.h"

void fun_lof(void) {
    int fnbr = parse_file_number(ep, false);
    if (fnbr == -1) ERROR_INVALID_FILE_NUMBER;
    targ = T_INT;
    iret = file_lof(fnbr);
}
