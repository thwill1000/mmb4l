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

/** BLIT CLOSE [#]id */
static MmResult cmd_blit_close(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) return kArgumentCount;
    MmSurfaceId blit_id = -1;
    MmResult result = parse_blit_id(p, true, &blit_id);
    if (SUCCEEDED(result)) {
        MmSurface *surface = &graphics_surfaces[blit_id];
        if (surface->type == kGraphicsBuffer) {
            result = graphics_surface_destroy(surface);
        } else {
            result = kGraphicsInvalidSurface;
        }
    }
    return result;
}

/** BLIT CLOSE ALL */
static MmResult cmd_blit_close_all(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) return kUnexpectedText;
    MmResult result = kOk;
    const MmSurfaceId start_id = (mmb_options.simulate == kSimulateMmb4l)
            ? 0
            : CMM2_BLIT_BASE + 1; // 64
    const MmSurfaceId end_id = (mmb_options.simulate == kSimulateMmb4l)
            ? GRAPHICS_MAX_ID
            : CMM2_BLIT_BASE + CMM2_BLIT_COUNT; // 127
    for (MmSurfaceId surface_id = start_id; surface_id <= end_id; ++surface_id) {
        MmSurface *surface = &graphics_surfaces[surface_id];
        if (surface->type == kGraphicsBuffer) {
            MmResult local_result = graphics_surface_destroy(surface);
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

/**
 * BLIT READ [#]dst_id, x, y, width, height [, src_id]
 *  - <dst_id> parameter unsupported on PicoMite{VGA}.
 */
MmResult cmd_blit_read(const char *p) {
    getargs(&p, 11, ",");
    if (!(argc == 9 || argc == 11)) return kArgumentCount;
    MmSurfaceId dst_id = -1;
    MmResult result = parse_blit_id(argv[0], false, &dst_id);
    if (result == kGraphicsInvalidSurface) result = kGraphicsInvalidWriteSurface;
    if (FAILED(result)) return result;
    const int x = getint(argv[2], 0, WINDOW_MAX_X);
    const int y = getint(argv[4], 0, WINDOW_MAX_Y);
    const int width = getint(argv[6], 0, WINDOW_MAX_WIDTH);
    const int height = getint(argv[8], 0, WINDOW_MAX_HEIGHT);
    if (width < 1 || height < 1) return kOk;

    MmSurface *dst_surface = &graphics_surfaces[dst_id];
    if (dst_surface->type == kGraphicsNone) {
        MmResult result = graphics_buffer_create(dst_id, width, height);
        if (FAILED(result)) return result;
    } else if (dst_surface->width != width || dst_surface->height != height) {
        return kGraphicsSurfaceSizeMismatch;
    }

    MmSurface *src_surface = graphics_current;
    if (argc == 11) {
        if (mmb_options.simulate == kSimulateGameMite
                || mmb_options.simulate == kSimulatePicoMiteVga) {
            return kUnsupportedParameterOnCurrentDevice;
        }
        MmSurfaceId src_id = -1;
        MmResult result = parse_read_page(argv[10], &src_id);
        if (FAILED(result)) return result;
        src_surface = &graphics_surfaces[src_id];
    }

    return graphics_blit(x, y, 0, 0, width, height, src_surface, dst_surface, 0x0);
}

/** BLIT WRITE [#]src_id, x, y [, orientation] */
MmResult cmd_blit_write(const char *p) {
    getargs(&p, 7, ",");
    if (!(argc == 5 || argc == 7)) return kArgumentCount;
    MmSurfaceId src_id = -1;
    MmResult result = parse_blit_id(argv[0], true, &src_id);
    if (result == kGraphicsInvalidSurface) result = kGraphicsInvalidReadSurface;
    if (FAILED(result)) return result;
    MmSurface *src_surface = &graphics_surfaces[src_id];

    const int x = getint(argv[2], -src_surface->width + 1, WINDOW_MAX_X);
    const int y = getint(argv[4], -src_surface->height + 1, WINDOW_MAX_Y);

    MmSurface *dst_surface = graphics_current;
    return graphics_blit(0, 0, x, y, src_surface->width, src_surface->height, src_surface,
                         dst_surface, 0x0);
}

/** BLIT x1, y1, x2, y2, width, height [, src_id] [, flags] */
static MmResult cmd_blit_default(const char *p) {
    if (!graphics_current) return kGraphicsInvalidWriteSurface;

    getargs(&p, 15, ",");
    if (argc < 11 || argc > 15) return kArgumentCount;
    const int x1 = getinteger(argv[0]);
    const int y1 = getinteger(argv[2]);
    const int x2 = getinteger(argv[4]);
    const int y2 = getinteger(argv[6]);
    const int width = getinteger(argv[8]);
    const int height = getinteger(argv[10]);
    MmSurface *src_surface = graphics_current;
    if (argc >= 13) {
        MmSurfaceId src_id = -1;
        MmResult result = parse_blit_id(argv[0], true, &src_id);
        if (result == kGraphicsInvalidSurface) result = kGraphicsInvalidReadSurface;
        if (FAILED(result)) return result;
        src_surface = &graphics_surfaces[src_id];
    }
    const int flags = (argc == 15) ? getint(argv[14], 0, 7) : 0x0;
    MmSurface* dst_surface = graphics_current;
    return graphics_blit(x1, y1, x2, y2, width, height, src_surface, dst_surface, flags);
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
