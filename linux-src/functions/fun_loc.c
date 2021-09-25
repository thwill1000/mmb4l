#include "../common/error.h"
#include "../common/version.h"

void fun_loc(void) {
    int fnbr;
    targ = T_INT;
    skipspace(ep);
    if(*ep == '#') ep++;
    fnbr = getinteger(ep) - 1;
    if(fnbr < 0 || fnbr >= 10) error("Invalid file number");
    if(MMComPtr[fnbr] != NULL) {
        // iret = SerialRxQueueSize(MMComPtr[fnbr]);                   // it is a serial I/O port
    } else {
        if(MMFilePtr[fnbr] == NULL) error("File number is not open");
        iret = ftell(MMFilePtr[fnbr]) + 1;                          // it is a file
        if (error_check()) return;
    }
}
