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

static void cmd_blit_close(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT CLOSE");
}

static void cmd_blit_close_all(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT CLOSE ALL");
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

static void cmd_blit_move(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT MOVE");
}

static void cmd_blit_next(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT NEXT");
}

static void cmd_blit_nointerrupt(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT NOINTERRUPT");
}

static void cmd_blit_read(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT READ");
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

static void cmd_blit_write(const char *p) {
    ERROR_UNIMPLEMENTED("BLIT WRITE");
}

/** BLIT x1, y1, x2, y2, w, h [,surface] [,orientation] */
static void cmd_blit_default(const char *p) {
    if (!graphics_current) error_throw_ex(kGraphicsSurfaceNotFound, "Write surface does not exist");

    getargs(&p, 15, ",");
    if (argc < 11) ERROR_ARGUMENT_COUNT;
    MMINTEGER x1 = getinteger(argv[0]);
    MMINTEGER y1 = getinteger(argv[2]);
    MMINTEGER x2 = getinteger(argv[4]);
    MMINTEGER y2 = getinteger(argv[6]);
    MMINTEGER w = getinteger(argv[8]);
    MMINTEGER h = getinteger(argv[10]);
    MMINTEGER read_id = argc >= 13 ? getint(argv[12], 0, GRAPHICS_MAX_ID) : -1;
    if (read_id != -1 && !graphics_surface_exists(read_id)) {
        error_throw_ex(kGraphicsSurfaceNotFound, "Read surface does not exist: %%", read_id);
    }

    MmSurface* read_surface = &graphics_surfaces[read_id];
    MmSurface* write_surface = graphics_current;
    GRAPHICS_CHECK_RESULT(graphics_blit(x1, y1, x2, y2, w, h, read_surface, write_surface, 0x0));
}

void cmd_blit(void) {
    const char *p;
    if ((p = checkstring(cmdline, "CLOSE"))) {
        cmd_blit_close(p);
    } else if ((p = checkstring(cmdline, "CLOSE ALL"))) {
        cmd_blit_close_all(p);
    } else if ((p = checkstring(cmdline, "COPY"))) {
        cmd_blit_copy(p);
    } else if ((p = checkstring(cmdline, "HIDE"))) {
        cmd_blit_hide(p);
    } else if ((p = checkstring(cmdline, "HIDE ALL"))) {
        cmd_blit_hide_all(p);
    } else if ((p = checkstring(cmdline, "HIDE SAFE"))) {
        cmd_blit_hide_safe(p);
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
    } else if ((p = checkstring(cmdline, "NEXT"))) {
        cmd_blit_next(p);
    } else if ((p = checkstring(cmdline, "NOINTERRUPT"))) {
        cmd_blit_nointerrupt(p);
    } else if ((p = checkstring(cmdline, "SCROLL"))) {
        cmd_blit_scroll(p);
    } else if ((p = checkstring(cmdline, "SCROLLR"))) {
        cmd_blit_scrollr(p);
    } else if ((p = checkstring(cmdline, "SHOW"))) {
        cmd_blit_show(p);
    } else if ((p = checkstring(cmdline, "SHOW SAFE"))) {
        cmd_blit_show_safe(p);
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
