/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_rbox.c

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
#include "../common/utility.h"

/**
 * RBOX x, y, width, height [, radius] [, colour] [, fill]
 * RBOX x(), y(), width(), height() [, radius] [, colour] [, fill]
 *   - radius, colour and fill may optionally be arrays.
 */
void cmd_rbox(void) {
    if (!graphics_current) error_throw(kGraphicsInvalidWriteSurface);

    int x1, y1, wi, h, w = 0, r = 0, n = 0, i, nc = 0, nw = 0, nf = 0, hmod, wmod;
    MMINTEGER *x1ptr, *y1ptr, *wiptr, *hptr, *wptr, *cptr, *fptr;
    MMFLOAT *x1fptr, *y1fptr, *wifptr, *hfptr, *wfptr, *cfptr, *ffptr;
    getargs(&cmdline, 13, ",");
    if (!(argc & 1) || argc < 7) ERROR_ARGUMENT_COUNT;
    getargaddress(argv[0], &x1ptr, &x1fptr, &n);
    if (n != 1) {
        getargaddress(argv[2], &y1ptr, &y1fptr, &n);
        getargaddress(argv[4], &wiptr, &wifptr, &n);
        getargaddress(argv[6], &hptr, &hfptr, &n);
    }
    if (n == 1) {
        MmGraphicsColour c = graphics_fcolour;
        MmGraphicsColour f = -1;
        w = 1;
        r = 10;
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        w = getinteger(argv[4]);
        h = getinteger(argv[6]);
        wmod = (w > 0 ? -1 : 1);
        hmod = (h > 0 ? -1 : 1);
        if (argc > 7 && *argv[8]) r = getint(argv[8], 0, 100);
        if (argc > 9 && *argv[10]) c = getint(argv[10], RGB_BLACK, RGB_WHITE);
        if (argc == 13) f = getint(argv[12], -1, RGB_WHITE);
        if (w != 0 && h != 0) {
            ON_FAILURE_ERROR(graphics_draw_rbox(graphics_current, x1, y1, x1 + w + wmod,
                                                y1 + h + hmod, r, c, f));
        }
    } else {
        MmGraphicsColour c = graphics_fcolour;
        MmGraphicsColour f = -1;
        w = 1;
        if (argc > 7 && *argv[8]) {
            getargaddress(argv[8], &wptr, &wfptr, &nw);
            if (nw == 1)
                w = getint(argv[8], 0, 100);
            else if (nw > 1) {
                if (nw > 1 && nw < n) n = nw;  // adjust the dimensionality
                for (i = 0; i < nw; i++) {
                    w = (wfptr == NULL ? wptr[i] : (int)wfptr[i]);
                    if (w < 0 || w > 100) ERROR_INVALID_INTEGER_RANGE(w, 0, 100);
                }
            }
        }
        if (argc > 9 && *argv[10]) {
            getargaddress(argv[10], &cptr, &cfptr, &nc);
            if (nc == 1) {
                c = getint(argv[10], RGB_BLACK, RGB_WHITE);
            } else if (nc > 1) {
                if (nc > 1 && nc < n) n = nc;  // adjust the dimensionality
                for (i = 0; i < nc; i++) {
                    c = (cfptr == NULL ? cptr[i] : (MmGraphicsColour) cfptr[i]);
                    if (c < RGB_BLACK || c > RGB_WHITE) {
                        ERROR_INVALID_INTEGER_RANGE(c, RGB_BLACK, RGB_WHITE);
                    }
                }
            }
        }
        if (argc == 13) {
            getargaddress(argv[12], &fptr, &ffptr, &nf);
            if (nf == 1) {
                f = getint(argv[12], RGB_BLACK, RGB_WHITE);
            } else if (nf > 1) {
                if (nf > 1 && nf < n) n = nf;  // adjust the dimensionality
                for (i = 0; i < nf; i++) {
                    f = (ffptr == NULL ? fptr[i] : (MmGraphicsColour) ffptr[i]);
                    if (f < -1 || f > RGB_WHITE) ERROR_INVALID_INTEGER_RANGE(f, -1, RGB_WHITE);
                }
            }
        }
        for (i = 0; i < n; i++) {
            x1 = (x1fptr == NULL ? x1ptr[i] : (int)x1fptr[i]);
            y1 = (y1fptr == NULL ? y1ptr[i] : (int)y1fptr[i]);
            wi = (wifptr == NULL ? wiptr[i] : (int)wifptr[i]);
            h = (hfptr == NULL ? hptr[i] : (int)hfptr[i]);
            wmod = (wi > 0 ? -1 : 1);
            hmod = (h > 0 ? -1 : 1);
            if (nw > 1) w = (wfptr == NULL ? wptr[i] : (int)wfptr[i]);
            if (nc > 1) c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
            if (nf > 1) f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
            if (wi != 0 && h != 0) {
                ON_FAILURE_ERROR(graphics_draw_rbox(graphics_current, x1, y1, x1 + wi + wmod,
                                                    y1 + h + hmod, w, c, f));
            }
        }
    }
}
