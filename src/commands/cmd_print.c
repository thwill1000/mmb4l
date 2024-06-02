/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_print.c

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"

void cmd_print(void) {

    char *s;
    const char *p;
    MMFLOAT f;
    MMINTEGER i64;
    int i, t, fnbr;
    bool docrlf = true;                                             // this is used to suppress the cr/lf if needed

    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ";,");               // this is a macro and must be the first executable stmt

    if (argc > 0 && *argv[0] == '#') {
        // First argument is a file number.
        fnbr = parse_file_number(argv[0], true);
        if (fnbr == -1) {
            error_throw(kFileInvalidFileNumber);
            return;
        }
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
            file_write(fnbr, "\t", 1);                              // print a tab for a comma
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

                file_write(fnbr, s + 1, (size_t) s[0]);
            }
            docrlf = true;
        }
    }

    if (docrlf) file_write(fnbr, "\r\n", 2);                        // print the terminating cr/lf unless it has been suppressed
}
