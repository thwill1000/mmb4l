/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_list.c

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
#include "../common/console.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"
#include "../common/path.h"
#include "../common/program.h"
#include "../common/utility.h"
#include "../core/tokentbl.h"

#include <stdlib.h>
#include <string.h>

#define ERROR_NOTHING_TO_LIST  error_throw_ex(kError, "Nothing to list")

void cmd_files_internal(const char *);  // cmd_files.c
void cmd_option_list(const char *);     // cmd_option.c

/* qsort C-string comparison function */
static int cstring_cmp(const void *a, const void *b)  {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcasecmp(*ia, *ib);
}

static void list_tokens(const char *title, const struct s_tokentbl *primary, const char **secondary) {
    int num_primary = 0;
    struct s_tokentbl *ptok = (struct s_tokentbl *) primary;
    while (ptok->name[0] != '\0') {
        if (ptok->fptr != cmd_dummy) num_primary++;
        ptok++;
    }

    int num_secondary = 0;
    const char **ptr = secondary;
    while (*ptr++) num_secondary++;

    const int total = num_primary + num_secondary;

    char **tbl = (char **) GetTempMemory(
            total * sizeof(char *) // Memory for char pointers.
            + total * 20);         // Memory for 20 character strings.

    // Initialise char pointers.
    for (int i = 0; i < total; i++) {
        tbl[i] = (char *) (tbl + total + i * 20);
    }

    // Copy primary items.
    ptok = (struct s_tokentbl *) primary;
    for (int i = 0; ptok->name[0] != '\0'; ) {
        if (ptok->fptr != cmd_dummy) strcpy(tbl[i++], ptok->name);
        ptok++;
    }

    // Copy secondary items.
    char buf[STRINGSIZE];
    for (int i = num_primary; i < total; i++) {
        sprintf(buf, "%s (*)", secondary[i - num_primary]);
        strcpy(tbl[i], buf);
    }

    // Sort the table.
    qsort(tbl, total, sizeof(char *), cstring_cmp);

    int step = mmb_options.width / 20;
    for (int i = 0; i < total; i += step) {
        for (int k = 0; k < step; k++) {
            if (i + k < total) {
                console_puts(tbl[i + k]);
                if (k != (step - 1))
                    for (int j = strlen(tbl[i + k]); j < 19; j++) console_puts(" ");
            }
        }
        console_puts("\r\n");
    }
    sprintf(buf, "Total of %d %s using %d slots\r\n\r\n", total, title, num_primary);
    console_puts(buf);
}

static void list_commands() {
    const char *secondary_commands[] = {
            // "foo",
            // "bar",
            (char *) NULL };
    list_tokens("commands", commandtbl, secondary_commands);
}

static void list_functions() {
    const char *secondary_functions[] = {
            // "foo",
            // "bar",
            (char *) NULL };
    list_tokens("functions", tokentbl, secondary_functions);
}

static void list_file(const char *filename, int all) {
    if (!filename && CurrentFile[0] == '\0') {
        ERROR_NOTHING_TO_LIST;
        return;
    }

    char file_path[STRINGSIZE];
    MmResult result = path_munge(filename ? filename : CurrentFile, file_path, STRINGSIZE);
    if (FAILED(result)) error_throw(result);

    char line_buffer[STRINGSIZE];
    int list_count = 1;
    int fnbr = file_find_free();
    file_open(file_path, "rb", fnbr);
    while (!file_eof(fnbr)) {
        memset(line_buffer, 0, STRINGSIZE);
        MMgetline(fnbr, line_buffer);
        for (size_t i = 0; i < strlen(line_buffer); i++) {
            if (line_buffer[i] == TAB) line_buffer[i] = ' ';
        }
        console_puts(line_buffer);
        list_count += strlen(line_buffer) / mmb_options.width;
        ListNewLine(&list_count, all);
    }
    file_close(fnbr);

    // Ensure listing is followed by an empty line.
    if (strcmp(line_buffer, "") != 0) console_puts("\r\n");
}

static void list_flash(int all) {
    if (CurrentFile[0] == '\0') {
        ERROR_NOTHING_TO_LIST;
        return;
    }

    // Make sure we are looking at the latest (on disk) version of the program.
    if (FAILED(program_load_file(CurrentFile))) return;

    ListProgram(ProgMemory, all);

    console_puts("\r\n");
}

static void list_csubs(int all) {
    if (CurrentFile[0] == '\0') {
        ERROR_NOTHING_TO_LIST;
        return;
    }

    // Make sure we are looking at the latest (on disk) version of the program.
    if (FAILED(program_load_file(CurrentFile))) return;

    program_list_csubs(all);
}

static void list_options(const char *p) {
    cmd_option_list(p);
}

void cmd_list(void) {
    const char *p;
    skipspace(cmdline);

    // Use the current console dimensions for the output of the LIST command.
    if (FAILED(console_get_size(&mmb_options.width, &mmb_options.height, 0))) {
        ERROR_UNKNOWN_TERMINAL_SIZE;
    }

    if (parse_is_end(cmdline)) {
        list_file(NULL, false);
    } else if ((p = checkstring(cmdline, "COMMANDS"))) {
        if (!parse_is_end(p)) ERROR_SYNTAX;
        list_commands();
    } else if ((p = checkstring(cmdline, "CSUB")) || (p = checkstring(cmdline, "CSUBS"))) {
        if (parse_is_end(p)) {
            list_csubs(false);
        } else if ((p = checkstring(p, "ALL"))) {
            if (!parse_is_end(p)) ERROR_SYNTAX;
            list_csubs(true);
        } else {
            ERROR_SYNTAX;
        }
    } else if ((p = checkstring(cmdline, "FILES"))) {
        cmd_files_internal(p);
    } else if ((p = checkstring(cmdline, "FLASH"))) {
        if (parse_is_end(p)) {
            list_flash(false);
        } else if ((p = checkstring(p, "ALL"))) {
            if (!parse_is_end(p)) ERROR_SYNTAX;
            list_flash(true);
        } else {
            ERROR_SYNTAX;
        }
    } else if ((p = checkstring(cmdline, "FUNCTIONS"))) {
        if (!parse_is_end(p)) ERROR_SYNTAX;
        list_functions();
    } else if ((p = checkstring(cmdline, "OPTIONS"))) {
        list_options(p);
    } else {
        if ((p = checkstring(cmdline, "ALL"))) {
            if (parse_is_end(p)) {
                list_file(NULL, true);
            } else {
                list_file(getCstring(p), true);
            }
        } else {
            list_file(getCstring(cmdline), false);
        }
    }
}
