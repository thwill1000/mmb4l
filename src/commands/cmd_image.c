/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_image.c

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

#include <stdlib.h>

#include "../common/mmb4l.h"
#include "../common/graphics.h"
#include "../common/utility.h"

static MmResult cmd_image_resize(const char *p) {
    ERROR_UNIMPLEMENTED("IMAGE RESIZE");
    return kUnimplemented;
}

/**
 * IMAGE RESIZE_FAST x, y, width, height, new_x, new_y, new_width, new_height [, src_id] [, flag]
 */
static MmResult cmd_image_resize_fast(const char *p) {
    getargs(&p, 19, DELIM_COMMA);
    if (argc < 15) RETURN_RESULT(kArgumentCount);

    const MmSurfaceId src_id = has_arg(16)
            ? getint(argv[16], 0, GRAPHICS_MAX_ID)
            : -1;
    if (src_id != -1 && !graphics_surface_exists(src_id)) return kGraphicsInvalidReadSurface;
    MmSurface *src_surface = (src_id == -1) ? graphics_current : &graphics_surfaces[src_id];
    MmSurface *dst_surface = graphics_current;

    const int sx = getint(argv[0], 0, src_surface->width - 1);
    const int sy = getint(argv[2], 0, src_surface->height - 1);
    const int sw = getint(argv[4], 1, src_surface->width - sx);
    const int sh = getint(argv[6], 1, src_surface->height - sy);
    const int dx = getint(argv[8], INT32_MIN, dst_surface->width - 1);
    const int dy = getint(argv[10], INT32_MIN, dst_surface->height - 1);
    const int dw = getint(argv[12], 1, dst_surface->width - dx);
    const int dh = getint(argv[14], 1, dst_surface->height - dy);
    const MmGraphicsColour transparent = (has_arg(18) && (getint(argv[18], 0, 1) == 1))
            ? (MmGraphicsColour) RGB_BLACK
            : NO_TRANSPARENCY;

    return graphics_blit_resize(src_surface, sx, sy, sw, sh,
                                dst_surface, dx, dy, dw, dh,
                                transparent);
}

static MmResult cmd_image_rotate(const char *p) {
    ERROR_UNIMPLEMENTED("IMAGE ROTATE");
    return kUnimplemented;
}

static MmResult cmd_image_rotate_fast(const char *p) {
    ERROR_UNIMPLEMENTED("IMAGE ROTATE FAST");
    return kUnimplemented;
}

static MmResult cmd_image_warp_h(const char *p) {
    ERROR_UNIMPLEMENTED("IMAGE WARP H");
    return kUnimplemented;
}

static MmResult cmd_image_warp_v(const char *p) {
    ERROR_UNIMPLEMENTED("IMAGE WARP V");
    return kUnimplemented;
}

void cmd_image(void) {
    if (!graphics_current) error_throw(kGraphicsInvalidWriteSurface);

    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(cmdline, "RESIZE"))) {
        result = cmd_image_resize(p);
    } else if ((p = checkstring(cmdline, "RESIZE_FAST"))) {
        result = cmd_image_resize_fast(p);
    } else if ((p = checkstring(cmdline, "RESIZE FAST"))) {
        result = cmd_image_resize_fast(p);
    } else if ((p = checkstring(cmdline, "ROTATE"))) {
        result = cmd_image_rotate(p);
    } else if ((p = checkstring(cmdline, "ROTATE_FAST"))) {
        result = cmd_image_rotate_fast(p);
    } else if ((p = checkstring(cmdline, "ROTATE FAST"))) {
        result = cmd_image_rotate_fast(p);
    } else if ((p = checkstring(cmdline, "WARP_H"))) {
        result = cmd_image_warp_h(p);
    } else if ((p = checkstring(cmdline, "WARP H"))) {
        result = cmd_image_warp_h(p);
    } else if ((p = checkstring(cmdline, "WARP_V"))) {
        result = cmd_image_warp_v(p);
    } else if ((p = checkstring(cmdline, "WARP V"))) {
        result = cmd_image_warp_v(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("IMAGE");
        result = kSyntax;
    }
    ON_FAILURE_ERROR(result);
}
