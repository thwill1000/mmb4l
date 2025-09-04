/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_run.c

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

#include <string.h>

#include "../common/mmb4l.h"
#include "../common/cstring.h"
#include "../common/flash.h"
#include "../common/logger.h"
#include "../common/program.h"
#include "../common/utility.h"
#include "../core/tokentbl.h"

char cmd_run_args[STRINGSIZE];

/**
 * Heuristically determines whether the "legacy" (non-string expression) format
 * is being used for RUN arguments.
 *
 * This and the 'cmd_run_transform_legacy_args' functions are probably only
 * required short-term on the CMM2 and MMB4W to support old versions of
 * "The Welcome Tape" and tools by @thwill.
 *
 * @return true   if the 'cmd_args' contain the MMBasic subtract token
 *                or if the 'filename' contains "menu/menu.bas" AND the
 *                'cmd_args' start with "MENU_".
 *         false  otherwise.
 */
static bool cmd_run_is_legacy_args(const char *filename, const char *run_args) {
    // Seach for subtract token.
    for (const char *p = run_args; *p; ) {
        if (tokentbl_read(&p) == tokenSUBTRACT) return true;
    }

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
    for (const char *p = run_args; *p; ) {
        const FunctionToken funtok = tokentbl_read(&p);
        if (funtok >= C_BASETOKEN) {
            // Convert tokens back to literals and try to do sensible things
            // regarding spaces.
            if (ptmp != tmp && *(ptmp - 1) != ' ') {
                if (funtok == tokenSUBTRACT) {
                    if (*(ptmp - 1) != '-' && !isalnum(*(ptmp - 1))) *ptmp++ = ' ';
                } else if (funtok == tokenEQUAL) {
                    if (!isalnum(*(ptmp - 1))) *ptmp++ = ' ';
                } else {
                    *ptmp++ = ' ';
                }
            }
            const char *fnname = tokenname(funtok);
            memcpy(ptmp, fnname, strlen(fnname));
            ptmp += strlen(fnname);
        } else if (funtok == '"') {
            // Do not mangle quoted sections.
            *ptmp++ = (char) funtok;
            for (; *p; ++p) {
                *ptmp++ = *p;
                if (*p == '"') break;
            }
            p++;
        } else if (funtok == ' ' && ptmp != tmp) {
            // Compress consecutive spaces.
            if (*(ptmp - 1) != ' ') *ptmp++ = ' ';
        } else {
            // Though the current MMB4L tokeniser preserves case, that in
            // MMB4W and other MMBasic ports by Peter will have converted
            // unquoted legacy arguments to upper-case. On the balance of
            // probabilities we convert them to lower-case here.
            *ptmp++ = tolower((char) funtok);
        }

        if (ptmp - tmp >= STRINGSIZE - 1) break;
    }
    *ptmp = '\0';
    cstring_cpy(run_args, tmp, STRINGSIZE);
    ClearSpecificTempMemory(tmp);
}

/**
 * Parses filename and RUN arguments from a token buffer.
 *
 * @param[in]   p            pointer to the buffer.
 * @param[out]  simulate     on exit the device/platform to simulate.
 * @param[out]  filename     buffer to hold the filename, should be at least STRINGSIZE.
 * @param[in,out]  run_args  buffer to hold the RUN args, should be at least STRINGSIZE.
 *                             on entry: the RUN args that were passed to the current program.
 *                             on exit:  the RUN args to pass to the new program.
 * @return                   kOk on success.
 */
MmResult cmd_run_parse_args(const char *p, OptionsSimulate *simulate, char *filename,
                            char *run_args) {
    *simulate = kSimulateMmb4l;
    *filename = '\0';

    // WARNING! do not clear 'run_args' at the start of this function,
    // its existing value may need to be evaluated to calculate its new value.

    const DelimType delim[] = { ',', tokenAS, 0 };
    getargs(&p, 5, delim);
    int filename_idx = -1;  // Index into argv[] for filename.
    int run_args_idx = -1;  // Index into argv[] for additional arguments.

    // Check for trailing "AS <simulate option>".
    if (argc >= 2 && tokentbl_peek(argv[argc - 2]) == tokenAS) {
        // First check for the <device> as a "keyword".
        int match = options_simulate_from_string(argv[argc - 1]);

        // If unmatched then check for the <device> as a string.
        if (match == -1) {
            const char *s = getCstring(argv[argc - 1]);
            match = options_simulate_from_string(s);
        }

        if (match == -1) ON_FAILURE_RETURN(kUnknownDevice);

        *simulate = (OptionsSimulate) match;
        argc -= 2;
    }

    // Note for legacy compatibility we need to allow the trailing comma.
    if (argc == 0) {
        // RUN
        // Do nothing.
    } else if (argc == 1 && *(argv[0]) == ',') {
        // RUN ,
        // Don't set filename, and clear cmd_run_args.
    } else if (argc == 1) {
        // RUN file$
        if (*(argv[0]) != ',') filename_idx = 0;
    } else if (argc == 2 && *(argv[0]) == ',') {
        // RUN , args$
        run_args_idx = 1;
    } else if (argc == 2 && (*argv[1]) == ',') {
        // RUN file$ ,
        filename_idx = 0;
    } else if (argc == 3 && (*argv[1]) == ',') {
        // RUN file$ , args$
        filename_idx = 0;
        run_args_idx = 2;
    } else {
        return kSyntax;
    }

    if (filename_idx >= 0) strcpy(filename, getCstring(argv[0]));

    if (run_args_idx >= 0) {
        if (cmd_run_is_legacy_args(filename, argv[run_args_idx])) {
            strcpy(run_args, argv[run_args_idx]);
            cmd_run_transform_legacy_args(run_args);
        } else {
            strcpy(run_args, getCstring(argv[run_args_idx]));
        }
    } else {
        *run_args = '\0';
    }

    return kOk;
}

void cmd_run(void) {
    LOG_FN_ENTRY("cmdline=%s", cmdline);

#if defined(__ANDROID__)
    OptionsSimulate simulate = kSimulatePicocalc;
#else
    OptionsSimulate simulate = kSimulateMmb4l;
#endif
    char filename[STRINGSIZE];  // Filename to RUN.

    ON_FAILURE_ERROR(cmd_run_parse_args(cmdline, &simulate, filename, cmd_run_args));

    if (!*filename) {
        if (*CurrentFile != '\0') {
            strcpy(filename, CurrentFile);
        } else {
            ON_FAILURE_ERROR(mmresult_ex(kError, "Nothing to run"));
        }
    }

    ON_FAILURE_ERROR(program_load_file(filename));

    ClearRuntime();

    simulate = kSimulatePicocalc;
    if (simulate != mmb_options.simulate) {
        mmb_options.simulate = simulate;
        // TODO: Eliminate duplication with cmd_option().
        ON_FAILURE_ERROR(features_init(&mmb_features, simulate));
        ON_FAILURE_ERROR(graphics_set_mode(1, 32, RGB_BLACK));
        ON_FAILURE_ERROR(mmb_features.has_cmd_flash ? flash_init() : flash_term());
    }

    WatchdogSet = false;
    PrepareProgram(true);
    IgnorePIN = false;
    if (*ProgMemory != T_NEWLINE) return;  // no program to run
    nextstmt = ProgMemory;

    LOG_FN_EXIT();
}
