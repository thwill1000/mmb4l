#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../common/error.h"
#include "../common/parse.h"
#include "../common/version.h"

void fun_lof(void) {
    int fnbr = parse_file_number(ep, false);
    if (fnbr == -1) ERROR_INVALID_FILE_NUMBER;

    targ = T_INT;
    fnbr--;
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
