#include <stdio.h>

#include "../common/mmb4l.h"

#define IsDigitinline(a) ( a >= '0' && a <= '9' )

void fun_format(void) {
    char *p, *fmt;
    int inspec;
    getargs(&ep, 3, ",");
    if (argc % 2 == 0) error("Invalid syntax");
    if (argc == 3)
        fmt = getCstring(argv[2]);
    else
        fmt = "%g";

    // check the format string for errors that might crash the CPU
    for (inspec = 0, p = fmt; *p; p++) {
        if (*p == '%') {
            inspec++;
            if (inspec > 1) error("Only one format specifier (%) allowed");
            continue;
        }

        if (inspec == 1 && (*p == 'g' || *p == 'G' || *p == 'f' || *p == 'e' ||
                            *p == 'E' || *p == 'l'))
            inspec++;

        if (inspec == 1 && !(IsDigitinline(*p) || *p == '+' || *p == '-' ||
                             *p == '.' || *p == ' '))
            error("Illegal character in format specification");
    }
    if (inspec != 2) error("Format specification not found");
    sret = GetTempStrMemory();  // this will last for the life of the command
    sprintf(sret, fmt, getnumber(argv[0]));
    CtoM(sret);
    targ = T_STR;
}
