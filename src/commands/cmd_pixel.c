/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_pixel.c

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

#include <string.h>

#include "../common/error.h"
#include "../common/graphics.h"
#include "../common/mmb4l.h"

static MmResult cmd_pixel_cmm1(const char *p) {
    ERROR_UNIMPLEMENTED("cmd_pixel_cmm1");
    return kUnimplemented;
#if 0
    int x, y, value;
    getcoord((char *)cmdline, &x, &y);
    cmdline = getclosebracket(cmdline) + 1;
    while (*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
    if (!*cmdline) error("Invalid syntax");
    ++cmdline;
    if (!*cmdline) error("Invalid syntax");
    value = getColour((char *)cmdline, 0);
    DrawPixel(x, y, value);
    lastx = x;
    lasty = y;
#endif
}

/**
 * PIXEL x, y [, colour]
 * PIXEL x(), y() [, colour]
 *   - colour may optionally be an array.
 */
static MmResult cmd_pixel_default(const char *p) {
    int n = 0, nc = 0;
    MMINTEGER *x1ptr, *y1ptr, *cptr;
    MMFLOAT *x1fptr, *y1fptr, *cfptr;

    getargs(&cmdline, 5, ",");
    if (argc != 3 && argc != 5) return kArgumentCount;
    getargaddress(argv[0], &x1ptr, &x1fptr, &n);
    if (n != 1) getargaddress(argv[2], &y1ptr, &y1fptr, &n);

    MmResult result = kOk;
    if (n == 1) {  // just a single point
        const int x = getinteger(argv[0]);
        const int y = getinteger(argv[2]);
        MmGraphicsColour colour = (argc == 5) ? getint(argv[4], RGB_BLACK, RGB_WHITE) : graphics_fcolour;
        result = graphics_draw_pixel(graphics_current, x, y, colour);
    } else {
        MmGraphicsColour colour = graphics_fcolour;
        if (argc == 5) {
            getargaddress(argv[4], &cptr, &cfptr, &nc);
            if (nc == 1)
                colour = getint(argv[4], RGB_BLACK, RGB_WHITE);
            else if (nc > 1) {
                if (nc < n) n = nc;  // adjust the dimensionality
                for (int i = 0; i < nc; i++) {
                    colour = (cfptr == NULL ? cptr[i] : (MmGraphicsColour) cfptr[i]);
                    if (colour < RGB_BLACK || colour > RGB_WHITE)
                        ERROR_INVALID_INTEGER_RANGE(c, RGB_BLACK, RGB_WHITE);
                }
            }
        }
        for (int i = 0; SUCCEEDED(result) && i < n; ++i) {
            const int x = (x1fptr == NULL ? x1ptr[i] : (int)x1fptr[i]);
            const int y = (y1fptr == NULL ? y1ptr[i] : (int)y1fptr[i]);
            if (nc > 1) colour = (cfptr == NULL ? cptr[i] : (MmGraphicsColour) cfptr[i]);
            result = graphics_draw_pixel(graphics_current, x, y, colour);
        }
    }

    return result;
}

void cmd_pixel(void) {
    if (!graphics_current) error_throw(kGraphicsSurfaceNotFound);
    MmResult result = kOk;
    if (false /* CMM1 */) {
        result = cmd_pixel_cmm1(cmdline);
    } else {
        result = cmd_pixel_default(cmdline);
    }
    ERROR_ON_FAILURE(result);
}
