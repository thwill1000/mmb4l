/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_autosave.c

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
#include <errno.h>
#include <stdbool.h>

#include "../common/mmb4l.h"
#include "../common/console.h"
#include "../common/cstring.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/memory.h"
#include "../common/parse.h"
#include "../common/path.h"
#include "../common/program.h"
#include "../common/utility.h"

/** Gets the path of the file to write to. */
static void cmd_autosave_get_file_path(char *file_path) {
    char *filename = getCstring(cmdline);
    MmResult result = path_munge(filename, file_path, STRINGSIZE);
    if (FAILED(result)) error_throw(result);

    if (strlen(path_get_extension(file_path)) == 0) {
        if (strlen(file_path) > MAXSTRLEN - 4) ERROR_PATH_TOO_LONG;
        cstring_cat(file_path, ".bas", STRINGSIZE);
    }
}

/** Reads input from the console into the buffer. */
static int cmd_autosave_read(char *buf) {
    int ch;
    int count = 0;
    char *p = buf;
    char previous = '\0';

    for (;;) {
        ch = console_getc();

        switch (ch) {
            case -1:
                continue;
            case F1:
            case F2:
            case F6:
                goto cmd_autosave_read_exit;
            case TAB:
                ch = ' ';
                break;
            case '\n':
                if (p == buf) continue;  // Throw away an initial line feed
                                         // which can follow the command.
                break;

        }

        if ((p - buf) >= EDIT_BUFFER_SIZE) ERROR_OUT_OF_MEMORY;

        if ((isprint(ch) && (previous == '\r'))
                || (ch == '\r' && previous == '\r')
                || (ch == '\n')) {
            *p++ = '\n';
            count = 0;
            console_putc('\n');
        }

        if (isprint(ch)) {
            *p++ = ch;
            if (count++ > 240) ERROR_LINE_LENGTH;
            console_putc(ch);
        }

        previous = ch;
    }

cmd_autosave_read_exit:

    if (previous == '\r') *p++ = '\n';
    *p = '\0'; // Terminate with a NULL.

    if (MMCharPos > 1) console_putc('\n');

    return ch;
}

/** Writes out the file. */
static void cmd_autosave_write_file(char *file_path, char *buf) {
    int fnbr = file_find_free();
    file_open(file_path, "wb", fnbr);
    char *p = buf;
    while (*p) {
        file_putc(fnbr, *p++);
    }
    file_close(fnbr);
}

void cmd_autosave(void) {
    if (CurrentLinePtr) ERROR_INVALID_IN_PROGRAM;

    char file_path[STRINGSIZE]; // Don't use GetTempStrMemory() because it will
                                // be cleared when we call ClearProgram() later.
    cmd_autosave_get_file_path(file_path);
    ClearProgram();             // Clear leftovers from the previous program.
    char *buf = GetTempMemory(EDIT_BUFFER_SIZE);
    int exit_key = cmd_autosave_read(buf);
    cmd_autosave_write_file(file_path, buf);

    if (path_has_suffix(file_path, ".bas", true)) {
        program_load_file(file_path);
        if (exit_key == F2) {
            strcpy(inpbuf, "RUN\n");
            tokenise(true);
            ExecuteProgram(tknbuf);
        }
    }
}
