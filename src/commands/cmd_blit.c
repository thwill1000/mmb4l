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
#include "../common/sprite.h"
#include "../common/mmb4l.h"
#include "../common/utility.h"

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

/** BLIT COMPRESSED address, x, y [, transparent] */
MmResult cmd_blit_compressed(const char *p) {
    getargs(&p, 7, ",");
    if (argc != 5 && argc != 7) return kArgumentCount;
    char *data = (char *) get_peek_addr(argv[0]);
    const int x = getint(argv[2], INT32_MIN, INT32_MAX);
    const int y = getint(argv[4], INT32_MIN, INT32_MAX);
    const int transparent = (argc == 7) ? getint(argv[6], -1, 15) : -1;

    const uint16_t *size = (uint16_t *) data;
    const int w = size[0] & 0x7FFF;
    const int h = size[1] & 0x7FFF;
    data += 4;

    return graphics_blit_memory_compressed(graphics_current, data, x, y, w, h, transparent);
}

/** BLIT FRAMEBUFFER from, to, x1, y1, x2, y2, w, h [, transparent] */
MmResult cmd_blit_framebuffer(const char *p) {
    if (mmb_options.simulate != kSimulateGameMite && mmb_options.simulate != kSimulatePicoMiteVga) {
        return kUnsupportedOnCurrentDevice;
    }

    getargs(&p, 17, ",");
    if (argc < 15) return kArgumentCount;

    MmSurfaceId src_id = -1;
    MmResult result = parse_read_page(argv[0], &src_id);
    if (FAILED(result)) return result;
    MmSurface *src_surface = &graphics_surfaces[src_id];

    MmSurfaceId dst_id = -1;
    result = parse_write_page(argv[2], &dst_id);
    if (FAILED(result)) return result;
    MmSurface *dst_surface = &graphics_surfaces[dst_id];

    if (src_surface == dst_surface) return kGraphicsReadAndWriteSurfaceSame;

    const int x1 = getinteger(argv[4]);
    const int y1 = getinteger(argv[6]);
    const int x2 = getinteger(argv[8]);
    const int y2 = getinteger(argv[10]);
    const int w = getinteger(argv[12]);
    const int h = getinteger(argv[14]);
    int8_t t4bit = (argc == 17) ? getint(argv[16], 0, 15) : -1;

    const unsigned flags = (t4bit == -1) ? 0x0 : 0x4;
    const MmGraphicsColour transparent = (t4bit == -1) ? RGB_BLACK : GRAPHICS_RGB121_COLOURS[t4bit];

    return graphics_blit(x1, y1, x2, y2, w, h, src_surface, dst_surface, flags, transparent);
}

/** BLIT MEMORY address, x, y [, transparent] */
MmResult cmd_blit_memory(const char *p) {
    getargs(&p, 7, ",");
    if (argc != 5 && argc != 7) return kArgumentCount;
    char *data = (char *) get_peek_addr(argv[0]);
    const int x = getint(argv[2], INT32_MIN, INT32_MAX);
    const int y = getint(argv[4], INT32_MIN, INT32_MAX);
    const int transparent = (argc == 7) ? getint(argv[6], -1, 15) : -1;

    const uint16_t *size = (uint16_t *) data;
    const int w = size[0] & 0x7FFF;
    const int h = size[1] & 0x7FFF;
    const bool compressed = size[0] & 0x8000 || size[1] & 0x8000;
    data += 4;

    return compressed
        ? graphics_blit_memory_compressed(graphics_current, data, x, y, w, h, transparent)
        : graphics_blit_memory_uncompressed(graphics_current, data, x, y, w, h, transparent);
}

/**
 * BLIT READ [#]dst_id, x, y, width, height [, src_id]
 *  - <src_id> parameter unsupported on PicoMite{VGA}.
 *
 * @param  sprite  If true then parse as SPRITE READ instead of BLIT READ.
 */
MmResult cmd_blit_read(const char *p, bool sprite) {
    getargs(&p, 11, ",");
    if (argc != 9 && argc != 11) return kArgumentCount;

    if (has_arg(10) && (mmb_options.simulate == kSimulateGameMite
            || mmb_options.simulate == kSimulatePicoMiteVga)) {
        return kUnsupportedParameterOnCurrentDevice;
    }

    MmResult result = kOk;
    MmSurface *dst_surface = NULL;
    {
        MmSurfaceId dst_id = -1;
        result = sprite
            ? parse_sprite_id(argv[0], false, &dst_id)
            : parse_blit_id(argv[0], false, &dst_id);
        if (result == kGraphicsInvalidSurface) result = kGraphicsInvalidWriteSurface;
        if (FAILED(result)) return result;
        dst_surface = &graphics_surfaces[dst_id];
    }

    // if (sprite && dst_surface->type != kGraphicsSprite
    //         && dst_surface->type != kGraphicsInactiveSprite) {
    //     return kGraphicsInvalidSprite;
    // }

    int x = getint(argv[2], 0, WINDOW_MAX_X);
    int y = getint(argv[4], 0, WINDOW_MAX_Y);
    int width = getint(argv[6], 0, WINDOW_MAX_WIDTH);
    int height = getint(argv[8], 0, WINDOW_MAX_HEIGHT);
    if (width < 1 || height < 1) return kOk;

    MmSurface *src_surface = graphics_current;
    if (has_arg(10)) {
        MmSurfaceId src_id = -1;
        result = parse_read_page(argv[10], &src_id);
        if (FAILED(result)) return result;
        src_surface = &graphics_surfaces[src_id];
    }

    if (src_surface->type == kGraphicsNone) return kGraphicsInvalidReadSurface;

    // Not sure we should be doing this.
    if (x < 0) { width += x; x = 0; }
    if (y < 0) { height += y; y = 0; }
    if (x + width > src_surface->width) width = src_surface->width - x;
    if (y + height > src_surface->height) height = src_surface->height - y;
    if (width < 1 || height < 1
            || x < 0 || x + width > src_surface->width
            || y < 0 || y + height > src_surface->height) return kOk;

    // Create destination sprite/buffer if it does not exist.
    if (dst_surface->type == kGraphicsNone) {
        MmResult result = sprite
                ? graphics_sprite_create(dst_surface->id, width, height)
                : graphics_buffer_create(dst_surface->id, width, height);
        if (FAILED(result)) return result;
    } else {
        if (dst_surface->width != width || dst_surface->height != height) {
            return kGraphicsSurfaceSizeMismatch;
        }
    }

    return graphics_blit(x, y, 0, 0, width, height, src_surface, dst_surface, 0x0, RGB_BLACK);
}

/**
 * BLIT WRITE [#]src_id, x, y [, flags]
 *
 * 'flags' is a Bitwise AND of:
 *     0x01 = mirrored left to right.
 *     0x02 = mirrored top to bottom.
 *     0x04 = don't copy transparent pixels
 * Where 0x0 is the default when unspecified.
 *
 * @param  sprite  If true then parse as SPRITE WRITE instead of BLIT WRITE,
 *                 in which case the default for flags is 0x04.
 */
MmResult cmd_blit_write(const char *p, bool sprite) {
    getargs(&p, 7, ",");
    if (argc != 5 && argc != 7) return kArgumentCount;

    MmSurface *src_surface = NULL;
    {
        MmSurfaceId src_id = -1;
        MmResult result = sprite
                ? parse_sprite_id(argv[0], true, &src_id)
                : parse_blit_id(argv[0], true, &src_id);
        if (result == kGraphicsInvalidSurface) result = kGraphicsInvalidReadSurface;
        if (FAILED(result)) return result;
        src_surface = &graphics_surfaces[src_id];
    }

    const int x = getint(argv[2], -src_surface->width + 1, WINDOW_MAX_X);
    const int y = getint(argv[4], -src_surface->height + 1, WINDOW_MAX_Y);

    const unsigned flags = has_arg(6)
            ? getint(argv[6], 0, 7)
            : sprite ? 0x04 : 0x0;

    MmSurface *dst_surface = graphics_current;
    const MmGraphicsColour transparent_colour = sprite ? sprite_transparent_colour : RGB_BLACK;
    return graphics_blit(0, 0, x, y, src_surface->width, src_surface->height, src_surface,
                         dst_surface, flags, transparent_colour);
}

/**
 * BLIT x1, y1, x2, y2, width, height [, src_id] [, flags]
 *  - <src_id> parameter unsupported on PicoMite{VGA}.
 *  - <flags> parameter unsupported on PicoMite{VGA}.
 */
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
        if (mmb_options.simulate == kSimulateGameMite || mmb_options.simulate == kSimulatePicoMiteVga) {
            return kUnsupportedParameterOnCurrentDevice;
        }
        MmSurfaceId src_id = -1;
        MmResult result = parse_read_page(argv[12], &src_id);
        if (FAILED(result)) return result;
        src_surface = &graphics_surfaces[src_id];
    }
    const unsigned flags = (argc == 15) ? getint(argv[14], 0, 7) : 0x0;
    MmSurface* dst_surface = graphics_current;
    return graphics_blit(x1, y1, x2, y2, width, height, src_surface, dst_surface, flags, RGB_BLACK);
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
        result = cmd_blit_read(p, false);
    } else if ((p = checkstring(cmdline, "WRITE"))) {
        result = cmd_blit_write(p, false);
    } else {
        result = cmd_blit_default(cmdline);
    }
    ERROR_ON_FAILURE(result);
}
