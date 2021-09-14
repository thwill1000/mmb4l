#include "../common/console.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/version.h"

void fun_inputstr(void) {
    getargs(&ep, 3, ",");
    if (argc != 3) ERROR_SYNTAX;
    int nbr = getint(argv[0], 1, MAXSTRLEN);
    if (*argv[2] == '#') argv[2]++;
    sret = GetTempStrMemory();  // this will last for the life of the command
    targ = T_STR;
    int fnbr = getinteger(argv[2]);

    if (fnbr == 0) {  // accessing the console
        int i;
        for (i = 1; i <= nbr && console_kbhit(); i++) {
            sret[i] = getConsole();  // get the char from the console input
                                     // buffer and save in our returned string
        }
        *sret = i - 1;
    } else {
        char *p = sret + 1;  // point to the start of the char array
        *sret = nbr;         // set the length of the returned string
        while (nbr) {
            if (MMfeof(fnbr)) break;
            *p++ =
                MMfgetc(fnbr);  // get the char and save in our returned string
            nbr--;
        }
        *sret -= nbr;  // correct if we get less than nbr chars
    }
}
