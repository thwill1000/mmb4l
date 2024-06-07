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
#include "../core/tokentbl.h"

/** GRAPHICS BUFFER id, width, height */
static void cmd_graphics_buffer(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 5) ERROR_ARGUMENT_COUNT;
    int id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    int width = getint(argv[2], 8, WINDOW_MAX_WIDTH);
    int height = getint(argv[4], 8, WINDOW_MAX_HEIGHT);

    MmResult result = graphics_buffer_create(id, width, height);
    if (FAILED(result)) error_throw(result);
}

/** GRAPHICS COPY n TO m [,when] [,t] */
static void cmd_graphics_copy(const char *p) {
    bool transparent_black = false;
    char ss[3];
    ss[0] = tokenTO;
    ss[1] =',';
    ss[2] = 0;
    getargs(&p, 7, ss);
    if (argc<3) ERROR_ARGUMENT_COUNT;
    MMINTEGER read_id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    MMINTEGER write_id = getint(argv[2], 0, GRAPHICS_MAX_ID);

    if (!graphics_surface_exists(read_id)) {
        error_throw_ex(kGraphicsSurfaceNotFound, "Read surface does not exist: %%", read_id);
    }
    if (!graphics_surface_exists(write_id)) {
        error_throw_ex(kGraphicsSurfaceNotFound, "Write surface does not exist: %%", write_id);
    }

    MmSurface* read_surface = &graphics_surfaces[read_id];
    MmSurface* write_surface = &graphics_surfaces[write_id];

    if (read_surface->width != write_surface->width || read_surface->height != write_surface->height) {
        error_throw_ex(kError, "Surface size mismatch - use BLIT");
    }

    if (argc >= 5 && *argv[4]) {
        const char *p = argv[4];
        switch (toupper(*p)) {
            case 'I':
            case 'B':
            case 'D':
                // Ignore for now
                break;
            default:
                ERROR_SYNTAX;
                break;
        }
    }

    if (argc == 7) {
        const char *p = argv[6];
        if (toupper(*p) == 'T' || *p == '1') transparent_black = true;
    }

    MmResult result = graphics_blit(0, 0, 0, 0, read_surface->width, read_surface->height,
                                    read_surface, write_surface, transparent_black ? 0x4 : 0x0,
                                    RGB_BLACK);
    if (FAILED(result)) error_throw(result);
}

/** GRAPHICS WINDOW id, x, y, width, height [, scale] [, interrupt] */
static void cmd_graphics_window(const char *p) {
    getargs(&p, 13, ",");
    if (argc < 9 || argc > 13) ERROR_ARGUMENT_COUNT;
    MMINTEGER id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    MMINTEGER x = getint(argv[2], -1, WINDOW_MAX_X);
    MMINTEGER y = getint(argv[4], -1, WINDOW_MAX_Y);
    MMINTEGER width = getint(argv[6], 8, WINDOW_MAX_WIDTH);
    MMINTEGER height = getint(argv[8], 8, WINDOW_MAX_HEIGHT);
    MMINTEGER scale = argc == 11 ? getint(argv[10], 1, WINDOW_MAX_SCALE) : 1;
    const char* interrupt_addr = argc == 13 ? GetIntAddress(argv[12]): NULL;
    if (interrupt_addr) {
        // Check interrupt is a SUB with the correct signature.
        FunctionSignature *fn = (FunctionSignature *) GetTempMemory(sizeof(FunctionSignature));
        const char *p2 = interrupt_addr;
        const char *cached_line_ptr = CurrentLinePtr;
        CurrentLinePtr = interrupt_addr; // So any error is reported on the correct line.
        MmResult result = parse_fn_sig(&p2, fn);
        if (FAILED(result)) error_throw(result);
        CurrentLinePtr = cached_line_ptr;
        if (fn->token != cmdSUB
            || fn->num_params != 2
            || !(fn->params[0].type & T_INT)
            || !(fn->params[1].type & T_INT)
            || fn->params[0].array
            || fn->params[1].array) error_throw(kInvalidInterruptSignature);
        ClearSpecificTempMemory(fn);
    }

    // TODO width & height should be divisible by 8.
    // TODO check window has not already been created.

    // printf("%d %d %d %d %d\n", id, x, y, width, height);

    char title[256];
    sprintf(title, "MMB4L: %ld", id);
    MmResult result = graphics_window_create(id, x, y, width, height, scale, title, interrupt_addr);
    if (FAILED(result)) error_throw(result);
}

/** GRAPHICS DESTROY id | ALL */
static void cmd_graphics_destroy(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    if ((p = checkstring(argv[0], "ALL"))) {
        MmResult result = graphics_surface_destroy_all();
        if (FAILED(result)) error_throw(result);
    } else {
        int id = getint(argv[0], 0, GRAPHICS_MAX_ID);
        MmResult result = graphics_surface_destroy(id);
        if (FAILED(result)) error_throw(result);
    }
}

/** GRAPHICS WRITE id */
static void cmd_graphics_write(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    const int id = getint(argv[0], -1, GRAPHICS_MAX_ID);

    MmResult result = graphics_surface_write(id);
    if (FAILED(result)) error_throw(result);
}

void cmd_graphics(void) {
    const char *p;
    if ((p = checkstring(cmdline, "BUFFER"))) {
        cmd_graphics_buffer(p);
    } else if ((p = checkstring(cmdline, "COPY"))) {
        cmd_graphics_copy(p);
    } else if ((p = checkstring(cmdline, "DESTROY"))) {
        cmd_graphics_destroy(p);
    } else if ((p = checkstring(cmdline, "WINDOW"))) {
        cmd_graphics_window(p);
    } else if ((p = checkstring(cmdline, "WRITE"))) {
        cmd_graphics_write(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("GRAPHICS");
    }
}
