/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_line.c

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

#define HRes graphics_current->width
#define VRes graphics_current->height

static MmResult cmd_line_cmm1(const char *p) {
    ERROR_UNIMPLEMENTED("cmd_line_cmm1");
    return kUnimplemented;
#if 0
    int x1, y1, x2, y2, colour, box, fill;
    getargs(&cmdline, 5, ",");

    // check if it is actually a LINE INPUT command
    if (argc < 1) ERROR_SYNTAX;
    x1 = lastx;
    y1 = lasty;
    colour = graphics_current->fcolour;
    box = false;
    fill = false;  // set the defaults
    for
        optional components p = argv[0];
    if (tokenfunction(*p) != op_subtract) {
        // the start point is specified - get the coordinates and step over to where the minus token should be
        if (*p != '(') error("Expected opening bracket");
        getcoord((char *)p, &x1, &y1);
        p = getclosebracket(p) + 1;
        skipspace(p);
    }
    if (tokenfunction(*p) != op_subtract) ERROR_SYNTAX;
    p++;
    skipspace(p);
    if (*p != '(') error("Expected opening bracket");
    getcoord((char *)p, &x2, &y2);
    if (argc > 1 && *argv[2]) {
        colour = getColour((char *)argv[2], 0);
    }
    if (argc == 5) {
        box = (strchr((char *)argv[4], 'b') != NULL || strchr((char *)argv[4], 'B') != NULL);
        fill = (strchr((char *)argv[4], 'f') != NULL || strchr((char *)argv[4], 'F') != NULL);
    }
    if (box) DrawBox(x1, y1, x2, y2, 1, colour, (fill ? colour : -1));  // draw a box
    else DrawLine(x1, y1, x2, y2, 1, colour); // or just a line

    lastx = x2;
    lasty = y2;  // save in case the user wants the last value
#endif
}

/** LINE PLOT ydata() [, nbr] [, xstart] [, xinc] [,y start] [, yinc] [,colour] */
static MmResult cmd_line_plot(const char *p) {
    int n = 0;
    MMINTEGER *y1ptr;
    MMFLOAT *y1fptr;
    int xs = 0, xinc = 1;
    int ys = 0, yinc = 1;
    getargs(&p, 13, ",");
    getargaddress(argv[0], &y1ptr, &y1fptr, &n);
    if (n == 1) ERROR_ARG_NOT_ARRAY(1);
    int nc = n;
    if (argc >= 3 && *argv[2]) nc = getint(argv[2], 1, HRes - 1);
    if (nc > n) nc = n;
    if (argc >= 5 && *argv[4]) xs = getint(argv[4], 0, HRes - 1);
    if (argc >= 7 && *argv[6]) xinc = getint(argv[6], 1, HRes - 1);
    if (argc >= 9 && *argv[8]) ys = getint(argv[8], 0, n - 1);
    if (argc >= 11 && *argv[10]) yinc = getint(argv[10], 1, n - 1);
    uint32_t w = 1;  // setup the defaults
    MmGraphicsColour colour = (argc == 13) ? getint(argv[12], RGB_BLACK, RGB_WHITE) : graphics_fcolour;

    MmResult result = kOk;
    for (int i = 0; SUCCEEDED(result) && i < nc - 1; i += xinc) {
        int y = ys + yinc * (i / xinc);
        int x1 = xs + i;
        int y1 = (y1fptr == NULL ? y1ptr[y] : (int)y1fptr[y]);
        if (y1 < 0) y1 = 0;
        if (y1 >= VRes) y1 = VRes - 1;
        int x2 = xs + (i + xinc);
        int y2 = (y1fptr == NULL ? y1ptr[y + yinc] : (int)y1fptr[y + yinc]);
        if (x1 >= HRes) break;  // can only get worse so stop now
        if (x2 >= HRes) x2 = HRes - 1;
        if (y2 < 0) y2 = 0;
        if (y2 >= VRes) y2 = VRes - 1;
        result = graphics_draw_line(graphics_current, x1, y1, x2, y2, w, colour);
    }
    return result;
}

/** LINE GRAPH x(), y() [, colour] */
static MmResult cmd_line_graph(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 3 && argc != 5) return kArgumentCount;

    MMINTEGER *px = NULL; // Pointer to x-coordinates (if INTEGER).
    MMFLOAT *pfx = NULL; // Pointer to x-coordinates (if FLOAT).
    int nx = 0; // Number of x-coordinates.
    getargaddress(argv[0], &px, &pfx, &nx);
    if (nx < 3) MMRESULT_RETURN_EX(kInvalidArray, "X Dimensions %d", nx);

    MMINTEGER *py = NULL; // Pointer to y-coordinates (if INTEGER).
    MMFLOAT *pfy = NULL; // Pointer to y-coordinates (if FLOAT).
    int ny = 0; // Number of y-coordinates.
    getargaddress(argv[2], &py, &pfy, &ny);
    if (ny != nx) MMRESULT_RETURN_EX(kInvalidArray, "Y Dimensions %d", ny);

    MmGraphicsColour c = (argc == 5) ? getint(argv[4], RGB_BLACK, RGB_WHITE) : graphics_fcolour;

    MmResult result = kOk;
    for (int i = 0; SUCCEEDED(result) && i < nx - 1; ++i) {
        result = graphics_draw_line(
            graphics_current,
            pfx ? (int) pfx[i] : (int) px[i],
            pfy ? (int) pfy[i] : (int) py[i],
            pfx ? (int) pfx[i + 1] : (int) px[i + 1],
            pfx ? (int) pfy[i + 1] : (int) py[i + 1], 1, c);
    }
    return result;
}

/** LINE AA x1, y1, x2, y2 [, width [, colour]] */
static MmResult cmd_line_aa(const char *p) {
    MMFLOAT x1, y1, x2, y2;
    getargs(&p, 11, ",");
    x1 = getnumber(argv[0]);
    y1 = getnumber(argv[2]);
    x2 = getnumber(argv[4]);
    y2 = getnumber(argv[6]);
    uint32_t w = (argc > 7 && *argv[8]) ? getint(argv[8], 1, 100) : 1;
    MmGraphicsColour colour = (argc == 11) ? getint(argv[10], RGB_BLACK, RGB_WHITE) : graphics_fcolour;
    return graphics_draw_aa_line(graphics_current, x1, y1, x2, y2, colour, w);
}

/** LINE x1, y1, x2, y2 [, width [, colour]] */
static MmResult cmd_line_default(const char *p) {
    int n = 0;
    MMINTEGER *x1ptr, *y1ptr, *x2ptr, *y2ptr;
    MMFLOAT *x1fptr, *y1fptr, *x2fptr, *y2fptr;

    getargs(&cmdline, 11, ",");
    if (!(argc & 1) || argc < 3) return kArgumentCount;
    getargaddress(argv[0], &x1ptr, &x1fptr, &n);
    if (n != 1) {
        if (argc < 7) return kArgumentCount;
        getargaddress(argv[2], &y1ptr, &y1fptr, &n);
        getargaddress(argv[4], &x2ptr, &x2fptr, &n);
        getargaddress(argv[6], &y2ptr, &y2fptr, &n);
    }

    if (n == 1) {
        int x1 = getinteger(argv[0]);
        int y1 = getinteger(argv[2]);
        int x2 = getinteger(argv[4]);
        int y2 = getinteger(argv[6]);
        uint32_t w = (argc > 7 && *argv[8]) ? getint(argv[8], 1, 100) : 1;
        MmGraphicsColour colour = (argc == 11) ? getint(argv[10], RGB_BLACK, RGB_WHITE) : graphics_fcolour;
        return graphics_draw_line(graphics_current, x1, y1, x2, y2, w, colour);
    } else {
        int nc = 0, nw = 0;
        MMINTEGER *wptr, *cptr;
        int32_t w = 1;
        MmGraphicsColour colour = graphics_fcolour;
        MMFLOAT *wfptr, *cfptr;
        if (argc > 7 && *argv[8]) {
            getargaddress(argv[8], &wptr, &wfptr, &nw);
            if (nw == 1) {
                w = getint(argv[8], 0, 100);
            } else if (nw > 1) {
                if (nw > 1 && nw < n) n = nw;  // adjust the dimensionality
                for (int i = 0; i < nw; i++) {
                    w = (wfptr == NULL ? wptr[i] : (int)wfptr[i]);
                    if (w < 0 || w > 100) ERROR_INVALID_INTEGER_RANGE(w, 0, 100);
                }
            }
        }
        if (argc == 11) {
            getargaddress(argv[10], &cptr, &cfptr, &nc);
            if (nc == 1) {
                colour = getint(argv[10], RGB_BLACK, RGB_WHITE);
            } else if (nc > 1) {
                if (nc > 1 && nc < n) n = nc;  // adjust the dimensionality
                for (int i = 0; i < nc; i++) {
                    colour = cfptr ? (MmGraphicsColour) cfptr[i] : cptr[i];
                    if (colour < RGB_BLACK || colour > RGB_WHITE) {
                        ERROR_INVALID_INTEGER_RANGE(colour, RGB_BLACK, RGB_WHITE);
                    }
                }
            }
        }
        int x1, y1, x2, y2;
        MmResult result = kOk;
        for (int i = 0; SUCCEEDED(result) && i < n; i++) {
            x1 = x1fptr ? (int) x1fptr[i] : x1ptr[i];
            y1 = y1fptr ? (int) y1fptr[i] : y1ptr[i];
            x2 = x2fptr ? (int) x2fptr[i] : x2ptr[i];
            y2 = y2fptr ? (int) y2fptr[i] : y2ptr[i];
            if (nw > 1) w = wfptr ? (int32_t) wfptr[i] : wptr[i];
            if (nc > 1) colour = cfptr ? (MmGraphicsColour) cfptr[i] : cptr[i];
            result = graphics_draw_line(graphics_current, x1, y1, x2, y2, w, colour);
        }
        return result;
    }
}

void cmd_line(void) {
    if (!graphics_current) error_throw(kGraphicsSurfaceNotFound);
    MmResult result = kOk;
    const char *p;
    if (false /* CMM1 */) {
        result = cmd_line_cmm1(cmdline);
    } else if ((p = checkstring(cmdline, "PLOT"))) {
        result = cmd_line_plot(p);
    } else if ((p = checkstring(cmdline, "GRAPH"))) {
        result = cmd_line_graph(p);
    } else if ((p = checkstring(cmdline, "AA"))) {
        result = cmd_line_aa(p);
    } else {
        result = cmd_line_default(cmdline);
    }
    ERROR_ON_FAILURE(result);
}
