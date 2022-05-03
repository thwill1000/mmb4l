/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_system.c

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
#include <stdio.h>
#include <stdlib.h>

#include "../common/mmb4l.h"
#include "../common/cstring.h"
#include "../common/error.h"
#include "../common/parse.h"
#include "../common/utility.h"

/**
 * @brief  Reads value of an environment variable into a buffer.
 *
 * The value read into the buffer does not include a trailing '\0'.
 *
 * @param[in] name     Name of the environment variable.
 * @param[out] buf     Buffer to read value into.
 * @param[in, out] sz  On entry the size of the buffer.
 *                     On exit the number of characters read into the buffer.
 */
static MmResult cmd_system_getenv_to_buf(const char *name, char *buf, size_t *sz) {
    char *value = getenv(name);
    if (!value) value = "";
    size_t len = strlen(value);
    if (len > *sz) return kStringTooLong;
    memcpy(buf, value, len);
    *sz = len;
    return kOk;
}

/**
 * @brief  Implements SYSTEM GETENV sub-command.
 *
 * SYSTEM GETENV name$, value$
 * SYSTEM GETENV name$, value%()
 */
static void cmd_system_getenv(char *p) {
    getargs(&p, 3, ",");
    if (argc != 3) ERROR_SYNTAX;

    // Name of environment variable to query.
    char *name = getCstring(p);

    // Determine where to capture the value.
    char *value_var_ptr = findvar(argv[2], V_FIND | V_EMPTY_OK);
    char *buf = NULL;
    size_t buf_sz = 0;
    if ((vartbl[VarIndex].type & T_STR)
            &&  (vartbl[VarIndex].dims[0] == 0)) {
        // Capture value in STRING variable.
        buf = value_var_ptr + 1;
        buf_sz = STRINGSIZE - 1;
    } else if ((vartbl[VarIndex].type & T_INT)
            && (vartbl[VarIndex].dims[0] > 0)
            && (vartbl[VarIndex].dims[1] == 0)) {
        // Capture value in LONGSTRING variable.
        buf = (char *) value_var_ptr + 8;
        buf_sz = (vartbl[VarIndex].dims[0] - mmb_options.base) * 8;
    } else {
        ERROR_INVALID("2nd argument; expected STRING or LONGSTRING");
    }

    MmResult result = cmd_system_getenv_to_buf(name, buf, &buf_sz);

    switch (result) {
        case kOk:
            if (buf == value_var_ptr + 1) {
                // Set size of STRING variable.
                *value_var_ptr = buf_sz;
            } else {
                // Set size of LONGSTRING variable.
                *((int64_t *) value_var_ptr) = buf_sz;
            }
            break;
        case kStringTooLong:
            ERROR_ENV_VAR_TOO_LONG;
            break;
        default:
            error_throw(result);
            break;
    }
}

/**
 * @brief  Implements SYSTEM SETENV sub-command.
 *
 * SYSTEM SETENV name$, value$
 * SYSTEM SETENV name$, value%()
 */
static void cmd_system_setenv(char *p) {
    getargs(&p, 3, ",");
    if (argc != 3) ERROR_SYNTAX;

    char *name = getCstring(p);
    char *value = NULL;
    MmResult result = kSyntax;

    // Is the value a LONGSTRING ?
    if (parse_matches_longstring_pattern(argv[2])) {
        char *var_ptr = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT
                && (vartbl[VarIndex].dims[0] > 0)
                && (vartbl[VarIndex].dims[1] == 0)) {
            int64_t sz = *((int64_t *) var_ptr);
            value = GetTempMemory(sz + 1);
            memcpy(value, var_ptr + 8, sz);
            value[sz] = 0;
            result = setenv(name, value, 1);
            ClearSpecificTempMemory(value);
        }
    }

    if (result != kOk) {
        value = getCstring(argv[2]);
        result = setenv(name, value, 1);
    }

    if (result != kOk) error_throw(result);
}

/**
 * @brief  Executes a system command potentially capturing its STDOUT in a buffer.
 *
 * @param[in]       cmd          System command to execute.
 * @param[out]      buf          Buffer to capture output in.
 *                               Note that this will not be '\0' terminated.
 *                               If NULL then output is not captured but instead printed on STDOUT.
 * @param[in, out]  sz           On entry the size of buffer.
 *                               On exit the number of characters in the buffer.
 * @param[out]      exit_status  On exit the exit status of the executed system command.
 */
static MmResult cmd_system_to_buf(char *cmd, char *buf, size_t *sz, int64_t *exit_status) {

    if (!buf) {
        // Special handling when we are not capturing the output.
        *exit_status = system(cmd);
    } else {
        FILE *f = popen(cmd, "r");
        if (!f) return errno;

        int i;
        for (i = 0; i < *sz; ++i) {
            int ch = fgetc(f);
            if (ch == EOF) break;
            buf[i] = (char) ch;
        }

        // Trim any trailing CRLF.
        i--;
        if (i > -1 && buf[i] == '\n') i--;
        if (i > -1 && buf[i] == '\r') i--;
        *sz = i + 1;

        *exit_status = pclose(f);
    }

    if (*exit_status == -1) {
        return errno;
    } else {
        // TODO: WEXITSTATUS() is only valid if the child-process terminated normally.
        *exit_status = WEXITSTATUS(*exit_status);
        return kOk;
    }
}

/**
 * @brief  Implements base SYSTEM command.
 *
 * SYSTEM command$ [, output$   [, exit_code%]]
 * SYSTEM command$ [, output%() [, exit_code%]]
 */
static void cmd_system_execute(char *p) {
    getargs(&p, 5, ",");
    if (argc != 1 && argc != 3 && argc != 5) ERROR_SYNTAX;

    // System command to run.
    char *command = getCstring(argv[0]);

    // Redirect STDERR to STDOUT so it can be captured.
    // TODO: What if the command already contains redirection ?
    if (FAILED(cstring_cat(command, " 2>&1", STRINGSIZE))) ERROR_STRING_TOO_LONG;

    // Determine where to store the output.
    char *output_var_ptr = NULL;
    char *buf = NULL;
    size_t buf_sz = 0;
    if (argc >= 3 && strlen(argv[2]) > 0) {
        output_var_ptr = findvar(argv[2], V_FIND | V_EMPTY_OK);

        if ((vartbl[VarIndex].type & T_STR)
                && (vartbl[VarIndex].dims[0] == 0)) {
            // Capture output in STRING variable.
            buf = output_var_ptr + 1;
            buf_sz = STRINGSIZE - 1;
        } else if ((vartbl[VarIndex].type & T_INT)
                && (vartbl[VarIndex].dims[0] > 0)
                && (vartbl[VarIndex].dims[1] == 0)) {
            // Capture output in LONGSTRING variable.
            buf = (char *) output_var_ptr + 8;
            buf_sz = (vartbl[VarIndex].dims[0] - mmb_options.base) * 8;
        } else {
            ERROR_INVALID("2nd argument; expected STRING or LONGSTRING");
        }
    }

    // Determine where to store the exit status.
    int64_t exit_status = 0;
    int64_t *exit_status_ptr = &exit_status;
    if (argc == 5) {
        char *exit_status_var_ptr = findvar(argv[4], V_FIND);
        if (!((vartbl[VarIndex].type & T_INT) && (vartbl[VarIndex].dims[0] == 0))) ERROR_SYNTAX;
        exit_status_ptr = (int64_t *) exit_status_var_ptr;
    }

    MmResult result = cmd_system_to_buf(command, buf, &buf_sz, exit_status_ptr);

    if (result == kOk) {
        if (!buf) {
            // We didn't capture the output.
        } else if (buf == output_var_ptr + 1) {
            // Set size of STRING variable.
            *output_var_ptr = buf_sz;
        } else {
            // Set size of LONGSTRING variable.
            *((int64_t *) output_var_ptr) = buf_sz;
        }
        if (*exit_status_ptr == 127) error_throw(kUnknownSystemCommand);
    } else {
        error_throw(result);
    }
}

void cmd_system(void) {
    char* p;

    if ((p = checkstring(cmdline, "GETENV"))) {
        cmd_system_getenv(p);
    } else if ((p = checkstring(cmdline, "SETENV"))) {
        cmd_system_setenv(p);
    } else {
        cmd_system_execute(cmdline);
    }
}
