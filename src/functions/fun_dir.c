/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_dir.c

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
#include "../common/path.h"
#include "../common/utility.h"

#include <dirent.h>
#include <libgen.h>
#include <string.h>

#define ERROR_INVALID_FLAG_SPECIFICATION  error_throw_ex(kError, "Invalid flag specification")

int32_t dirflags;

static char get_achar(/* Get a character and advances ptr 1 or 2 */
                      const char **ptr /* Pointer to pointer to the
                                          SBCS/DBCS/Unicode string */
) {
    char chr;

    chr = (char)*(*ptr)++;                         /* Get a byte */
    if ((chr >= 'a') && (chr <= 'z')) chr -= 0x20; /* To upper ASCII char */
    return chr;
}

static int
pattern_matching(                 /* 0:not matched, 1:matched */
                 const char *pat, /* Matching pattern */
                 const char *nam, /* String to be tested */
                 int skip,        /* Number of pre-skip chars (number of ?s) */
                 int inf          /* Infinite search (* specified) */
) {
    const char *pp, *np;
    char pc, nc;
    int nm, nx;

    while (skip--) { /* Pre-skip name chars */
        if (!get_achar(&nam))
            return 0; /* Branch mismatched if less name chars */
    }
    if (!*pat && inf) return 1; /* (short circuit) */

    do {
        pp = pat;
        np = nam; /* Top of pattern and name to match */
        for (;;) {
            if (*pp == '?' || *pp == '*') { /* Wildcard? */
                nm = nx = 0;
                do { /* Analyze the wildcard chars */
                    if (*pp++ == '?')
                        nm++;
                    else
                        nx = 1;
                } while (*pp == '?' || *pp == '*');
                if (pattern_matching(pp, np, nm, nx))
                    return 1; /* Test new branch (recurs upto number of wildcard
                                 blocks in the pattern) */
                nc = *np;
                break; /* Branch mismatched */
            }
            pc = get_achar(&pp); /* Get a pattern char */
            nc = get_achar(&np); /* Get a name char */
            if (pc != nc) break; /* Branch mismatched? */
            if (pc == 0)
                return 1; /* Branch matched? (matched at end of both strings) */
        }
        get_achar(&nam); /* nam++ */
    } while (inf &&
             nc); /* Retry until end of name if infinite search is specified */

    return 0;
}

void fun_dir(void) {
    static DIR *dp = NULL;
    static char pp[32];
    getargs(&ep, 3, ",");
    g_rtn_type = T_STR;

    switch (argc) {
        case 0:
            if (!dp) {
                g_string_rtn = GetTempStrMemory();
                *g_string_rtn = '\0';
                CtoM(g_string_rtn);
                return;
            }
            break;

        case 1:
            dirflags = DT_REG;
            break;

        case 3:
            if (checkstring(argv[2], "DIR")) {
                dirflags = DT_DIR;
            } else if (checkstring(argv[2], "FILE")) {
                dirflags = DT_REG;
            } else if (checkstring(argv[2], "ALL")) {
                dirflags = 0;
            } else {
                ERROR_INVALID_FLAG_SPECIFICATION;
                return;
            }
            break;

        default:
            ERROR_SYNTAX;
            return;
    }

    if (argc != 0) {
        // This must be the first call eg:  DIR$("*.*", FILE)

        char *path = GetTempStrMemory();
        ON_FAILURE_ERROR(parse_filename(argv[0], path, STRINGSIZE));

        strcpy(pp, basename(path));
        dp = opendir(dirname(path));
        if (!dp) ON_FAILURE_ERROR(errno);
    }

    struct dirent *entry;
    while ((entry = readdir(dp))) {
        if (strcmp(entry->d_name, ".") != 0
                && strcmp(entry->d_name, "..") != 0
                && pattern_matching(pp, entry->d_name, 0, 0)
                && (!dirflags || entry->d_type == dirflags)) break;
    }

    g_string_rtn = GetTempStrMemory();

    if (entry) {
        strcpy(sret, entry->d_name);
    } else {
        closedir(dp);
        g_string_rtn[0] = 0;
        dp = NULL;
    }

    CtoM(g_string_rtn);
}
