/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_graphics.c

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

/** GRAPHICS BUFFER id, width, height */
static MmResult cmd_graphics_buffer(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 5) return kArgumentCount;
    int id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    int width = getint(argv[2], 8, WINDOW_MAX_WIDTH);
    int height = getint(argv[4], 8, WINDOW_MAX_HEIGHT);

    return graphics_buffer_create(id, width, height);
}

/** GRAPHICS WINDOW id, x, y, width, height, scale */
static MmResult cmd_graphics_window(const char *p) {
    getargs(&p, 11, ",");
    if (argc != 9 && argc != 11) return kArgumentCount;
    int id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    int x = getint(argv[2], -1, WINDOW_MAX_X);
    int y = getint(argv[4], -1, WINDOW_MAX_Y);
    int width = getint(argv[6], 8, WINDOW_MAX_WIDTH);
    int height = getint(argv[8], 8, WINDOW_MAX_HEIGHT);
    int scale = argc == 11 ? getint(argv[10], 1, WINDOW_MAX_SCALE) : 1;

    // TODO width & height should be divisible by 8.
    // TODO check window has not already been created.

    return graphics_window_create(id, x, y, width, height, scale);
}

/** GRAPHICS DESTROY id | ALL */
static MmResult cmd_graphics_destroy(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) return kArgumentCount;
    if ((p = checkstring(argv[0], "ALL"))) {
        return graphics_surface_destroy_all();
    } else {
        int surface_id = getint(argv[0], 0, GRAPHICS_MAX_ID);
        return graphics_surface_destroy(&graphics_surfaces[surface_id]);
    }
}

/** GRAPHICS WRITE id */
static MmResult cmd_graphics_write(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) return kArgumentCount;
    const int id = getint(argv[0], -1, GRAPHICS_MAX_ID);
    return graphics_surface_write(id);
}

void cmd_graphics(void) {
    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(cmdline, "BUFFER"))) {
        result = cmd_graphics_buffer(p);
    } else if ((p = checkstring(cmdline, "DESTROY"))) {
        result = cmd_graphics_destroy(p);
    } else if ((p = checkstring(cmdline, "WINDOW"))) {
        result = cmd_graphics_window(p);
    } else if ((p = checkstring(cmdline, "WRITE"))) {
        result = cmd_graphics_write(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("GRAPHICS");
    }
    ERROR_ON_FAILURE(result);
}
