/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_list.c

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../common/cstring.h"
#include "../common/display.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/iodevice.h"
#include "../common/keycodes.h"
#include "../common/mmb4l.h"
#include "../common/mmgetchar.h"
#include "../common/parse.h"
#include "../common/program.h"
#include "../common/utility.h"
#include "../core/Commands.h"
#include "../core/tokentbl.h"

#define ERROR_NOTHING_TO_LIST  error_throw_ex(kError, "Nothing to list")

void cmd_files_internal(const char *);      // cmd_files.c
MmResult cmd_graphics_list(const char *p);  // cmd_graphics.c
void cmd_option_list(const char *);         // cmd_option.c

static void ListProgram(const char *p, int all) {
    char b[STRINGSIZE];
    char *pp;
    int ListCnt = 1;

    while(!(*p == 0 || *p == 0xff)) {                               // normally a LIST ends at the break so this is a safety precaution
        if(*p == T_NEWLINE) {
            p = llist(b, p);                                        // otherwise expand the line
            pp = b;
            while(*pp) {
                if(MMCharPos >= mmb_options.width) ListNewLine(&ListCnt, all);
                (void) display_putc(*pp++);
            }
            ListNewLine(&ListCnt, all);
            if(p[0] == 0 && p[1] == 0) break;                       // end of the listing ?
        }
    }
}

/* qsort C-string comparison function */
static int cstring_cmp(const void *a, const void *b)  {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcasecmp(*ia, *ib);
}

static MmResult cmd_list_tokens(const char *title, const struct s_tokentbl *primary,
                                const char **secondary) {
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

    char **tbl = (char **) GetTempMemory(total * sizeof(char *));
    char *storage = (char *) GetTempMemory(total * 20);

    // Initialise char pointers.
    for (int i = 0; i < total; ++i) {
        tbl[i] = storage + i * 20;
    }

    // Copy primary items.
    ptok = (struct s_tokentbl *) primary;
    for (int i = 0; ptok->name[0] != '\0' && ptok->fptr != cmd_dummy; ++i, ++ptok) {
        strcpy(tbl[i], ptok->name);
    }

    // Copy secondary items.
    char buf[STRINGSIZE];
    for (int i = num_primary; i < total; ++i) {
        sprintf(buf, "%s (*)", secondary[i - num_primary]);
        strcpy(tbl[i], buf);
    }

    // Sort the table.
    qsort(tbl, total, sizeof(char *), cstring_cmp);

    int step = mmb_options.width / 20;
    for (int i = 0; i < total; i += step) {
        for (int k = 0; k < step; k++) {
            if (i + k < total) {
                display_puts(tbl[i + k]);
                if (k != (step - 1))
                    for (int j = strlen(tbl[i + k]); j < 19; j++) display_puts(" ");
            }
        }
        display_puts("\r\n");
    }
    sprintf(buf, "Total of %d %s using %d slots\r\n\r\n", total, title, num_primary);
    display_puts(buf);

    return kOk;
}

/** LIST COMMANDS */
static MmResult cmd_list_commands(const char *p) {
    if (!parse_is_end(p)) return kUnexpectedText;
    const char *secondary_commands[] = {
            // "foo",
            // "bar",
            (char *) NULL };
    return cmd_list_tokens("commands", commandtbl, secondary_commands);
}

/** LIST {CSUB|CSUBS} [ALL] */
static MmResult cmd_list_csubs(const char *p) {
    const char *p2 = checkstring(p, "ALL");
    const bool all = p2;
    p2 = p2 ? p2 : p;
    if (!parse_is_end(p2)) return kUnexpectedText;
    if (!CurrentFile[0]) return mmresult_ex(kError, "Nothing to list");

    // Make sure we are looking at the latest (on disk) version of the program.
    ON_FAILURE_RETURN(program_load_file(CurrentFile));

    program_list_csubs(all);

    return kOk;
}

/** LIST FLASH [ALL] */
static MmResult cmd_list_flash(const char *p) {
    const char *p2 = checkstring(p, "ALL");
    const bool all = p2;
    p2 = p2 ? p2 : p;
    if (!parse_is_end(p2)) return kUnexpectedText;
    if (!CurrentFile[0]) return mmresult_ex(kError, "Nothing to list");

    // Make sure we are looking at the latest (on disk) version of the program.
    ON_FAILURE_RETURN(program_load_file(CurrentFile));

    ListProgram(ProgMemory, all);
    display_puts("\r\n");

    return kOk;
}

/** LIST FUNCTIONS */
static MmResult cmd_list_functions(const char *p) {
    if (!parse_is_end(p)) return kUnexpectedText;
    const char *secondary_functions[] = {
            // "foo",
            // "bar",
            (char *) NULL };
    return cmd_list_tokens("functions", tokentbl, secondary_functions);
}

/** LIST VARIABLES [ALL|GLOBAL|LOCAL|level%] */
static MmResult cmd_list_variables(const char *p) {
    getargs(&p, 1, DELIM_COMMA);
    int level = -1; // ALL
    if (argc == 1) {
        if (checkstring(argv[0], "ALL")) {
            // Do nothing, level = -1 is correct.
        } else if (checkstring(argv[0], "GLOBAL")) {
            level = 0;
        } else if (checkstring(argv[0], "LOCAL")) {
            level = LocalIndex;
        } else {
            level = getint(p, 0, 1000);
        }
    }

    char name[MAXVARLEN + 2];
    char type[15];
    char dimensions[STRINGSIZE];
    char latest[MAXVARLEN + 2] = "";
    int idx = -1;
    int count = 0;

    display_puts("+------------------------------------------------------------------------------+\r\n");
    display_puts("| Name                              | Type          | Level | Dimensions       |\r\n");
    display_puts("| --------------------------------- | ------------- | ----- | ---------------- |\r\n");
    for (;;) {
        // Determine next variable in alphabetical order.
        memset(name, 255, MAXVARLEN + 2);
        idx = -1;
        for (int i = 0; i < MAXVARS; ++i) {
            const struct s_vartbl *var = &vartbl[i];
            if (!var->type) continue;
            if (level != -1 && level != var->level) continue;
            if (memcmp(name, var->name, MAXVARLEN) > 0
                    && memcmp(latest, var->name, MAXVARLEN) < 0) {
                memset(name, 0, MAXVARLEN + 2);
                memcpy(name, var->name, MAXVARLEN);
                idx = i;
            }
        }

        if (idx == -1) break; // Reached the end of the variables.

        strcpy(latest, name);

        const struct s_vartbl *var = &vartbl[idx];

        // Add type extension to name.
        if (var->type & T_IMPLIED) {
            cstring_cat(name, "*", MAXVARLEN + 2);
        } else {
            if (var->type & T_INT) cstring_cat(name, "\%", MAXVARLEN + 2);
            if (var->type & T_STR) cstring_cat(name, "$", MAXVARLEN + 2);
            if (var->type & T_NBR) cstring_cat(name, "!", MAXVARLEN + 2);
        }

        // Type.
        type[0] = '\0';
        if (var->type & T_CONST) cstring_cat(type, "CONST ", sizeof(type));
        if (var->type & T_PTR) cstring_cat(type, "PTR ", sizeof(type));
        if (var->type & T_INT) cstring_cat(type, "INT ", sizeof(type));
        if (var->type & T_STR) {
            cstring_cat(type, "STR ", sizeof(type));
            cstring_cat_int64(type, var->size, sizeof(type));
            cstring_cat(type, " ", sizeof(type));
        }
        if (var->type & T_NBR) cstring_cat(type, "NBR ", sizeof(type));
        type[strlen(type) - 1] = '\0'; // Remove trailing space.

        // Dimensions.
        dimensions[0] = '\0';
        if (var->dims[0] == 0) {
            cstring_cat(dimensions, "-", STRINGSIZE);
        } else {
            for (int j = 0; j < MAXDIM; ++j) {
                if (var->dims[j] == 0) break;
                if (j != 0) cstring_cat(dimensions, ",", STRINGSIZE);
                cstring_cat_int64(dimensions, var->dims[j], STRINGSIZE);
            }
        }

        sprintf(inpbuf, "| %-33s | %-13s | %-5d | %-16s | \r\n", name, type, var->level,
                dimensions);
        display_puts(inpbuf);
        count++;
    }
    if (count == 0) {
        sprintf(inpbuf, "| %-33s | %-13s | %-5d | %-16s | \r\n", "No variables declared", "", 0, "");
        display_puts(inpbuf);
    }
    display_puts("+------------------------------------------------------------------------------+\r\n");

    return kOk;
}

/** LIST [ALL] file$ */
static MmResult cmd_list_default(const char *p) {
    const char *p2 = checkstring(p, "ALL");
    const bool all = p2;
    p2 = p2 ? p2 : p;
    char *filename = GetTempStrMemory();
    if (parse_is_end(p2)) {
        if (!CurrentFile[0]) {
            return mmresult_ex(kError, "Nothing to list");
        } else if (FAILED(cstring_cpy(filename, CurrentFile, STRINGSIZE))) {
            return kFilenameTooLong;
        }
    } else {
        ON_FAILURE_RETURN(parse_filename(p2, filename, STRINGSIZE));
    }

    char line_buffer[STRINGSIZE];
    int list_count = 1;
    int fnbr = file_find_free();
    ON_FAILURE_RETURN(iodevice_open(filename, "rb", fnbr));
    while (!file_eof(fnbr)) {
        memset(line_buffer, 0, STRINGSIZE);
        MMgetline(fnbr, line_buffer);
        for (size_t i = 0; i < strlen(line_buffer); i++) {
            if (line_buffer[i] == TAB) line_buffer[i] = ' ';
        }
        display_puts(line_buffer);
        list_count += strlen(line_buffer) / mmb_options.width;
        ListNewLine(&list_count, all);
    }

    // Ensure listing is followed by an empty line.
    if (strcmp(line_buffer, "") != 0) display_puts("\r\n");

    return file_close(fnbr);
}

void cmd_list(void) {
    const char *p;
    skipspace(cmdline);

    // Use the current display dimensions for the output of the LIST command.
    ON_FAILURE_ERROR(display_get_size(false, &mmb_options.width, &mmb_options.height));

    MmResult result = kOk;
    if ((p = checkstring(cmdline, "COMMANDS"))) {
        result = cmd_list_commands(p);
    } else if ((p = checkstring(cmdline, "CSUB"))) {
        result = cmd_list_csubs(p);
    } else if ((p = checkstring(cmdline, "CSUBS"))) {
        result = cmd_list_csubs(p);
    } else if ((p = checkstring(cmdline, "FILES"))) {
        // LIST FILES
        cmd_files_internal(p);
    } else if ((p = checkstring(cmdline, "FLASH"))) {
        result = cmd_list_flash(p);
    } else if ((p = checkstring(cmdline, "FUNCTIONS"))) {
        result = cmd_list_functions(p);
    } else if ((p = checkstring(cmdline, "GRAPHICS"))) {
        // LIST GRAPHICS
        result = cmd_graphics_list(p);
    } else if ((p = checkstring(cmdline, "OPTIONS"))) {
        // LIST OPTIONS
        cmd_option_list(p);
    } else if ((p = checkstring(cmdline, "VARIABLES"))) {
        cmd_list_variables(p);
    } else if ((p = checkstring(cmdline, "VARS"))) {
        result = cmd_list_variables(p);
    } else {
        result = cmd_list_default(cmdline);
    }
    ON_FAILURE_ERROR(result);
}
