/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_open.c

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

#include <strings.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"
#include "../common/serial.h"

static void cmd_open_file(int argc, char **argv) {
    const char *filename = getCstring(argv[0]);

    const char *mode = NULL;
    if (str_equal(argv[2], "OUTPUT"))
        mode = "wb";  // binary mode so that we do not have lf to cr/lf
                      // translation
    else if (str_equal(argv[2], "APPEND"))
        mode = "ab";  // binary mode is used in MMfopen()
    else if (str_equal(argv[2], "INPUT"))
        mode = "rb";  // note binary mode
    else if (str_equal(argv[2], "RANDOM"))
        mode = "x";  // a special mode for MMfopen()
    else
        ERROR_INVALID("file access mode");

    int fnbr = parse_file_number(argv[4], false);
    if (fnbr == -1) ERROR_INVALID_FILE_NUMBER;

    file_open(filename, mode, fnbr);
}

static void cmd_open_gps(int argc, char **argv) {
    ERROR_UNIMPLEMENTED("OPEN comspec AS GPS");
}

static void cmd_open_serial(int argc, char **argv) {
    char *comspec = getCstring(argv[0]);

    int fnbr = parse_file_number(argv[2], false);
    if (fnbr == -1) ERROR_INVALID_FILE_NUMBER;

    serial_open(comspec, fnbr);
}

/**
 * OPEN fname$ FOR mode AS [#]fnbr
 * OPEN comspec$ AS [#]fnbr
 * OPEN comspec$ AS GPS [,timezone_offset] [,monitor]
 */
void cmd_open(void) {
    char separators[4] = { tokenFOR, tokenAS, ',', '\0' };
    getargs(&cmdline, 7, separators);

    if (argc == 5 && *argv[1] == tokenFOR && *argv[3] == tokenAS) {
        cmd_open_file(argc, argv);
    } else if (argc > 2 && argc < 8 && *argv[1] == tokenAS && strcasecmp(argv[2], "GPS") == 0) {
        cmd_open_gps(argc, argv);
    } else if (argc == 3 && *argv[1] == tokenAS) {
        cmd_open_serial(argc, argv);
    } else {
        ERROR_SYNTAX;
    }
}
