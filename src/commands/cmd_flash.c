/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_flash.c

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

#include "../common/error.h"
#include "../common/flash.h"
#include "../common/parse.h"
#include "../common/mmb4l.h"

/** FLASH DISK LOAD n, file$ [, O[VERWRITE]] */
MmResult cmd_flash_disk_load(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 3 && argc != 5) return kArgumentCount;

    unsigned flash_index = getint(argv[0], 1, FLASH_NUM_SLOTS) - 1;

    char *filename = GetTempStrMemory();
    ON_FAILURE_RETURN(parse_filename(argv[2], filename, STRINGSIZE));

    bool overwrite = false;
    if (argc == 5) {
        if (checkstring(argv[4], "O") || checkstring(argv[4], "OVERWRITE")) {
            overwrite = true;
        } else {
            return kSyntax;
        }
    }

    return flash_disk_load(flash_index, filename, overwrite);
}

#define ELSE_IF_UNIMPLEMENTED(s) \
    else if ((p = checkstring(cmdline, s))) { \
        ERROR_UNIMPLEMENTED("FLASH " s); \
    }

void cmd_flash(void) {
    if (mmb_options.simulate != kSimulateGameMite && mmb_options.simulate != kSimulatePicoMiteVga) {
        ON_FAILURE_LONGJMP(kUnsupportedOnCurrentDevice);
    }
    MmResult result = kOk;
    const char *p;

    if ((p = checkstring(cmdline, "DISK LOAD"))) {
        result = cmd_flash_disk_load(p);
    }
    ELSE_IF_UNIMPLEMENTED("ERASE ALL")
    ELSE_IF_UNIMPLEMENTED("ERASE")
    ELSE_IF_UNIMPLEMENTED("OVERWRITE")
    ELSE_IF_UNIMPLEMENTED("LIST")
    ELSE_IF_UNIMPLEMENTED("SAVE")
    ELSE_IF_UNIMPLEMENTED("LOAD")
    ELSE_IF_UNIMPLEMENTED("CHAIN")
    ELSE_IF_UNIMPLEMENTED("RUN")
    else {
        ERROR_UNKNOWN_SUBCOMMAND("FLASH");
    }

    ON_FAILURE_LONGJMP(result);
}
