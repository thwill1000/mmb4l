#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../common/error.h"
#include "../common/file.h"

void fun_lof(void) {
    int fnbr, pos;
    struct stat buf;
    targ = T_INT;
    skipspace(ep);
    if (*ep == '#') ep++;
    fnbr = getinteger(ep) - 1;
    if (fnbr < 0 || fnbr >= 10) error("Invalid file number");
    if (MMComPtr[fnbr] != NULL) {
        iret = 0;  // it is a serial I/O port and they are unbuffered
    } else {
        if (MMFilePtr[fnbr] == NULL) error("File number is not open");
        pos = ftell(MMFilePtr[fnbr]);
        if (error_check()) return;
        fseek(MMFilePtr[fnbr], 0L, SEEK_END);
        if (error_check()) return;
        iret = ftell(MMFilePtr[fnbr]);
        if (error_check()) return;
        fseek(MMFilePtr[fnbr], pos, SEEK_SET);
        if (error_check()) return;
    }
}
