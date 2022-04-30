#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/error.h"

#define ERROR_ONLY_ONE_FORMAT_SPECIFIER_ALLOWED  error_throw_ex(kError, "Only one format specifier (%) allowed")
#define ERROR_ILLEGAL_CHARACTER_IN_FORMAT        error_throw_ex(kError, "Illegal character in format specification")
#define ERROR_FORMAT_SPECIFICATION_NOT_FOUND     error_throw_ex(kError, "Format specification not found")

#define IsDigitinline(a) ( a >= '0' && a <= '9' )

void fun_format(void) {
    char *p, *fmt;
    int inspec;
    getargs(&ep, 3, ",");
    if (argc % 2 == 0) ERROR_SYNTAX;
    if (argc == 3)
        fmt = getCstring(argv[2]);
    else
        fmt = "%g";

    // check the format string for errors that might crash the CPU
    for (inspec = 0, p = fmt; *p; p++) {
        if (*p == '%') {
            inspec++;
            if (inspec > 1) ERROR_ONLY_ONE_FORMAT_SPECIFIER_ALLOWED;
            continue;
        }

        if (inspec == 1 && (*p == 'g' || *p == 'G' || *p == 'f' || *p == 'e' ||
                            *p == 'E' || *p == 'l'))
            inspec++;

        if (inspec == 1 && !(IsDigitinline(*p) || *p == '+' || *p == '-' ||
                             *p == '.' || *p == ' '))
            ERROR_ILLEGAL_CHARACTER_IN_FORMAT;
    }
    if (inspec != 2) ERROR_FORMAT_SPECIFICATION_NOT_FOUND;
    sret = GetTempStrMemory();  // this will last for the life of the command
    sprintf(sret, fmt, getnumber(argv[0]));
    CtoM(sret);
    targ = T_STR;
}
