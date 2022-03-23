#include <stdbool.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/parse.h"

#define PRINT_MMSTRING(s, fnbr)  MMfputs(s, fnbr)

void cmd_print(void) {

    char *s, *p;
    MMFLOAT f;
    MMINTEGER i64;
    int i, t, fnbr;
    bool docrlf = true;                                             // this is used to suppress the cr/lf if needed

    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ";,");               // this is a macro and must be the first executable stmt

    if (argc > 0 && *argv[0] == '#') {
        // First argument is a file number.
        fnbr = parse_file_number(argv[0], true);
        if (fnbr == -1) ERROR_INVALID_FILE_NUMBER;
        // Set the next argument to be looked at.
        i = 1;
        if (argc >= 2 && *argv[1] == ',') i = 2;
    } else {
        // Use standard output.
        fnbr = 0;
        i = 0;
    }

    for (; i < argc; i++) {                                         // step through the arguments
        if (*argv[i] == ',') {
            PRINT_MMSTRING("\1\t", fnbr);                           // print a tab for a comma
            docrlf = false;                                         // a trailing comma should suppress CR/LF
        }
        else if (*argv[i] == ';') {
            docrlf = false;                                         // other than suppress cr/lf do nothing for a semicolon
        }
        else {                                                      // we have a normal expression
            p = argv[i];
            while (*p) {
                t = T_NOTYPE;
                p = evaluate(p, &f, &i64, &s, &t, true);            // get the value and type of the argument
                if (t & T_NBR) {
                    *inpbuf = ' ';                                  // preload a space
                    FloatToStr(inpbuf + ((f >= 0) ? 1:0), f, 0, STR_AUTO_PRECISION, ' '); // if positive output a space instead of the sign
                    s = CtoM(inpbuf);                               // convert to a MMBasic string
                } else if (t & T_INT) {
                    *inpbuf = ' ';                                  // preload a space
                    IntToStr(inpbuf + ((i64 >= 0) ? 1:0), i64, 10); // if positive output a space instead of the sign
                    s = CtoM(inpbuf);                               // convert to a MMBasic string
                } else if (t & T_STR) {
                    // Do nothing, 's' is already the MMBasic string we wish to output.
                } else {
                    ERROR_INTERNAL_FAULT;
                }

                PRINT_MMSTRING(s, fnbr);
            }
            docrlf = true;
        }
    }

    if (docrlf) PRINT_MMSTRING("\2\r\n", fnbr);                     // print the terminating cr/lf unless it has been suppressed
}
