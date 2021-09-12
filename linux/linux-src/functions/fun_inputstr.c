#include "../common/file.h"

void fun_inputstr(void) {
    int nbr, fnbr;
    char *p;
    getargs(&ep, 3, ",");
    if (argc != 3) error("Invalid syntax");
    nbr = getinteger(argv[0]);
    if (nbr < 1 || nbr > MAXSTRLEN) error("Number out of bounds");
    if (*argv[2] == '#') argv[2]++;
    fnbr = getinteger(argv[2]);
    sret = GetTempStrMemory();  // this will last for the life of the command
    p = sret + 1;               // point to the start of the char array
    *sret = nbr;                // set the length of the returned string
    while (nbr) {
        if (MMfeof(fnbr)) break;
        *p++ = MMfgetc(fnbr);  // get the char and save in our returned string
        nbr--;
    }
    *sret -= nbr;  // correct if we get less than nbr chars
    targ = T_STR;
}
