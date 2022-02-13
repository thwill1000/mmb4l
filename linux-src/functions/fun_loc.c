#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"

void fun_loc(void) {
    int fnbr = parse_file_number(ep, false);
    if (fnbr == -1) ERROR_INVALID_FILE_NUMBER;
    targ = T_INT;
    iret = file_loc(fnbr);
}
