#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../common/error.h"
#include "../common/version.h"

void fun_lof(void) {
    targ = T_INT;

    skipspace(ep);
    if (*ep == '#') ep++;
    int fnbr = getinteger(ep) - 1;
    if (fnbr < 0 || fnbr >= 10) ERROR_INVALID("file number");

    switch (file_table[fnbr].type) {
        case fet_closed:
            ERROR_NOT_OPEN;
            break;

        case fet_file: {
            int pos = ftell(file_table[fnbr].file_ptr);
            FILE *f = file_table[fnbr].file_ptr;
            error_check();
            fseek(f, 0L, SEEK_END);
            error_check();
            iret = ftell(f);
            error_check();
            fseek(f, pos, SEEK_SET);
            error_check();
            break;
        }

        case fet_serial:
            iret = 0; // Serial I/O ports are unbuffered.
            break;
    }
}
