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

static void cmd_pixel_cmm1(const char *p) {
    ERROR_UNIMPLEMENTED("cmd_pixel_cmm1");
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

static void cmd_pixel_default(const char *p) {
    int x1, y1, c = 0, n = 0, i, nc = 0;
    long long int *x1ptr, *y1ptr, *cptr;
    MMFLOAT *x1fptr, *y1fptr, *cfptr;
    getargs(&cmdline, 5, ",");
    if (argc != 3 && argc != 5) ERROR_ARGUMENT_COUNT;
    getargaddress(argv[0], &x1ptr, &x1fptr, &n);
    if (n != 1) getargaddress(argv[2], &y1ptr, &y1fptr, &n);
    if (n == 1) {         // just a single point
        c = graphics_current->fcolour;  // setup the defaults
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        if (argc == 5)
            c = getint(argv[4], -1, RGB_WHITE);
        else
            c = graphics_current->fcolour;
        if (c != -1)
            graphics_draw_pixel(x1, y1, c);
        else {
            graphics_current->x = x1;
            graphics_current->y = y1;
        }
    } else {
        c = graphics_current->fcolour;  // setup the defaults
        if (argc == 5) {
            getargaddress(argv[4], &cptr, &cfptr, &nc);
            if (nc == 1)
                c = getint(argv[4], 0, RGB_WHITE);
            else if (nc > 1) {
                if (nc < n) n = nc;  // adjust the dimensionality
                for (i = 0; i < nc; i++) {
                    c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                    if (c < 0 || (uint32_t)c > RGB_WHITE)
                        ERROR_INVALID_INTEGER_RANGE(c, 0, RGB_WHITE);
                }
            }
        }
        for (i = 0; i < n; i++) {
            x1 = (x1fptr == NULL ? x1ptr[i] : (int)x1fptr[i]);
            y1 = (y1fptr == NULL ? y1ptr[i] : (int)y1fptr[i]);
            if (nc > 1) c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
            graphics_draw_pixel(x1, y1, c);
        }
    }
}

void cmd_pixel(void) {
    if (!graphics_current) ERROR_GRAPHICS_WINDOW_NOT_FOUND;
    if (false /* CMM1 */) {
        cmd_pixel_cmm1(cmdline);
    } else {
        cmd_pixel_default(cmdline);
    }
    // if (Option.Refresh) Display_Refresh();
}
