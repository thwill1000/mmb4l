#include "../common/error.h"
#include "../common/version.h"

void fun_loc(void) {
    targ = T_INT;

    skipspace(ep);
    if (*ep == '#') ep++;
    int fnbr = getinteger(ep) - 1;
    if (fnbr < 0 || fnbr >= 10) ERROR_INVALID("file number");

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
