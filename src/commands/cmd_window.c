/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_window.c

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

#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/graphics.h"
#include "../common/utility.h"

#define CHECK_GRAPHICS_RESULT(result) \
    if (FAILED(result)) { \
        error_throw_ex( \
            result, \
            "$: $", \
            mmresult_to_string(result), \
            graphics_last_error()); \
    }

/** WINDOW CREATE id, x, y, width, height */
static void cmd_window_create(const char *p) {
    getargs(&p, 9, ",");
    if (argc != 9) ERROR_ARGUMENT_COUNT;
    int id = getint(argv[0], 0, WINDOW_MAX_ID);
    int x = getint(argv[2], 0, WINDOW_MAX_X);
    int y = getint(argv[4], 0, WINDOW_MAX_Y);
    int width = getint(argv[6], 0, WINDOW_MAX_WIDTH);
    int height = getint(argv[8], 0, WINDOW_MAX_HEIGHT);

    // TODO width & height should be divisible by 8.
    // TODO check window has not already been created.

    // printf("%d %d %d %d %d\n", id, x, y, width, height);

    CHECK_GRAPHICS_RESULT(graphics_window_create(id, x, y, width, height));
}

/** WINDOW DESTROY id */
static void cmd_window_destroy(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    int id = getint(argv[0], 0, WINDOW_MAX_ID);

    CHECK_GRAPHICS_RESULT(graphics_window_destroy(id));
}

/** WINDOW USE id */
static void cmd_window_use(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    const int id = getint(argv[0], 0, WINDOW_MAX_ID);
    const MmResult result = graphics_window_use(id);
    if (FAILED(result)) error_throw(result);
}

void cmd_window(void) {
    const char *p;
    if ((p = checkstring(cmdline, "CREATE"))) {
        cmd_window_create(p);
    } else if ((p = checkstring(cmdline, "DESTROY"))) {
        cmd_window_destroy(p);
    } else if ((p = checkstring(cmdline, "USE"))) {
        cmd_window_use(p);
    } else {
        ERROR_UNKNOWN_ARGUMENT;
    }
}
