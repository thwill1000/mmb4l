/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmgetline.c

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "console.h"
#include "display.h"
#include "error.h"
#include "exit_codes.h"
#include "keycodes.h"
#include "mmb4l.h"
#include "mmtime.h"
#include "options.h"
#include "streamio.h"

void CheckAbort(void);

// get a keystroke.  Will wait forever for input
// if the char is a lf then replace it with a cr
// unless it was preceded by a cr and in that case throw away the char
// so console end of line is always cr
int MMgetchar(void) {
    static char prevchar = 0;
    int c;
    ON_FAILURE_ERROR_EX(display_show_cursor(true), -1);
    for (;;) {
        ON_FAILURE_ERROR_EX(display_update_cursor(), -1);
        c = console_getc();
        if (c == -1) {
            if (!isatty(STDIN_FILENO)) {
                // In this case there will never be anything to read.
                if (MMCharPos > 1) display_puts("\r\n");
                display_puts("Error: STDIN exhausted\r\n");
                mmb_state.exit_code = EX_FAIL;
                ON_FAILURE_ERROR_EX(display_show_cursor(false), -1);
                longjmp(mark, JMP_QUIT);
            }
            nanosleep(&ONE_MILLISECOND, NULL);
        // } else if (c == 3) {
        //     longjmp(mark, JMP_BREAK); // jump back to the input prompt if CTRL-C
        } else if (c == '\n' && prevchar == '\r') {
            prevchar = 0;
        } else {
            break;
        }
    }
    prevchar = c;
    ON_FAILURE_ERROR_EX(display_show_cursor(false), -1);
    return c == '\n' ? '\r' : c;
}

// get a line from the keyboard or a file handle
void MMgetline(int fnbr, char *p) {
    int c, nbrchars = 0;
    const char *tp;

    while (1) {
        CheckAbort();  // jump right out if CTRL-C

        if (streamio_is_file(fnbr) && streamio_eof(fnbr)) break; // End of file.
        c = streamio_getc(fnbr);

        // -1 - no character.
        //  0 - the null character which we ignore.
        if (c <= 0) continue;

        // if this is the console, check for a programmed function key and
        // insert the text
        if (fnbr == 0) {
            tp = NULL;
            if (c == F2) tp = "RUN";
            if (c == F3) tp = "LIST";
            if (c == F4) tp = "EDIT";
            if (c == F5) tp = "WEDIT";
            if (tp) {
                strcpy(p, tp);
                display_puts(tp);
                display_puts("\r\n");
                return;
            }
        }

        if (c == '\t') {  // expand tabs to spaces
            do {
                if (++nbrchars > MAXSTRLEN) error_throw(kLineTooLong);
                *p++ = ' ';
                if (fnbr == 0) display_putc(' ');
            } while (nbrchars % mmb_options.tab);
            continue;
        }

        if (c == '\b') {  // handle the backspace
            if (nbrchars) {
                if (fnbr == 0) display_puts("\b \b");
                nbrchars--;
                p--;
            }
            continue;
        }

        if (c == '\n') {  // what to do with a newline
            break;        // a newline terminates a line (for a file or serial)
        }

        if (c == '\r') {
            if (fnbr == 0) {
                display_puts("\r\n");
                break;  // on the console this means the end of the line
                        // - stop collecting
            } else {
                continue;  // for files and serial loop around looking for the
                           // following newline
            }
        }

        if (isprint(c) && (fnbr == 0)) {
            display_putc(c);  // The console requires that chars be echoed
        }

        if (++nbrchars > MAXSTRLEN) error_throw(kLineTooLong);  // stop collecting if maximum length

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
