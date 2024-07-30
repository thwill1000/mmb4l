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

#include <math.h>
#include <string.h>

#include "../common/error.h"
#include "../common/graphics.h"
#include "../common/mmb4l.h"

#define TFLOAT float
#define HRes graphics_current->width
#define VRes graphics_current->height

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} rgb_t;

typedef struct {
    TFLOAT  xpos;       // current position and heading
    TFLOAT  ypos;       // (uses floating-point numbers for
    TFLOAT  heading;    //  increased accuracy)

    rgb_t  pen_color;   // current pen color
    rgb_t  fill_color;  // current fill color
    bool   pendown;     // currently drawing?
    bool   filled;      // currently filling?
} fill_t;
fill_t main_fill;
fill_t backup_fill;

static int  main_fill_poly_vertex_count = 0;  // polygon vertex count
static TFLOAT *main_fill_polyX = NULL;  // polygon vertex x-coords
static TFLOAT *main_fill_polyY = NULL;  // polygon vertex y-coords

static void fill_set_pen_color(int red, int green, int blue) {
    main_fill.pen_color.red = red;
    main_fill.pen_color.green = green;
    main_fill.pen_color.blue = blue;
}

static void fill_set_fill_color(int red, int green, int blue) {
    main_fill.fill_color.red = red;
    main_fill.fill_color.green = green;
    main_fill.fill_color.blue = blue;
}

static void fill_begin_fill() {
    main_fill.filled = true;
    main_fill_poly_vertex_count = 0;
}

static void fill_end_fill(int count, int ystart, int yend)
{
    // based on public-domain fill algorithm in C by Darel Rex Finley, 2007
    //   from http://alienryderflex.com/polygon_fill/

    TFLOAT *nodeX=GetMemory(count * sizeof(TFLOAT));     // x-coords of polygon intercepts
    int nodes;                              // size of nodeX
    int y, i, j;                         // current pixel and loop indices
    TFLOAT temp;                            // temporary variable for sorting
    int f=(main_fill.fill_color.red<<16) | (main_fill.fill_color.green<<8) | main_fill.fill_color.blue;
    int c= (main_fill.pen_color.red<<16) | (main_fill.pen_color.green<<8) | main_fill.pen_color.blue;
    int xstart, xend;
    //  loop through the rows of the image

    for (y = ystart; y < yend; y++) {

        //  build a list of polygon intercepts on the current line
        nodes = 0;
        j = main_fill_poly_vertex_count-1;
        for (i = 0; i < main_fill_poly_vertex_count; i++) {
            if ((main_fill_polyY[i] <  (TFLOAT)y &&
                 main_fill_polyY[j] >= (TFLOAT)y) ||
                (main_fill_polyY[j] <  (TFLOAT)y &&
                 main_fill_polyY[i] >= (TFLOAT)y)) {

                // intercept found; record it
                nodeX[nodes++] = (main_fill_polyX[i] +
                        ((TFLOAT)y - main_fill_polyY[i]) /
                        (main_fill_polyY[j] - main_fill_polyY[i]) *
                        (main_fill_polyX[j] - main_fill_polyX[i]));
            }
            j = i;
        }

        //  sort the nodes via simple insertion sort
        for (i = 1; i < nodes; i++) {
            temp = nodeX[i];
            for (j = i; j > 0 && temp < nodeX[j-1]; j--) {
                nodeX[j] = nodeX[j-1];
            }
            nodeX[j] = temp;
        }

        //  fill the pixels between node pairs
        for (i = 0; i < nodes; i += 2) {
            xstart=(int)floorf(nodeX[i])+1;
            xend=(int)ceilf(nodeX[i+1])-1;
            (void) graphics_draw_line(graphics_current, xstart, y, xend, y, 1, f);
        }
    }

    main_fill.filled = false;

    // redraw polygon (filling is imperfect and can occasionally occlude sides)
    for (i = 0; i < main_fill_poly_vertex_count; i++) {
        int x0 = (int)roundf(main_fill_polyX[i]);
        int y0 = (int)roundf(main_fill_polyY[i]);
        int x1 = (int)roundf(main_fill_polyX[(i+1) %
            main_fill_poly_vertex_count]);
        int y1 = (int)roundf(main_fill_polyY[(i+1) %
            main_fill_poly_vertex_count]);
        (void) graphics_draw_line(graphics_current, x0, y0, x1, y1, 1, c);
    }
    FreeMemory((void *)nodeX);
}

static MmResult cmd_polygon_single(int argc, char **argv, bool close) {
    MMINTEGER *xptr = NULL, *yptr = NULL;
    MMINTEGER xptr2 = 0, yptr2 = 0;
    MMFLOAT *xfptr = NULL, *yfptr = NULL;
    MMFLOAT xfptr2 = 0, yfptr2 = 0;
    int c = 0, f = 0, nx = 0, ny = 0;
    int ymax = 0, ymin = 1000000;
    int xtot = getinteger(argv[0]);
    int xcount = xtot;
    if ((xcount < 3 || xcount > 9999) && xcount != 0) error_throw_legacy("Invalid number of vertices");
    getargaddress(argv[2], &xptr, &xfptr, &nx);
    if (xcount == 0) {
        xcount = xtot = nx;
    }
    if (nx < xtot) error_throw_legacy("X Dimensions %", nx);
    getargaddress(argv[4], &yptr, &yfptr, &ny);
    if (ny < xtot) error_throw_legacy("Y Dimensions %", ny);
    if (xptr)
        xptr2 = *xptr;
    else
        xfptr2 = *xfptr;
    if (yptr)
        yptr2 = *yptr;
    else
        yfptr2 = *yfptr;
    c = graphics_fcolour;  // setup the defaults
    if (argc > 5 && *argv[6]) c = getint(argv[6], 0, RGB_WHITE);
    if (argc > 7 && *argv[8]) {
        main_fill_polyX = (TFLOAT *)GetTempMemory(xtot * sizeof(TFLOAT));
        main_fill_polyY = (TFLOAT *)GetTempMemory(xtot * sizeof(TFLOAT));
        f = getint(argv[8], 0, RGB_WHITE);
        fill_set_fill_color((f >> 16) & 0xFF, (f >> 8) & 0xFF, f & 0xFF);
        fill_set_pen_color((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
        fill_begin_fill();
    }
    for (int i = 0; i < xcount - 1; i++) {
        if (argc > 7) {
            main_fill_polyX[main_fill_poly_vertex_count] =
                (xfptr == NULL ? (TFLOAT)*xptr++ : (TFLOAT)*xfptr++);
            main_fill_polyY[main_fill_poly_vertex_count] =
                (yfptr == NULL ? (TFLOAT)*yptr++ : (TFLOAT)*yfptr++);
            if (main_fill_polyY[main_fill_poly_vertex_count] > ymax)
                ymax = main_fill_polyY[main_fill_poly_vertex_count];
            if (main_fill_polyY[main_fill_poly_vertex_count] < ymin)
                ymin = main_fill_polyY[main_fill_poly_vertex_count];
            main_fill_poly_vertex_count++;
        } else {
            int x1 = (xfptr == NULL ? *xptr++ : (int)*xfptr++);
            int x2 = (xfptr == NULL ? *xptr : (int)*xfptr);
            int y1 = (yfptr == NULL ? *yptr++ : (int)*yfptr++);
            int y2 = (yfptr == NULL ? *yptr : (int)*yfptr);
            (void) graphics_draw_line(graphics_current, x1, y1, x2, y2, 1, c);
        }
    }
    if (argc > 7) {
        main_fill_polyX[main_fill_poly_vertex_count] =
            (xfptr == NULL ? (TFLOAT)*xptr++ : (TFLOAT)*xfptr++);
        main_fill_polyY[main_fill_poly_vertex_count] =
            (yfptr == NULL ? (TFLOAT)*yptr++ : (TFLOAT)*yfptr++);
        if (main_fill_polyY[main_fill_poly_vertex_count] > ymax)
            ymax = main_fill_polyY[main_fill_poly_vertex_count];
        if (main_fill_polyY[main_fill_poly_vertex_count] < ymin)
            ymin = main_fill_polyY[main_fill_poly_vertex_count];
        if (main_fill_polyY[main_fill_poly_vertex_count] != main_fill_polyY[0] ||
            main_fill_polyX[main_fill_poly_vertex_count] != main_fill_polyX[0]) {
            main_fill_poly_vertex_count++;
            main_fill_polyX[main_fill_poly_vertex_count] = main_fill_polyX[0];
            main_fill_polyY[main_fill_poly_vertex_count] = main_fill_polyY[0];
        }
        main_fill_poly_vertex_count++;
        if (main_fill_poly_vertex_count > 5) {
            fill_end_fill(xcount, ymin, ymax);
        } else if (main_fill_poly_vertex_count == 5) {
            (void) graphics_draw_triangle(graphics_current,
                                          main_fill_polyX[0], main_fill_polyY[0],
                                          main_fill_polyX[1], main_fill_polyY[1],
                                          main_fill_polyX[2], main_fill_polyY[2],
                                          f, f);
            (void) graphics_draw_triangle(graphics_current,
                                          main_fill_polyX[0], main_fill_polyY[0],
                                          main_fill_polyX[2], main_fill_polyY[2],
                                          main_fill_polyX[3], main_fill_polyY[3],
                                          f, f);
            if (f != c) {
                (void) graphics_draw_line(graphics_current,
                                          main_fill_polyX[0], main_fill_polyY[0],
                                          main_fill_polyX[1], main_fill_polyY[1],
                                          1, c);
                (void) graphics_draw_line(graphics_current,
                                          main_fill_polyX[1], main_fill_polyY[1],
                                          main_fill_polyX[2], main_fill_polyY[2],
                                          1, c);
                (void) graphics_draw_line(graphics_current,
                                          main_fill_polyX[2], main_fill_polyY[2],
                                          main_fill_polyX[3], main_fill_polyY[3],
                                          1, c);
                (void) graphics_draw_line(graphics_current,
                                          main_fill_polyX[3], main_fill_polyY[3],
                                          main_fill_polyX[4], main_fill_polyY[4],
                                          1, c);
            }
        } else {
            (void) graphics_draw_triangle(graphics_current,
                                          main_fill_polyX[0], main_fill_polyY[0],
                                          main_fill_polyX[1], main_fill_polyY[1],
                                          main_fill_polyX[2], main_fill_polyY[2],
                                          c, f);
        }
    } else if (close) {
        int x1 = (xfptr == NULL ? *xptr : (int)*xfptr);
        int x2 = (xfptr == NULL ? xptr2 : (int)xfptr2);
        int y1 = (yfptr == NULL ? *yptr : (int)*yfptr);
        int y2 = (yfptr == NULL ? yptr2 : (int)yfptr2);
        (void) graphics_draw_line(graphics_current, x1, y1, x2, y2, 1, c);
    }
    return kOk;
}

static MmResult cmd_polygon_multiple(int argc, char **argv) {
/*
    int *cc = GetTempMemory(n * sizeof(int));  // array for foreground colours
    int *ff = GetTempMemory(n * sizeof(int));  // array for background colours
    int xstart, j, xmax = 0;
    for (i = 0; i < n; i++) {
        if ((polycountf == NULL ? polycount[i] : (int)polycountf[i]) > xmax)
            xmax = (polycountf == NULL ? polycount[i] : (int)polycountf[i]);
        if (!(polycountf == NULL ? polycount[i] : (int)polycountf[i])) break;
        xtot += (polycountf == NULL ? polycount[i] : (int)polycountf[i]);
        if ((polycountf == NULL ? polycount[i] : (int)polycountf[i]) < 3 ||
            (polycountf == NULL ? polycount[i] : (int)polycountf[i]) > 9999)
            error_throw_legacy("Invalid number of vertices, polygon %", i);
    }
    n = i;
    getargaddress(argv[2], &xptr, &xfptr, &nx);
    if (nx < xtot) error_throw_legacy("X Dimensions %", nx);
    getargaddress(argv[4], &yptr, &yfptr, &ny);
    if (ny < xtot) error_throw_legacy("Y Dimensions %", ny);
    main_fill_polyX = (TFLOAT *)GetTempMemory(xmax * sizeof(TFLOAT));
    main_fill_polyY = (TFLOAT *)GetTempMemory(xmax * sizeof(TFLOAT));
    if (argc > 5 && *argv[6]) {  // foreground colour specified
        getargaddress(argv[6], &cptr, &cfptr, &nc);
        if (nc == 1)
            for (i = 0; i < n; i++) cc[i] = getint(argv[6], 0, RGB_WHITE);
        else {
            if (nc < n) error_throw_legacy("Foreground colour Dimensions");
            for (i = 0; i < n; i++) {
                cc[i] = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                if (cc[i] < 0 || cc[i] > 0xFFFFFF)
                    error_throw_legacy("% is invalid (valid is % to %)", (int)cc[i], 0, 0xFFFFFF);
            }
        }
    } else
        for (i = 0; i < n; i++) cc[i] = gui_fcolour;
    if (argc > 7) {  // background colour specified
        getargaddress(argv[8], &fptr, &ffptr, &nf);
        if (nf == 1)
            for (i = 0; i < n; i++) ff[i] = getint(argv[8], 0, RGB_WHITE);
        else {
            if (nf < n) error_throw_legacy("Background colour Dimensions");
            for (i = 0; i < n; i++) {
                ff[i] = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
                if (ff[i] < 0 || ff[i] > 0xFFFFFF)
                    error_throw_legacy("% is invalid (valid is % to %)", (int)ff[i], 0, 0xFFFFFF);
            }
        }
    }
    xstart = 0;
    for (i = 0; i < n; i++) {
        if (xptr)
            xptr2 = *xptr;
        else
            xfptr2 = *xfptr;
        if (yptr)
            yptr2 = *yptr;
        else
            yfptr2 = *yfptr;
        ymax = 0;
        ymin = 1000000;
        main_fill_poly_vertex_count = 0;
        xcount = (int)(polycountf == NULL ? polycount[i] : (int)polycountf[i]);
        if (argc > 7 && *argv[8]) {
            fill_set_pen_color((cc[i] >> 16) & 0xFF, (cc[i] >> 8) & 0xFF, cc[i] & 0xFF);
            fill_set_fill_color((ff[i] >> 16) & 0xFF, (ff[i] >> 8) & 0xFF, ff[i] & 0xFF);
            fill_begin_fill();
        }
        for (j = xstart; j < xstart + xcount - 1; j++) {
            if (argc > 7) {
                main_fill_polyX[main_fill_poly_vertex_count] =
                    (xfptr == NULL ? (TFLOAT)*xptr++ : (TFLOAT)*xfptr++);
                main_fill_polyY[main_fill_poly_vertex_count] =
                    (yfptr == NULL ? (TFLOAT)*yptr++ : (TFLOAT)*yfptr++);
                if (main_fill_polyY[main_fill_poly_vertex_count] > ymax)
                    ymax = main_fill_polyY[main_fill_poly_vertex_count];
                if (main_fill_polyY[main_fill_poly_vertex_count] < ymin)
                    ymin = main_fill_polyY[main_fill_poly_vertex_count];
                main_fill_poly_vertex_count++;
            } else {
                int x1 = (xfptr == NULL ? *xptr++ : (int)*xfptr++);
                int x2 = (xfptr == NULL ? *xptr : (int)*xfptr);
                int y1 = (yfptr == NULL ? *yptr++ : (int)*yfptr++);
                int y2 = (yfptr == NULL ? *yptr : (int)*yfptr);
                DrawLine(x1, y1, x2, y2, 1, cc[i]);
            }
        }
        if (argc > 7) {
            main_fill_polyX[main_fill_poly_vertex_count] =
                (xfptr == NULL ? (TFLOAT)*xptr++ : (TFLOAT)*xfptr++);
            main_fill_polyY[main_fill_poly_vertex_count] =
                (yfptr == NULL ? (TFLOAT)*yptr++ : (TFLOAT)*yfptr++);
            if (main_fill_polyY[main_fill_poly_vertex_count] > ymax)
                ymax = main_fill_polyY[main_fill_poly_vertex_count];
            if (main_fill_polyY[main_fill_poly_vertex_count] < ymin)
                ymin = main_fill_polyY[main_fill_poly_vertex_count];
            if (main_fill_polyY[main_fill_poly_vertex_count] != main_fill_polyY[0] ||
                main_fill_polyX[main_fill_poly_vertex_count] != main_fill_polyX[0]) {
                main_fill_poly_vertex_count++;
                main_fill_polyX[main_fill_poly_vertex_count] = main_fill_polyX[0];
                main_fill_polyY[main_fill_poly_vertex_count] = main_fill_polyY[0];
            }
            main_fill_poly_vertex_count++;
            if (main_fill_poly_vertex_count > 5) {
                fill_end_fill(xcount, ymin, ymax);
            } else if (main_fill_poly_vertex_count == 5) {
                DrawTriangle(main_fill_polyX[0], main_fill_polyY[0], main_fill_polyX[1],
                             main_fill_polyY[1], main_fill_polyX[2], main_fill_polyY[2], ff[i],
                             ff[i]);
                DrawTriangle(main_fill_polyX[0], main_fill_polyY[0], main_fill_polyX[2],
                             main_fill_polyY[2], main_fill_polyX[3], main_fill_polyY[3], ff[i],
                             ff[i]);
                if (ff[i] != cc[i]) {
                    DrawLine(main_fill_polyX[0], main_fill_polyY[0], main_fill_polyX[1],
                             main_fill_polyY[1], 1, cc[i]);
                    DrawLine(main_fill_polyX[1], main_fill_polyY[1], main_fill_polyX[2],
                             main_fill_polyY[2], 1, cc[i]);
                    DrawLine(main_fill_polyX[2], main_fill_polyY[2], main_fill_polyX[3],
                             main_fill_polyY[3], 1, cc[i]);
                    DrawLine(main_fill_polyX[3], main_fill_polyY[3], main_fill_polyX[4],
                             main_fill_polyY[4], 1, cc[i]);
                }
            } else {
                DrawTriangle(main_fill_polyX[0], main_fill_polyY[0], main_fill_polyX[1],
                             main_fill_polyY[1], main_fill_polyX[2], main_fill_polyY[2], cc[i],
                             ff[i]);
            }
        } else {
            int x1 = (xfptr == NULL ? *xptr : (int)*xfptr);
            int x2 = (xfptr == NULL ? xptr2 : (int)xfptr2);
            int y1 = (yfptr == NULL ? *yptr : (int)*yfptr);
            int y2 = (yfptr == NULL ? yptr2 : (int)yfptr2);
            DrawLine(x1, y1, x2, y2, 1, cc[i]);
            if (xfptr != NULL)
                xfptr++;
            else
                xptr++;
            if (yfptr != NULL)
                yfptr++;
            else
                yptr++;
        }

        xstart += xcount;
    }
*/
    return kOk;
}

/**
 * POLYGON n%,   x%(), y%() [, bordercolour]   [, fillcolour]
 * POLYGON n%(), x%(), y%() [, bordercolour]   [, fillcolour]
 * POLYGON n%(), x%(), y%() [, bordercolour()] [, fillcolour()]
 */
void cmd_polygon(void) {
    if (!graphics_current) error_throw(kGraphicsSurfaceNotFound);

    getargs(&cmdline, 9, ",");

    MMINTEGER *polycount = NULL;
    MMFLOAT *polycountf = NULL;
    int n = 0;
    getargaddress(argv[0], &polycount, &polycountf, &n);

    MmResult result = kOk;
    if (n == 1) {
        result = cmd_polygon_single(argc, argv, true);
    } else {
        result = cmd_polygon_multiple(argc, argv);
    }
    ERROR_ON_FAILURE(result);

    //     int xcount = 0;
    //     long long int *xptr = NULL, *yptr = NULL, xptr2 = 0, yptr2 = 0, *polycount = NULL, *cptr
    //     = NULL,
    //                   *fptr = NULL;
    //     MMFLOAT *polycountf = NULL, *cfptr = NULL, *ffptr = NULL, *xfptr = NULL, *yfptr = NULL,
    //             xfptr2 = 0, yfptr2 = 0;
    //     int i, f = 0, c, xtot = 0, ymax = 0, ymin = 1000000;
    //     int n = 0, nx = 0, ny = 0, nc = 0, nf = 0;
    // //    if (Option.DISPLAY_TYPE == 0) error_throw_legacy("Display not configured");
    //     getargaddress(argv[0], &polycount, &polycountf, &n);
    //     if (n == 1) {

    //     } else {

    //     }
}
