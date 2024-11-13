/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_polygon.c

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

#include <assert.h>
#include <math.h>
#include <string.h>

#include "../common/error.h"
#include "../common/graphics.h"
#include "../common/mmb4l.h"
#include "../common/utility.h"

static MmResult cmd_polygon_single(int argc, char **argv) {
    int n = getint(argv[0], 0, UINT32_MAX);

    MMINTEGER *xptr = NULL; // Pointer to x-coordinates (if INTEGER).
    MMFLOAT *xfptr = NULL; // Pointer to x-coordinates (if FLOAT).
    int nx = 0; // Number of x-coordinates.
    getargaddress(argv[2], &xptr, &xfptr, &nx);

    MMINTEGER *yptr = NULL; // Pointer to y-coordinates (if INTEGER).
    MMFLOAT *yfptr = NULL; // Pointer to y-coordinates (if FLOAT).
    int ny = 0; // Number of y-coordinates.
    getargaddress(argv[4], &yptr, &yfptr, &ny);

    if (n == 0) n = nx;
    if (n == 1 || n == 2 || n > 9999) return kGraphicsInvalidVertices;
    if (nx < n) return mmresult_ex(kInvalidArray, "X Dimensions %d", nx);
    if (ny < n) return mmresult_ex(kInvalidArray, "Y Dimensions %d", ny);

    MmGraphicsColour c =
        (argc > 5 && *argv[6]) ? getint(argv[6], RGB_BLACK, RGB_WHITE) : graphics_fcolour;
    MmGraphicsColour f =
        (argc > 7 && *argv[8]) ? getint(argv[8], -1, RGB_WHITE) : -1;

    float *px = (float *) GetTempMemory((n + 1) * sizeof(float));
    for (int i = 0; i < n; ++i) {
        px[i] = xfptr ? (float) xfptr[i] : (float) xptr[i];
    }

    float *py = (float *) GetTempMemory((n + 1) * sizeof(float));
    for (int i = 0; i < n; ++i) {
        py[i] = yfptr ? (float) yfptr[i] : (float) yptr[i];
    }

    if (px[n - 1] != px[0] || py[n - 1] != py[0]) {
        // Close the polygon.
        px[n] = px[0];
        py[n] = py[0];
        n++;
    }

    return (f == -1)
        ? graphics_draw_polyline(graphics_current, n, px, py, c)
        : graphics_draw_filled_polygon(graphics_current, n, px, py, c, f);
}

static MmResult cmd_polygon_multiple(int argc, char **argv) {
    MMINTEGER *pn = NULL; // Pointer to number of vertices (if INTEGER).
    MMFLOAT *pfn = NULL; // Pointer to number of vertices (if FLOAT).
    int count = 0; // Number of polygons.
    getargaddress(argv[0], &pn, &pfn, &count);

    // Validate the number of vertices for each polygon.
    int total_n = 0;
    int max_n = 0;
    for (int i = 0; i < count; ++i) {
        const int n = pfn ? (int) pfn[i] : (int) pn[i];
        if (n == 1 || n == 2 || n > 9999) {
            return mmresult_ex(kGraphicsInvalidVertices,
                               "Invalid number of vertices, polygon %i", i);
        }
        total_n += n;
        max_n = max(max_n, n);
    }

    // Read and validate the x-coordinates.
    MMINTEGER *xptr = NULL; // Pointer to x-coordinates (if INTEGER).
    MMFLOAT *xfptr = NULL; // Pointer to x-coordinates (if FLOAT).
    int nx = 0; // Number of x-coordinates.
    getargaddress(argv[2], &xptr, &xfptr, &nx);
    if (nx < total_n) return mmresult_ex(kInvalidArray, "X Dimensions %d", nx);

    // Read and validate the y-coordinates.
    MMINTEGER *yptr = NULL; // Pointer to y-coordinates (if INTEGER).
    MMFLOAT *yfptr = NULL; // Pointer to y-coordinates (if FLOAT).
    int ny = 0; // Number of y-coordinates.
    getargaddress(argv[4], &yptr, &yfptr, &ny);
    if (ny < total_n) return mmresult_ex(kInvalidArray, "Y Dimensions %d", nx);

    // Read and validate the line colour.
    MmGraphicsColour c = graphics_fcolour;
    MmGraphicsColour *pc = NULL;  // Pointer to line colours (if INTEGER).
    MMFLOAT *pfc = NULL; // Pointer to line colours (if FLOAT).
    int nc = 0; // Number of line colours.
    if (argc > 5 && *argv[6]) {
      getargaddress(argv[6], &pc, &pfc, &nc);
      if (nc == 1) {
        c = getint(argv[6], RGB_BLACK, RGB_WHITE);
      } else {
        if (nc < count) return mmresult_ex(kInvalidArray, "Line colour Dimensions %d", nc);
        for (int i = 0; i < nc; ++i) {
            c = pfc ? (MmGraphicsColour) pfc[i] : (MmGraphicsColour) pc[i];
            if (c < RGB_BLACK || c > RGB_WHITE)
                return mmresult_ex(kInvalidValue, "%ld is invalid (valid is %d to %d)", c,
                                   RGB_BLACK, RGB_WHITE);

            }
        }
    }

    // Read and validate the fill colour.
    MmGraphicsColour f = -1; // Scalar fill colour.
    MmGraphicsColour *pf = NULL; // Pointer to fill colours (if INTEGER).
    MMFLOAT *pff = NULL; // Pointer to fill colours (if FLOAT).
    int nf = 0; // Number of fill colours.
    if (argc > 7 && *argv[8]) {
      getargaddress(argv[8], &pf, &pff, &nf);
      if (nf == 1) {
        f = getint(argv[8], -1, RGB_WHITE);
      } else {
        if (nf < count) return mmresult_ex(kInvalidArray, "Fill colour Dimensions %d", nf);
        for (int i = 0; i < nf; ++i) {
            f = pff ? (MmGraphicsColour) pff[i] : (MmGraphicsColour) pf[i];
            if (f < -1 || f > RGB_WHITE)
                return mmresult_ex(kInvalidValue, "%ld is invalid (valid is -1 to %d)", f,
                                   RGB_WHITE);

            }
        }
    }

    // Draw each polygon.
    MmResult result = kOk;
    float *px = GetTempMemory((max_n + 1) * sizeof(float));
    float *py = GetTempMemory((max_n + 1) * sizeof(float));
    int start = 0;
    for (int i = 0; i < count && SUCCEEDED(result); ++i) {
        int n = pfn ? (int) pfn[i] : (int) pn[i];
        for (int j = 0; j < n; ++j) {
            px[j] = xfptr ? (float) xfptr[start + j] : (float) xptr[start + j];
            py[j] = yfptr ? (float) yfptr[start + j] : (float) yptr[start + j];
        }

        start += pn[i];

        if (px[n - 1] != px[0] || py[n - 1] != py[0]) {
            // Close the polygon.
            px[n] = px[0];
            py[n] = py[0];
            n++;
        }

        c = pc ? (MmGraphicsColour) pc[i] : (pfc ? (MmGraphicsColour) pfc[i] : c);
        f = pf ? (MmGraphicsColour) pf[i] : (pff ? (MmGraphicsColour) pff[i] : f);

        result = (f == -1)
            ? graphics_draw_polyline(graphics_current, n, px, py, c)
            : graphics_draw_filled_polygon(graphics_current, n, px, py, c, f);
    }

    return result;
}

/**
 * POLYGON n,   x(), y() [, bordercolour]   [, fillcolour]
 * POLYGON n(), x(), y() [, bordercolour]   [, fillcolour]
 * POLYGON n(), x(), y() [, bordercolour()] [, fillcolour()]
 */
void cmd_polygon(void) {
    if (!graphics_current) error_throw(kGraphicsInvalidWriteSurface);

    getargs(&cmdline, 9, ",");

    MMINTEGER *pn = NULL;
    MMFLOAT *pfn = NULL;
    int count = 0; // Number of polygons.
    getargaddress(argv[0], &pn, &pfn, &count);

    MmResult result = (count == 1)
        ? cmd_polygon_single(argc, argv)
        : cmd_polygon_multiple(argc, argv);
    ERROR_ON_FAILURE(result);
}
