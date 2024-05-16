/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_blit.c

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
#include "../common/graphics.h"
#include "../common/mmb4l.h"

#define CMM2_BLIT_BUF_BASE   63
#define CMM2_BLIT_BUF_COUNT  64

/** BLIT CLOSE [#]b */
static void cmd_blit_close(const char *p) {
    skipspace(p);
    if (*p == '#') p++;
    MMINTEGER surface_id = getint(p, 0, GRAPHICS_MAX_ID);
    if (mmb_options.simulate == kSimulateCmm2) surface_id += CMM2_BLIT_BUF_BASE;
    MmResult result = kOk;
    if (!graphics_surface_exists(surface_id)) {
        result = kGraphicsSurfaceNotFound;
    } else if (graphics_surfaces[surface_id].type == kGraphicsWindow) {
        result = kCannotBlitCloseWindow;
    } else {
        result = graphics_surface_destroy(surface_id);
    }
    if (FAILED(result)) error_throw(result);
}

/** BLIT CLOSE ALL */
static void cmd_blit_close_all(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) {
        error_throw(kUnexpectedText);
        return;
    }
    MmResult result = kOk;
    const MmSurfaceId start_id = mmb_options.simulate == kSimulateCmm2 ? CMM2_BLIT_BUF_BASE : 0;
    const MmSurfaceId end_id = mmb_options.simulate == kSimulateCmm2
            ? CMM2_BLIT_BUF_BASE + CMM2_BLIT_BUF_COUNT
            : GRAPHICS_MAX_ID;
    for (MmSurfaceId surface_id = start_id; surface_id <= end_id; ++surface_id) {
        if (graphics_surfaces[surface_id].type != kGraphicsNone
                && graphics_surfaces[surface_id].type != kGraphicsWindow) {
            MmResult local_result = graphics_surface_destroy(surface_id);
            if (FAILED(local_result) && SUCCEEDED(result)) result = local_result;
        }
    }
    if (FAILED(result)) error_throw(result);
}

/** BLIT COMPRESSED address, x, y [, transparent] */
static void cmd_blit_compressed(const char *p) {
    getargs(&p, 7, ",");
    if (argc != 5 && argc != 7) ERROR_ARGUMENT_COUNT;
    char *data = (char *) get_peek_addr(argv[0]);
    const int32_t x = getint(argv[2], INT32_MIN, INT32_MAX);
    const int32_t y = getint(argv[4], INT32_MIN, INT32_MAX);
    const uint32_t transparent = argc == 7 ? getint(argv[6], -1, 15) : -1;

    const uint16_t *size = (uint16_t *) data;
    const uint32_t w = size[0] & 0x7FFF;
    const uint32_t h = size[1] & 0x7FFF;
    data += 4;

    MmResult result = graphics_blit_memory_compressed(graphics_current, data, x, y, w, h,
                                                      transparent);
    if (FAILED(result)) error_throw(result);
}

static void cmd_blit_copy(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT COPY");
}

static void cmd_blit_hide(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT HIDE");
}

static void cmd_blit_hide_all(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT HIDE ALL");
}

static void cmd_blit_hide_safe(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT HIDE SAFE");
}

static void cmd_blit_interrupt(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT INTERRUPT");
}

static void cmd_blit_load_sprite(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT LOAD");
}

static void cmd_blit_load_array(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT LOADARRAY");
}

static void cmd_blit_load_png(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT LOADPNG");
}

/** BLIT MEMORY address, x, y [, transparent] */
static void cmd_blit_memory(const char *p) {
    getargs(&p, 7, ",");
    if (argc != 5 && argc != 7) ERROR_ARGUMENT_COUNT;
    char *data = (char *) get_peek_addr(argv[0]);
    const int32_t x = getint(argv[2], INT32_MIN, INT32_MAX);
    const int32_t y = getint(argv[4], INT32_MIN, INT32_MAX);
    const uint32_t transparent = argc == 7 ? getint(argv[6], -1, 15) : -1;

    const uint16_t *size = (uint16_t *) data;
    const uint32_t w = size[0] & 0x7FFF;
    const uint32_t h = size[1] & 0x7FFF;
    const bool compressed = size[0] & 0x8000 || size[1] & 0x8000;
    data += 4;

    MmResult result = compressed
        ? graphics_blit_memory_compressed(graphics_current, data, x, y, w, h, transparent)
        : graphics_blit_memory_uncompressed(graphics_current, data, x, y, w, h, transparent);
    if (FAILED(result)) error_throw(result);
}

static void cmd_blit_move(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT MOVE");
}

static void cmd_blit_next(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT NEXT");
}

static void cmd_blit_nointerrupt(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT NOINTERRUPT");
}

/** BLIT READ [#]b, x, y, w, h [,pagenumber] */
static void cmd_blit_read(const char *p) {
    getargs(&p, 11, ",");
    if (!(argc == 9 || argc == 11)) ERROR_ARGUMENT_COUNT;
    if (*argv[0] == '#') argv[0]++;
    MMINTEGER write_id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    MMINTEGER x = getint(argv[2], 0, WINDOW_MAX_X);
    MMINTEGER y = getint(argv[4], 0, WINDOW_MAX_Y);
    MMINTEGER w = getint(argv[6], 0, WINDOW_MAX_WIDTH);
    MMINTEGER h = getint(argv[8], 0, WINDOW_MAX_HEIGHT);
    if (w < 1 || h < 1) return;

    if (mmb_options.simulate == kSimulateCmm2) write_id += CMM2_BLIT_BUF_BASE;
    // TODO: Check for CMM2 buffer out of range.
    if (graphics_surfaces[write_id].type == kGraphicsNone) {
        MmResult result = graphics_buffer_create(write_id, w, h);
        if (FAILED(result)) error_throw(result);
    } else {
        if (graphics_surfaces[write_id].width != w || graphics_surfaces[write_id].height != h) {
            error_throw_ex(kError, "Existing surface is incorrect size");
            return;
        }
    }
    MmSurface *write_surface = &graphics_surfaces[write_id];
    MmSurface *read_surface = graphics_current;
    if (argc == 11) {
        if (checkstring(argv[10], "FRAMEBUFFER")) {
            // TODO
        }
        else {
            MMINTEGER read_id = getint(argv[10], 0, GRAPHICS_MAX_ID);
            read_surface = &graphics_surfaces[read_id];
        }
    }
    graphics_blit(x, y, 0, 0, w, h, read_surface, write_surface, 0x0);
}

static void cmd_blit_restore(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT RESTORE");
}

static void cmd_blit_scroll(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT SCROLL");
}

static void cmd_blit_scrollr(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT SCROLLR");
}

static void cmd_blit_show(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT SHOW");
}

static void cmd_blit_show_safe(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT SHOW SAFE");
}

static void cmd_blit_swap(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT SWAP");
}

static void cmd_blit_transparency(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT TRANSPARENCY");
}

/** BLIT WRITE [#]b, x, y [,orientation] */
static void cmd_blit_write(const char *p) {
    getargs(&p, 7, ",");
    if (!(argc == 5 || argc == 7)) ERROR_ARGUMENT_COUNT;

    if (*argv[0] == '#') argv[0]++;
    MMINTEGER read_id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    if (!graphics_surface_exists(read_id)) {
        error_throw_ex(kGraphicsSurfaceNotFound, "Read surface does not exist");
        return;
    }
    if (mmb_options.simulate == kSimulateCmm2) read_id += CMM2_BLIT_BUF_BASE;
    // TODO: Check for CMM2 buffer out of range.
    MmSurface *read_surface = &graphics_surfaces[read_id];

    MMINTEGER x = getint(argv[2], -read_surface->width + 1, WINDOW_MAX_X);
    MMINTEGER y = getint(argv[4], -read_surface->height + 1, WINDOW_MAX_Y);

    MmSurface *write_surface = graphics_current;
    MmResult result = graphics_blit(0, 0, x, y, read_surface->width, read_surface->height,
                                    read_surface, write_surface, 0x0);
    if (FAILED(result)) error_throw(result);
}

/** BLIT x1, y1, x2, y2, w, h [,surface] [,flags] */
static void cmd_blit_default(const char *p) {
    if (!graphics_current) error_throw_ex(kGraphicsSurfaceNotFound, "Write surface does not exist");

    getargs(&p, 15, ",");
    if (argc < 11 || argc > 15) ERROR_ARGUMENT_COUNT;
    MMINTEGER x1 = getinteger(argv[0]);
    MMINTEGER y1 = getinteger(argv[2]);
    MMINTEGER x2 = getinteger(argv[4]);
    MMINTEGER y2 = getinteger(argv[6]);
    MMINTEGER w = getinteger(argv[8]);
    MMINTEGER h = getinteger(argv[10]);
    MmSurface *read_surface = graphics_current;
    if (argc >= 13) {
        MMINTEGER read_id = getint(argv[12], 0, GRAPHICS_MAX_ID);
        if (graphics_surface_exists(read_id)) {
            read_surface = &graphics_surfaces[read_id];
        } else {
            error_throw_ex(kGraphicsSurfaceNotFound, "Read surface does not exist: %%", read_id);
            return;
        }
    }
    MMINTEGER flags = argc == 15 ? getint(argv[14], 0, 7) : 0x0;

    MmSurface* write_surface = graphics_current;
    MmResult result = graphics_blit(x1, y1, x2, y2, w, h, read_surface, write_surface, flags);
    if (FAILED(result)) error_throw(result);
}

void cmd_blit(void) {
    const char *p;
    if ((p = checkstring(cmdline, "CLOSE ALL"))) {
        cmd_blit_close_all(p);
    } else if ((p = checkstring(cmdline, "CLOSE"))) {
        cmd_blit_close(p);
    } else if ((p = checkstring(cmdline, "COMPRESSED"))) {
        cmd_blit_compressed(p);
    } else if ((p = checkstring(cmdline, "COPY"))) {
        cmd_blit_copy(p);
    } else if ((p = checkstring(cmdline, "HIDE ALL"))) {
        cmd_blit_hide_all(p);
    } else if ((p = checkstring(cmdline, "HIDE SAFE"))) {
        cmd_blit_hide_safe(p);
    } else if ((p = checkstring(cmdline, "HIDE"))) {
        cmd_blit_hide(p);
    } else if ((p = checkstring(cmdline, "INTERRUPT"))) {
        cmd_blit_interrupt(p);
    } else if ((p = checkstring(cmdline, "LOAD"))) {
        cmd_blit_load_sprite(p);
    } else if ((p = checkstring(cmdline, "LOADARRAY"))) {
        cmd_blit_load_array(p);
    } else if ((p = checkstring(cmdline, "LOADPNG"))) {
        cmd_blit_load_png(p);
    } else if ((p = checkstring(cmdline, "MOVE"))) {
        cmd_blit_move(p);
    } else if ((p = checkstring(cmdline, "MEMORY"))) {
        cmd_blit_memory(p);
    } else if ((p = checkstring(cmdline, "NEXT"))) {
        cmd_blit_next(p);
    } else if ((p = checkstring(cmdline, "NOINTERRUPT"))) {
        cmd_blit_nointerrupt(p);
    } else if ((p = checkstring(cmdline, "SCROLL"))) {
        cmd_blit_scroll(p);
    } else if ((p = checkstring(cmdline, "SCROLLR"))) {
        cmd_blit_scrollr(p);
    } else if ((p = checkstring(cmdline, "SHOW SAFE"))) {
        cmd_blit_show_safe(p);
    } else if ((p = checkstring(cmdline, "SHOW"))) {
        cmd_blit_show(p);
    } else if ((p = checkstring(cmdline, "READ"))) {
        cmd_blit_read(p);
    } else if ((p = checkstring(cmdline, "RESTORE"))) {
        cmd_blit_restore(p);
    } else if ((p = checkstring(cmdline, "SWAP"))) {
        cmd_blit_swap(p);
    } else if ((p = checkstring(cmdline, "TRANSPARENCY"))) {
        cmd_blit_transparency(p);
    } else if ((p = checkstring(cmdline, "WRITE"))) {
        cmd_blit_write(p);
    } else {
        cmd_blit_default(cmdline);
    }
}
