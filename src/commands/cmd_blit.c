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
static MmResult cmd_blit_close(const char *p) {
    skipspace(p);
    if (*p == '#') p++;
    MmSurfaceId surface_id = getint(p, 0, GRAPHICS_MAX_ID);
    if (mmb_options.simulate == kSimulateCmm2) surface_id += CMM2_BLIT_BUF_BASE;
    MmResult result = kOk;
    if (!graphics_surface_exists(surface_id)) {
        result = kGraphicsSurfaceNotFound;
    } else if (graphics_surfaces[surface_id].type == kGraphicsWindow) {
        result = kCannotBlitCloseWindow;
    } else {
        result = graphics_surface_destroy(&graphics_surfaces[surface_id]);
    }
    return result;
}

/** BLIT CLOSE ALL */
static MmResult cmd_blit_close_all(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) return kUnexpectedText;
    MmResult result = kOk;
    const MmSurfaceId start_id = mmb_options.simulate == kSimulateCmm2 ? CMM2_BLIT_BUF_BASE : 0;
    const MmSurfaceId end_id = mmb_options.simulate == kSimulateCmm2
            ? CMM2_BLIT_BUF_BASE + CMM2_BLIT_BUF_COUNT
            : GRAPHICS_MAX_ID;
    for (MmSurfaceId surface_id = start_id; surface_id <= end_id; ++surface_id) {
        if (graphics_surfaces[surface_id].type != kGraphicsNone
                && graphics_surfaces[surface_id].type != kGraphicsWindow) {
            MmResult local_result = graphics_surface_destroy(&graphics_surfaces[surface_id]);
            if (FAILED(local_result) && SUCCEEDED(result)) result = local_result;
        }
    }
    return result;
}

static MmResult cmd_blit_compressed(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT COMPRESSED");
    return kUnimplemented;
}

static MmResult cmd_blit_framebuffer(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT FRAMEBUFFER");
    return kUnimplemented;
}

static MmResult cmd_blit_memory(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT MEMORY");
    return kUnimplemented;
}

/** BLIT READ [#]b, x, y, w, h [,pagenumber] */
static MmResult cmd_blit_read(const char *p) {
    getargs(&p, 11, ",");
    if (!(argc == 9 || argc == 11)) return kArgumentCount;
    if (*argv[0] == '#') argv[0]++;
    MmSurfaceId write_id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    const int x = getint(argv[2], 0, WINDOW_MAX_X);
    const int y = getint(argv[4], 0, WINDOW_MAX_Y);
    const int w = getint(argv[6], 0, WINDOW_MAX_WIDTH);
    const int h = getint(argv[8], 0, WINDOW_MAX_HEIGHT);
    if (w < 1 || h < 1) return kOk;

    if (mmb_options.simulate == kSimulateCmm2) write_id += CMM2_BLIT_BUF_BASE;
    // TODO: Check for CMM2 buffer out of range.
    if (graphics_surfaces[write_id].type == kGraphicsNone) {
        MmResult result = graphics_buffer_create(write_id, w, h);
        if (FAILED(result)) return result;
    } else {
        if (graphics_surfaces[write_id].width != w || graphics_surfaces[write_id].height != h) {
            return kGraphicsSurfaceSizeMismatch;
        }
    }
    MmSurface *write_surface = &graphics_surfaces[write_id];
    MmSurface *read_surface = graphics_current;
    if (argc == 11) {
        if (checkstring(argv[10], "FRAMEBUFFER")) {
            // TODO
        }
        else {
            const MmSurfaceId read_id = getint(argv[10], 0, GRAPHICS_MAX_ID);
            read_surface = &graphics_surfaces[read_id];
        }
    }
    return graphics_blit(x, y, 0, 0, w, h, read_surface, write_surface, 0x0);
}

/** BLIT WRITE [#]b, x, y [,orientation] */
static MmResult cmd_blit_write(const char *p) {
    getargs(&p, 7, ",");
    if (!(argc == 5 || argc == 7)) return kArgumentCount;

    if (*argv[0] == '#') argv[0]++;
    MmSurfaceId read_id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    if (!graphics_surface_exists(read_id)) return kGraphicsInvalidReadSurface;
    if (mmb_options.simulate == kSimulateCmm2) read_id += CMM2_BLIT_BUF_BASE;
    // TODO: Check for CMM2 buffer out of range.
    MmSurface *read_surface = &graphics_surfaces[read_id];

    const int x = getint(argv[2], -read_surface->width + 1, WINDOW_MAX_X);
    const int y = getint(argv[4], -read_surface->height + 1, WINDOW_MAX_Y);

    MmSurface *write_surface = graphics_current;
    return graphics_blit(0, 0, x, y, read_surface->width, read_surface->height,
                         read_surface, write_surface, 0x0);
}

/** BLIT x1, y1, x2, y2, w, h [,surface] [,flags] */
static MmResult cmd_blit_default(const char *p) {
    if (!graphics_current) return kGraphicsInvalidWriteSurface;

    getargs(&p, 15, ",");
    if (argc < 11 || argc > 15) return kArgumentCount;
    const int x1 = getinteger(argv[0]);
    const int y1 = getinteger(argv[2]);
    const int x2 = getinteger(argv[4]);
    const int y2 = getinteger(argv[6]);
    const int w = getinteger(argv[8]);
    const int h = getinteger(argv[10]);
    const MmSurfaceId read_id = (argc >= 13) ? getint(argv[12], 0, GRAPHICS_MAX_ID) : -1;
    MmSurface *read_surface = graphics_current;
    if (read_id != -1) {
        if (graphics_surface_exists(read_id)) {
            read_surface = &graphics_surfaces[read_id];
        } else {
            return kGraphicsInvalidReadSurface;
        }
    }
    const int flags = (argc == 15) ? getint(argv[14], 0, 7) : 0x0;
    MmSurface* write_surface = graphics_current;
    return graphics_blit(x1, y1, x2, y2, w, h, read_surface, write_surface, flags);
}

void cmd_blit(void) {
    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(cmdline, "CLOSE ALL"))) {
        result = cmd_blit_close_all(p);
    } else if ((p = checkstring(cmdline, "CLOSE"))) {
        result = cmd_blit_close(p);
    } else if ((p = checkstring(cmdline, "COMPRESSED"))) {
        result = cmd_blit_compressed(p);
    } else if ((p = checkstring(cmdline, "FRAMEBUFFER"))) {
        result = cmd_blit_framebuffer(p);
    } else if ((p = checkstring(cmdline, "MEMORY"))) {
        result = cmd_blit_memory(p);
    } else if ((p = checkstring(cmdline, "READ"))) {
        result = cmd_blit_read(p);
    } else if ((p = checkstring(cmdline, "WRITE"))) {
        result = cmd_blit_write(p);
    } else {
        result = cmd_blit_default(cmdline);
    }
    ERROR_ON_FAILURE(result);
}
