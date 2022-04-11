#include "../common/mmb4l.h"
#include "../common/console.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"

void fun_inputstr(void) {
    getargs(&ep, 3, ",");
    if (argc != 3) ERROR_SYNTAX;

    int nbr = getint(argv[0], 1, MAXSTRLEN);
    int fnbr = parse_file_number(argv[2], true);

    targ = T_STR;
    sret = GetTempStrMemory();

    if (fnbr == 0) {  // accessing the console
        int i;
        for (i = 1; i <= nbr && console_kbhit(); i++) {
            sret[i] = console_getc();
        }
        *sret = i - 1;
    } else {
        char *p = sret + 1;  // point to the start of the char array
        *sret = nbr;         // set the length of the returned string
        while (nbr) {
            if (file_eof(fnbr)) break;
            *p++ = file_getc(fnbr);
            nbr--;
        }
        *sret -= nbr;  // correct if we get less than nbr chars
    }
}
