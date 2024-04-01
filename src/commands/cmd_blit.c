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

static MmResult cmd_blit_close(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT CLOSE");
    return kUnimplemented;
}

static MmResult cmd_blit_close_all(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT CLOSE ALL");
    return kUnimplemented;
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

static MmResult cmd_blit_read(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT READ");
    return kUnimplemented;
}

static MmResult cmd_blit_write(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT WRITE");
    return kUnimplemented;
}

/** BLIT x1, y1, x2, y2, width, height [, surface] [, orientation] */
static MmResult cmd_blit_default(const char *p) {
    if (!graphics_current) return kGraphicsInvalidWriteSurface;

    getargs(&p, 15, ",");
    if (argc < 11) return kArgumentCount;
    int x1 = getinteger(argv[0]);
    int y1 = getinteger(argv[2]);
    int x2 = getinteger(argv[4]);
    int y2 = getinteger(argv[6]);
    int w = getinteger(argv[8]);
    int h = getinteger(argv[10]);
    MmSurfaceId read_id = argc >= 13 ? getint(argv[12], 0, GRAPHICS_MAX_ID) : -1;
    if (read_id != -1 && !graphics_surface_exists(read_id)) return kGraphicsInvalidReadSurface;

    MmSurface* read_surface = &graphics_surfaces[read_id];
    MmSurface* write_surface = graphics_current;
    return graphics_blit(x1, y1, x2, y2, w, h, read_surface, write_surface, 0x0);
}

void cmd_blit(void) {
    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(cmdline, "CLOSE_ALL"))) {
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
