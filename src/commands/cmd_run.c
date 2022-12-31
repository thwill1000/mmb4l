/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_run.c

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

#include "../common/mmb4l.h"
#include "../common/program.h"
#include "../common/utility.h"

#include <string.h>

#define ERROR_NOTHING_TO_RUN  error_throw_ex(kError, "Nothing to run")

char cmd_run_args[STRINGSIZE];

/**
 * Heuristically determines whether the "legacy" (non-string expression) format
 * is being used for RUN arguments.
 *
 * This and the 'cmd_run_transform_legacy_args' functions are probably only
 * required short-term on the CMM2 and MMB4W to support old versions of
 * "The Welcome Tape" and tools by @thwill.
 *
 * @return true   if the 'cmd_args' contain the MMBasic minus-sign token
 *                or if the 'filename' contains "menu/menu.bas" AND the
 *                'cmd_args' start with "MENU_".
 *         false  otherwise.
 */
static bool cmd_run_is_legacy_args(const char *filename, const char *run_args) {
    if (strchr(run_args, GetTokenValue("-"))) return true;
    if (filename
            && strstr(filename, "menu/menu.bas")
            && strstr(run_args, "MENU_") == run_args) return true;
    return false;
}

/**
 * Makes a best effort to restore tokenised RUN arguments to the
 * "legacy" (non-string expression) format
 *
 *  - tokens are converted back to literals,
 *  - unquoted strings are converted to lower-case,
 *  - whitespace may not be restored exactly the same.
 *
 * Probably it is "overkill".
 */
static void cmd_run_transform_legacy_args(char *run_args) {
    char *tmp = (char *) GetTempMemory(STRINGSIZE + 32); // Extra space to avoid string overrun.
    char *ptmp = tmp;
    for (char *p = run_args; *p; ++p) {
        char *tok = (char *) tokenname((unsigned char) *p);
        if (*tok) {
            // Convert tokens backs to literals and try to do sensible things
            // regarding spaces.
            if (ptmp != tmp && *(ptmp - 1) != ' ') {
                switch (*tok) {
                    case '-':
                        if (*(ptmp - 1) != '-' && !isalnum(*(ptmp - 1))) *ptmp++ = ' ';
                        break;
                    case '=':
                        if (!isalnum(*(ptmp - 1))) *ptmp++ = ' ';
                        break;
                    default:
                        *ptmp++ = ' ';
                        break;
                }
            }
            memcpy(ptmp, tok, strlen(tok));
            ptmp += strlen(tok);
        } else if (*p == '"') {
            // Do not mangle quoted sections.
            *ptmp++ = *p++;
            for (; *p; ++p) {
                *ptmp++ = *p;
                if (*p == '"') break;
            }
        } else {
            if (*p == ' ' && ptmp != tmp) {
                // Compress consecutive spaces.
                if (*(ptmp - 1) != ' ') *ptmp++ = ' ';
            } else {
                // Though the current MMB4L tokeniser preserves case, that in
                // MMB4W and other MMBasic ports by Peter will have converted
                // unquoted legacy arguments to upper-case. On the balance of
                // probabilities we convert them to lower-case here.
                *ptmp++ = tolower(*p);
            }
        }

        if (ptmp - tmp >= STRINGSIZE - 1) break;
    }
    *ptmp = '\0';
    strncpy(run_args, tmp, STRINGSIZE - 1);
    run_args[STRINGSIZE - 1] = '\0';
    ClearSpecificTempMemory(tmp);
}

/**
 * Parses filename and RUN arguments from a token buffer.
 *
 * @param[in]   p         pointer to the buffer.
 * @param[out]  filename  buffer to hold the filename, should be at least STRINGSIZE.
 * @param[out]  run_args  buffer to hold the RUN args, should be at least STRINGSIZE.
 * @return                kOk on success.
 */
MmResult cmd_run_parse_args(const char *p, char *filename, char *run_args) {
    *filename = '\0';
    *run_args = '\0';
    if (!*p) return kOk;

    getargs(&p, 3, ",");
    int filename_idx = -1;  // Index into argv[] for filename.
    int run_args_idx = -1;  // Index into argv[] for additional arguments.

    // Note for legacy compatibility we need to allow the trailing comma.
    if (argc == 1 && *(argv[0]) == ',') {
        // RUN ,
        // Don't set filename or cmd_run_args.
    } else if (argc == 1) {
        // RUN file$
        if (*(argv[0]) != ',') filename_idx = 0;
    } else if (argc == 2 && *(argv[0]) == ',') {
        // RUN , args$
        run_args_idx = 1;
    } else if (argc == 2) {
        // RUN file$ ,
        filename_idx = 0;
    } else if (argc == 3) {
        // RUN file$ , args$
        filename_idx = 0;
        run_args_idx = 2;
    } else {
        return kInternalFault;
    }

    if (filename_idx >= 0) strcpy(filename, getCstring(argv[0]));

    if (run_args_idx >= 0) {
        if (cmd_run_is_legacy_args(filename, argv[run_args_idx])) {
            strcpy(run_args, argv[run_args_idx]);
            cmd_run_transform_legacy_args(run_args);
        } else {
            strcpy(run_args, getCstring(argv[run_args_idx]));
        }
    }

    return kOk;
}

void cmd_run(void) {
    char filename[STRINGSIZE];  // Filename to RUN.

    MmResult result = cmd_run_parse_args(cmdline, filename, cmd_run_args);
    if (FAILED(result)) {
        error_throw(result);
        return;
    }

    if (!*filename) {
        if (*CurrentFile != '\0') {
            strcpy(filename, CurrentFile);
        } else {
            ERROR_NOTHING_TO_RUN;
            return;
        }
    }

    if (FAILED(program_load_file(filename))) return;

    ClearRuntime();
    WatchdogSet = false;
    PrepareProgram(true);
    IgnorePIN = false;
    if (*ProgMemory != T_NEWLINE) return;  // no program to run
    nextstmt = ProgMemory;
}
