/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmgetline.c

Copyright 2021-2023 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include "console.h"
#include "error.h"
#include "file.h"
#include "options.h"

#include <ctype.h>
#include <string.h>

void CheckAbort(void);

// get a line from the keyboard or a file handle
void MMgetline(int filenbr, char *p) {
    int c, nbrchars = 0;
    const char *tp;

    while (1) {
        CheckAbort();  // jump right out if CTRL-C

        if ((file_table[filenbr].type == fet_file) && file_eof(filenbr)) break; // End of file.
        c = file_getc(filenbr);

        // -1 - no character.
        //  0 - the null character which we ignore.
        if (c <= 0) continue;

        // if this is the console, check for a programmed function key and
        // insert the text
        if (filenbr == 0) {
            tp = NULL;
            if (c == F2) tp = "RUN";
            if (c == F3) tp = "LIST";
            if (c == F4) tp = "EDIT";
            if (c == F5) tp = "WEDIT";
            if (tp) {
                strcpy(p, tp);
                console_puts(tp);
                console_puts("\r\n");
                return;
            }
        }

        if (c == '\t') {  // expand tabs to spaces
            do {
                if (++nbrchars > MAXSTRLEN) ERROR_LINE_TOO_LONG;
                *p++ = ' ';
                if (filenbr == 0) console_putc(' ');
            } while (nbrchars % mmb_options.tab);
            continue;
        }

        if (c == '\b') {  // handle the backspace
            if (nbrchars) {
                if (filenbr == 0) console_puts("\b \b");
                nbrchars--;
                p--;
            }
            continue;
        }

        if (c == '\n') {  // what to do with a newline
            break;        // a newline terminates a line (for a file or serial)
        }

        if (c == '\r') {
            if (filenbr == 0) {
                console_puts("\r\n");
                break;  // on the console this means the end of the line
                        // - stop collecting
            } else {
                continue;  // for files and serial loop around looking for the
                           // following newline
            }
        }

        if (isprint(c) && (filenbr == 0)) {
            console_putc(c);  // The console requires that chars be echoed
        }

        if (++nbrchars > MAXSTRLEN) ERROR_LINE_TOO_LONG;  // stop collecting if maximum length

        // TODO: currently this function can return strings containing control
        //       characters, i.e. c < 32.
        //       Perhaps we should replace these with another character such as
        //       '?' or with a hex code <02> or <0x02>.
        //       The same might apply to c = 0 which we currently ignore.
        //       Possibly the behaviour could be controlled by an OPTION.

        *p++ = c;  // save our char
    }
    *p = 0;
}
