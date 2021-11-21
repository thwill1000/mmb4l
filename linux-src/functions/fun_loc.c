#include "../common/error.h"
#include "../common/parse.h"
#include "../common/version.h"

void fun_loc(void) {
    int fnbr = parse_file_number(ep, false);
    if (fnbr == -1) ERROR_INVALID_FILE_NUMBER;

    targ = T_INT;
    fnbr--;
    switch (file_table[fnbr].type) {
        case fet_closed:
            ERROR_NOT_OPEN;
            break;

        case fet_file:
            iret = ftell(file_table[fnbr].file_ptr) + 1;
            error_check();
            break;

        case fet_serial:
            ERROR_UNIMPLEMENTED("LOC() function for serial ports");
            // iret = SerialRxQueueSize(MMComPtr[fnbr]);
            break;
    }
}
