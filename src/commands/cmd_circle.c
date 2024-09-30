/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_circle.c

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

// #include <string.h>

#include "../common/error.h"
#include "../common/graphics.h"
#include "../common/mmb4l.h"
#include "../common/utility.h"

#if 0
static void getcoord(const char *p, int *x, int *y) {
    char b[STRINGSIZE];
    char savechar;
    char *tp = getclosebracket(p);
    savechar = *tp;
    *tp = 0;         // remove the closing brackets
    strcpy(b, p);    // copy the coordinates to the temp buffer
    *tp = savechar;  // put back the closing bracket
    const char *ttp = b + 1;
    // kludge (todo: fix this)
    {
        getargs(&ttp, 3, ",");  // this is a macro and must be the first executable
                                // stmt in a block
        if (argc != 3) ERROR_SYNTAX;
        *x = getinteger(argv[0]);
        *y = getinteger(argv[2]);
    }
}
#endif

static void cmd_circle_cmm1(void) {
    ERROR_UNIMPLEMENTED("cmd_circle_cmm1");
#if 0
    int x, y, radius, colour, fill;
    float aspect;
    getargs(&cmdline, 9, ",");
    if (argc % 2 == 0 || argc < 3) ERROR_SYNTAX;
    if (*argv[0] != '(') ERROR_SYNTAX;  // Expected opening bracket.
    if (toupper(*argv[argc - 1]) == 'F') {
        argc -= 2;
        fill = true;
    } else
        fill = false;
    getcoord((char *)argv[0], &x, &y);
    radius = getinteger(argv[2]);
    if (radius == 0) return;  // nothing to draw
    if (radius < 1) ERROR_INVALID_ARGUMENT;
    if (argc > 3 && *argv[4])
        colour = getColour((char *)argv[4], 0);
    else
        colour = graphics_current->fcolour;

    if (argc > 5 && *argv[6])
        aspect = getnumber(argv[6]);
    else
        aspect = 1;

    graphics_draw_circle(x, y, radius, (fill ? 0 : 1), colour, (fill ? colour : -1), aspect);
    // lastx = x;
    // lasty = y;
#endif
}

/**
 * CIRCLE x, y, radius [, line_width] [, aspect_ratio] [, colour] [, fill]
 * CIRCLE x(), y(), radius() [, line_width] [, aspect_ratio] [, colour] [, fill]
 *   - line_width, aspect_ratio, colour and fill may optionally be arrays.
 */
static void cmd_circle_default(void) {
    int x, y, r, n = 0, nc = 0, nw = 0, nf = 0, na = 0;
    MMFLOAT a;
    MMINTEGER *xptr, *yptr, *rptr, *fptr, *wptr, *cptr, *aptr;
    MMFLOAT *xfptr, *yfptr, *rfptr, *ffptr, *wfptr, *cfptr, *afptr;
    getargs(&cmdline, 13, ",");
    if (!(argc & 1) || argc < 5) ERROR_ARGUMENT_COUNT;
    getargaddress(argv[0], &xptr, &xfptr, &n);
    if (n != 1) {
        getargaddress(argv[2], &yptr, &yfptr, &n);
        getargaddress(argv[4], &rptr, &rfptr, &n);
    }
    if (n == 1) {
        int w = 1;
        MmGraphicsColour colour = graphics_fcolour;
        MmGraphicsColour fill = -1;
        a = 1;  // setup the defaults
        x = getinteger(argv[0]);
        y = getinteger(argv[2]);
        r = getinteger(argv[4]);
        if (argc > 5 && *argv[6]) w = getint(argv[6], 0, 100);
        if (argc > 7 && *argv[8]) a = getnumber(argv[8]);
        if (argc > 9 && *argv[10]) colour = getint(argv[10], RGB_BLACK, RGB_WHITE);
        if (argc > 11) fill = getint(argv[12], -1, RGB_WHITE);
        ERROR_ON_FAILURE(graphics_draw_circle(graphics_current, x, y, r, w, colour, fill, a));
    } else {
        int w = 1;
        MmGraphicsColour colour = graphics_fcolour;
        MmGraphicsColour fill = -1;
        a = 1;  // setup the defaults
        if (argc > 5 && *argv[6]) {
            getargaddress(argv[6], &wptr, &wfptr, &nw);
            if (nw == 1) {
                w = getint(argv[6], 0, 100);
            } else if (nw > 1) {
                if (nw > 1 && nw < n) n = nw;  // adjust the dimensionality
                for (int i = 0; i < nw; i++) {
                    w = (wfptr == NULL ? wptr[i] : (int) wfptr[i]);
                    if (w < 0 || w > 100) ERROR_INVALID_INTEGER_RANGE(w, 0, 100);
                }
            }
        }
        if (argc > 7 && *argv[8]) {
            getargaddress(argv[8], &aptr, &afptr, &na);
            if (na == 1) a = getnumber(argv[8]);
            if (na > 1 && na < n) n = na;  // adjust the dimensionality
        }
        if (argc > 9 && *argv[10]) {
            getargaddress(argv[10], &cptr, &cfptr, &nc);
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
        if (argc > 11) {
            getargaddress(argv[12], &fptr, &ffptr, &nf);
            if (nf == 1) {
                fill = getint(argv[12], -1, RGB_WHITE);
            } else if (nf > 1) {
                if (nf > 1 && nf < n) n = nf;  // adjust the dimensionality
                for (int i = 0; i < nf; i++) {
                    fill = (ffptr == NULL ? fptr[i] : (MmGraphicsColour) ffptr[i]);
                    if (fill < RGB_BLACK || fill > RGB_WHITE) {
                        ERROR_INVALID_INTEGER_RANGE(fill, RGB_BLACK, RGB_WHITE);
                    }
                }
            }
        }
        for (int i = 0; i < n; i++) {
            x = (xfptr == NULL ? xptr[i] : (int)xfptr[i]);
            y = (yfptr == NULL ? yptr[i] : (int)yfptr[i]);
            r = (rfptr == NULL ? rptr[i] : (int)rfptr[i]) - 1;
            if (nw > 1) w = (wfptr == NULL ? wptr[i] : (int) wfptr[i]);
            if (nc > 1) colour = (cfptr == NULL ? cptr[i] : (MmGraphicsColour) cfptr[i]);
            if (nf > 1) fill = (ffptr == NULL ? fptr[i] : (MmGraphicsColour) ffptr[i]);
            if (na > 1) a = (afptr == NULL ? (MMFLOAT)aptr[i] : afptr[i]);
            ERROR_ON_FAILURE(graphics_draw_circle(graphics_current, x, y, r, w, colour, fill, a));
        }
    }
}

void cmd_circle(void) {
    if (!graphics_current) error_throw(kGraphicsInvalidWriteSurface);

    if (false /*CMM1*/) {
        cmd_circle_cmm1();
    } else {
        cmd_circle_default();
    }
}
