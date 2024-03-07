/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_triangle.c

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

static void cmd_triangle_save(const char *p) {
    ERROR_UNIMPLEMENTED("TRIANGLE SAVE");
}

static void cmd_triangle_restore(const char *p) {
    ERROR_UNIMPLEMENTED("TRIANGLE RESTORE");
}

/** TRIANGLE x1, y1, x2, y2, x3, y3 [, colour] [, fill] */
static void cmd_triangle_default(const char *p) {
    int x1, y1, x2, y2, x3, y3, n = 0;
    MMINTEGER *x3ptr, *y3ptr, *x1ptr, *y1ptr, *x2ptr, *y2ptr, *fptr, *cptr;
    MMFLOAT *x3fptr, *y3fptr, *x1fptr, *y1fptr, *x2fptr, *y2fptr, *ffptr, *cfptr;
    getargs(&cmdline, 15, ",");
    if (!(argc & 1) || argc < 11) ERROR_ARGUMENT_COUNT;
    getargaddress(argv[0], &x1ptr, &x1fptr, &n);
    if (n != 1) {
        int cn = n;
        getargaddress(argv[2], &y1ptr, &y1fptr, &n);
        if (n < cn) cn = n;
        getargaddress(argv[4], &x2ptr, &x2fptr, &n);
        if (n < cn) cn = n;
        getargaddress(argv[6], &y2ptr, &y2fptr, &n);
        if (n < cn) cn = n;
        getargaddress(argv[8], &x3ptr, &x3fptr, &n);
        if (n < cn) cn = n;
        getargaddress(argv[10], &y3ptr, &y3fptr, &n);
        if (n < cn) cn = n;
        n = cn;
    }
    MmGraphicsColour colour = graphics_fcolour;
    MmGraphicsColour fill = -1;

    // Draw a single triangle.
    if (n == 1) {
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        x2 = getinteger(argv[4]);
        y2 = getinteger(argv[6]);
        x3 = getinteger(argv[8]);
        y3 = getinteger(argv[10]);
        if (argc >= 13 && *argv[12]) colour = getint(argv[12], RGB_BLACK, RGB_WHITE);
        if (argc == 15) fill = getint(argv[14], -1, RGB_WHITE);
        ERROR_ON_FAILURE(graphics_draw_triangle(graphics_current, x1, y1, x2, y2, x3, y3, colour,
                                                fill));
        return;
    }

    // Array outline colour.
    int nc = 0;
    if (argc >= 13 && *argv[12]) {
        getargaddress(argv[12], &cptr, &cfptr, &nc);
        if (nc == 1) {
            colour = getint(argv[10], RGB_BLACK, RGB_WHITE);
        } else if (nc > 1) {
            if (nc > 1 && nc < n) n = nc;  // adjust the dimensionality
            for (int i = 0; i < nc; i++) {
                colour = (cfptr == NULL ? cptr[i] : (MmGraphicsColour) cfptr[i]);
                if (colour < RGB_BLACK || colour > RGB_WHITE) {
                    ERROR_INVALID_INTEGER_RANGE(colour, RGB_BLACK, RGB_WHITE);
                }
            }
        }
    }

    // Array fill colour.
    int nf = 0;
    if (argc == 15) {
        getargaddress(argv[14], &fptr, &ffptr, &nf);
        if (nf == 1) {
            fill = getint(argv[14], -1, RGB_WHITE);
        } else if (nf > 1) {
            if (nf > 1 && nf < n) n = nf;  // adjust the dimensionality
            for (int i = 0; i < nf; i++) {
                fill = (ffptr == NULL ? fptr[i] : (MmGraphicsColour) ffptr[i]);
                if (fill < -1 || fill > RGB_WHITE) {
                    ERROR_INVALID_INTEGER_RANGE(fill, -1, RGB_WHITE);
                }
            }
        }
    }

    for (int i = 0; i < n; i++) {
        x1 = (x1fptr == NULL ? x1ptr[i] : (int)x1fptr[i]);
        y1 = (y1fptr == NULL ? y1ptr[i] : (int)y1fptr[i]);
        x2 = (x2fptr == NULL ? x2ptr[i] : (int)x2fptr[i]);
        y2 = (y2fptr == NULL ? y2ptr[i] : (int)y2fptr[i]);
        x3 = (x3fptr == NULL ? x3ptr[i] : (int)x3fptr[i]);
        y3 = (y3fptr == NULL ? y3ptr[i] : (int)y3fptr[i]);
        if (x1 == x2 && x1 == x3 && y1 == y2 && y1 == y3 && x1 == -1 && y1 == -1) return;
        if (nc > 1) colour = (cfptr == NULL ? cptr[i] : (MmGraphicsColour) cfptr[i]);
        if (nf > 1) fill = (ffptr == NULL ? fptr[i] : (MmGraphicsColour) ffptr[i]);
        ERROR_ON_FAILURE(graphics_draw_triangle(graphics_current, x1, y1, x2, y2, x3, y3, colour,
                                                fill));
    }
}

void cmd_triangle(void) {
    if (!graphics_current) error_throw(kGraphicsSurfaceNotFound);
    const char *p;
    if ((p = checkstring(cmdline, "SAVE"))) {
        cmd_triangle_save(p);
    } else if ((p = checkstring(cmdline, "RESTORE"))) {
        cmd_triangle_restore(p);
    } else {
        cmd_triangle_default(cmdline);
    }
}
