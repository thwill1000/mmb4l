/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_load.c

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

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/graphics.h"
#include "../common/program.h"
#include "../common/utility.h"

static void cmd_load_bmp(const char *p) {
    ERROR_UNIMPLEMENTED("LOAD {BMP|IMAGE}");
}

static void cmd_load_data(const char *p) {
    ERROR_UNIMPLEMENTED("LOAD DATA");
}

static void cmd_load_font(const char *p) {
    ERROR_UNIMPLEMENTED("LOAD FONT");
}

static void cmd_load_gif(const char *p) {
    ERROR_UNIMPLEMENTED("LOAD GIF");
}

static void cmd_load_jpg(const char *p) {
    ERROR_UNIMPLEMENTED("LOAD JPG");
}

/** LOAD PNG file$ [, x [, y [, transparency_cut_off]]] */
static void cmd_load_png(const char *p) {
    if (!graphics_current) error_throw(kGraphicsSurfaceNotFound);

	getargs(&p, 7, ",");
    if (argc == 0 || argc > 7) ERROR_ARGUMENT_COUNT;
    char *filename = getCstring(argv[0]);
    int xOrigin = (argc >= 3 && *argv[2]) ? getinteger(argv[2]) : 0;
    int yOrigin = (argc >= 5 && *argv[4]) ? getinteger(argv[4]) : 0;
    int transparent = (argc == 7) ? getint(argv[6], 0, 15) : 0;
    int force = 0;
    if (transparent > 4) {
        force = transparent << 4;
        transparent = 4;
    }

    MmResult result = graphics_load_png(graphics_current, filename, xOrigin, yOrigin, transparent,
                                        force);
    if (FAILED(result)) error_throw(result);
}

static void cmd_load_default(const char *p) {
    getargs(&p, 1, " ,");
    if (argc == 1) {
        const char *filename = getCstring(argv[0]);
        MmResult result = program_load_file(filename);
        if (FAILED(result)) error_throw(result);
    } else {
        ERROR_SYNTAX;
    }
}

void cmd_load(void) {
    const char *p;
    if ((p = checkstring(cmdline, "BMP"))) {
        cmd_load_bmp(p);
    } else if ((p = checkstring(cmdline, "DATA"))) {
        cmd_load_data(p);
    } else if ((p = checkstring(cmdline, "IMAGE"))) {
        cmd_load_bmp(p);
    } else if ((p = checkstring(cmdline, "JPG"))) {
        cmd_load_jpg(p);
    } else if ((p = checkstring(cmdline, "GIF"))) {
        cmd_load_gif(p);
    } else if ((p = checkstring(cmdline, "FONT"))) {
        cmd_load_font(p);
    } else if ((p = checkstring(cmdline, "PNG"))) {
        cmd_load_png(p);
    } else {
        cmd_load_default(cmdline);
    }
}
