/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_log.c

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
   be displayed on the console at startup (additional copyright messages may
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

#include "../common/cstring.h"
#include "../common/error.h"
#include "../common/logger.h"
#include "../common/mmb4l.h"
#include "../common/parse.h"

static MmResult cmd_log_internal() {
    // Short-circuit if even FATAL level won't be logged.
    if (!logger_will_log(kLoggerLevelFatal)) return kOk;

    LoggerLevel log_level = kLoggerLevelInfo;

    // Check if there is a log level prefix.
    const char *p;
    if ((p = checkstring(cmdline, "DEBUG"))) {
        log_level = kLoggerLevelDebug;
    } else if ((p = checkstring(cmdline, "INFO"))) {
        log_level = kLoggerLevelInfo;
    } else if ((p = checkstring(cmdline, "WARNING"))) {
        log_level = kLoggerLevelWarning;
    } else if ((p = checkstring(cmdline, "ERROR"))) {
        log_level = kLoggerLevelError;
    } else if ((p = checkstring(cmdline, "FATAL"))) {
        log_level = kLoggerLevelFatal;
    } else {
        p = cmdline;
    }

    // Short-circuit if message won't be logged.
    if (!logger_will_log(log_level)) return kOk;

    char *msg = GetTempMemory(1024);

    while (*p && *p != '\'') {
        int t = T_NOTYPE;
        MMFLOAT f = 0.0;
        MMINTEGER i64 = 0;
        char *s = NULL;

        p = evaluate(p, &f, &i64, &s, &t, true);
        if (t & T_NBR) {
           *inpbuf = ' ';  // preload a space
           FloatToStr(inpbuf + ((f >= 0) ? 1:0), f, 0, STR_AUTO_PRECISION, ' ');  // if positive output a space instead of the sign
           s = inpbuf;
        } else if (t & T_INT) {
           *inpbuf = ' ';  // preload a space
           IntToStr(inpbuf + ((i64 >= 0) ? 1:0), i64, 10);  // if positive output a space instead of the sign
           s = inpbuf;
        } else if (t & T_STR) {
           s = MtoC(s);
        } else {
           return INTERNAL_FAULT_EX("invalid type: %d", t);
        }

        if (FAILED(cstring_cat(msg, s, STRINGSIZE))) return kStringTooLong;
    }

    int line = 0;
    char *file_path = GetTempMemory(PATH_MAX);  // Buffer could probably be smaller
    error_get_line_and_file(&line, file_path);
    char function_name[MAXVARLEN + 2] = { '\0' };
    ON_FAILURE_RETURN(get_current_function_name(function_name, sizeof(function_name)));
    logger_write(log_level, file_path, line < 0 ? 0 : line, function_name, "%s", msg);

    return kOk;
}

/** LOG [ DEBUG | INFO | WARNING | ERROR | FATAL ]? expression ... */
void cmd_log(void) {
    // LOG_FN_ENTRY("cmdline=\"%s\"", cmdline);
    ON_FAILURE_ERROR(cmd_log_internal());
    RETURN_VOID();
}
