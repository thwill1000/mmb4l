/*-*****************************************************************************

MMBasic for Linux (MMB4L)

graphics.c

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

#include "error.h"
#include "graphics.h"
#include "memory.h"
#include "utility.h"

#include <stdbool.h>
#include <stdio.h>

#define SET_PIXEL(x, y, c) graphics_current->pixels[(y)*graphics_current->width + (x)] = c
#define SET_PIXEL_SAFE(x, y, c)                                           \
    if ((x) < 0 || (y) < 0 || (uint32_t)(x) >= graphics_current->width || \
        (uint32_t)(y) >= graphics_current->height) {                      \
    } else                                                                \
        SET_PIXEL(x, y, c)
#define INVALID_WINDOW(id) id<1 || id> WINDOW_MAX_ID || !windows[id - 1].pixels

MmBasicWindow* graphics_current = NULL;

static const char* NO_ERROR = "No error";

static bool graphics_initialised = false;
static MmBasicWindow windows[WINDOW_MAX_ID] = { 0 };
static uint32_t windows_count = 0;
static uint64_t frameEnd = 0;

static MmResult graphics_init() {
    if (graphics_initialised) return kOk;
    if (SDL_Init(SDL_INIT_VIDEO) >= 0) {
        graphics_initialised = true;
        frameEnd = 0;
        return kOk;
    } else {
        return kGraphicsFailedToInitialise;
    }
}

void graphics_term() {
    for (int id = 1; id <= WINDOW_MAX_ID; id++) {
        graphics_window_destroy(id);
    }
}

const char* graphics_last_error() {
    const char* emsg = SDL_GetError();
    return emsg && *emsg ? emsg : NO_ERROR;
}

void graphics_pump_events() {
    if (!graphics_initialised) return;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
    }
    if (SDL_GetTicks64() > frameEnd) {
        // TODO: Optimise by using linked-list of windows.
        for (int id = 1; id <= WINDOW_MAX_ID; ++id) {
            MmBasicWindow* w = &windows[id - 1];
            if (w->dirty) {
                SDL_UpdateTexture(w->texture, NULL, w->pixels, w->width * 4);
                SDL_RenderCopy(w->renderer, w->texture, NULL, NULL);
                SDL_RenderPresent(w->renderer);
                w->dirty = false;
            }
        }
        frameEnd = SDL_GetTicks64() + 15;
    }
}

MmResult graphics_window_create(int id, int x, int y, int width, int height) {
    MmResult result = graphics_init();
    if (FAILED(result)) return result;

    char name[256];
    sprintf(name, "MMB4L: %d", id);
    SDL_Window* window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          width, height, SDL_WINDOW_SHOWN);
    if (!window) return kGraphicsWindowNotCreated;

    // Create a renderer with V-Sync enabled.
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

    // Create a streaming texture.
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING, width, height);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    graphics_current = &windows[id - 1];

    graphics_current->window = window;
    graphics_current->renderer = renderer;
    graphics_current->texture = texture;
    graphics_current->dirty = false;
    graphics_current->pixels = calloc(width * height, sizeof(uint32_t));
    graphics_current->height = height;
    graphics_current->width = width;
    graphics_current->x = 0;
    graphics_current->y = 0;
    graphics_current->fcolour = RGB_WHITE;
    graphics_current->bcolour = RGB_BLACK;
    windows_count++;

    return kOk;
}

MmResult graphics_window_destroy(int id) {
    MmBasicWindow* w = &windows[id - 1];
    if (w->window) {
        SDL_DestroyTexture(w->texture);
        SDL_DestroyRenderer(w->renderer);
        SDL_DestroyWindow(w->window);
        free(w->pixels);
        memset(w, 0, sizeof(MmBasicWindow));
        windows_count--;
        if (graphics_current == w) graphics_current = NULL;
        if (windows_count == 0) {
            SDL_Quit();
            graphics_initialised = false;
        }
    }
    return kOk;
}

MmResult graphics_window_use(int id) {
    if (id == 0) {
        graphics_current = NULL;
    } else if (INVALID_WINDOW(id)) {
        return kGraphicsWindowNotFound;
    } else {
        graphics_current = &windows[id - 1];
    }
    return kOk;
}

MmResult graphics_cls() {
    return graphics_draw_rectangle(0, 0, graphics_current->width - 1, graphics_current->height - 1,
                                   graphics_current->bcolour);
}

MmResult graphics_draw_aa_line(MMFLOAT x0, MMFLOAT y0, MMFLOAT x1, MMFLOAT y1, uint32_t c, int w) {
    ERROR_UNIMPLEMENTED("graphics_draw_aa_line");
    return kOk;
}

MmResult graphics_draw_box(int x1, int y1, int x2, int y2, int w, int c, int fill) {
    // Make sure the coordinates are in the right sequence.
    if (x1 > x2) SWAP(int, x1, x2);
    if (y1 > y2) SWAP(int, x1, x2);

    w = min(min(w, x2 - x1), y2 - y1);
    if (w > 0) {
        w--;
        graphics_draw_rectangle(x1, y1, x2, y1 + w, c);  // Draw the top horiz line.
        graphics_draw_rectangle(x1, y2 - w, x2, y2, c);  // Draw the bottom horiz line.
        graphics_draw_rectangle(x1, y1, x1 + w, y2, c);  // Draw the left vert line.
        graphics_draw_rectangle(x2 - w, y1, x2, y2, c);  // Draw the right vert line.
        w++;
    }

    if (fill >= 0) graphics_draw_rectangle(x1 + w, y1 + w, x2 - w, y2 - w, fill);

    return kOk;
}

MmResult graphics_draw_buffered(int xti, int yti, int c, int complete) {
    static unsigned char pos = 0;
    static unsigned char movex, movey, movec;
    static short xtilast[8];
    static short ytilast[8];
    static int clast[8];
    xtilast[pos] = xti;
    ytilast[pos] = yti;
    clast[pos] = c;
    if (complete == 1) {
        if (pos == 1) {
            SET_PIXEL_SAFE(xtilast[0], ytilast[0], clast[0]);
        } else {
            graphics_draw_line(xtilast[0], ytilast[0], xtilast[pos - 1], ytilast[pos - 1], 1,
                               clast[0]);
        }
        pos = 0;
    } else {
        if (pos == 0) {
            movex = movey = movec = 1;
            pos += 1;
        } else {
            if (xti == xtilast[0] && abs(yti - ytilast[pos - 1]) == 1)
                movex = 0;
            else
                movex = 1;
            if (yti == ytilast[0] && abs(xti - xtilast[pos - 1]) == 1)
                movey = 0;
            else
                movey = 1;
            if (c == clast[0])
                movec = 0;
            else
                movec = 1;
            if (movec == 0 && (movex == 0 || movey == 0) && pos < 6)
                pos += 1;
            else {
                if (pos == 1) {
                    SET_PIXEL_SAFE(xtilast[0], ytilast[0], clast[0]);
                } else {
                    graphics_draw_line(xtilast[0], ytilast[0], xtilast[pos - 1], ytilast[pos - 1],
                                       1, clast[0]);
                }
                movex = movey = movec = 1;
                xtilast[0] = xti;
                ytilast[0] = yti;
                clast[0] = c;
                pos = 1;
            }
        }
    }

    return kOk;
}

static void graphics_hline(int x0, int x1, int y, int f, int ints_per_line,
                           uint32_t* br) {  // draw a horizontal line
    uint32_t w1, xx1, w0, xx0, x, xn, i;
    const uint32_t a[] = {
        0xFFFFFFFF, 0x7FFFFFFF, 0x3FFFFFFF, 0x1FFFFFFF, 0xFFFFFFF, 0x7FFFFFF, 0x3FFFFFF, 0x1FFFFFF,
        0xFFFFFF,   0x7FFFFF,   0x3FFFFF,   0x1FFFFF,   0xFFFFF,   0x7FFFF,   0x3FFFF,   0x1FFFF,
        0xFFFF,     0x7FFF,     0x3FFF,     0x1FFF,     0xFFF,     0x7FF,     0x3FF,     0x1FF,
        0xFF,       0x7F,       0x3F,       0x1F,       0x0F,      0x07,      0x03,      0x01};
    const uint32_t b[] = {0x80000000, 0xC0000000, 0xe0000000, 0xf0000000, 0xf8000000, 0xfc000000,
                          0xfe000000, 0xff000000, 0xff800000, 0xffC00000, 0xffe00000, 0xfff00000,
                          0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000, 0xffff8000, 0xffffC000,
                          0xffffe000, 0xfffff000, 0xfffff800, 0xfffffc00, 0xfffffe00, 0xffffff00,
                          0xffffff80, 0xffffffC0, 0xffffffe0, 0xfffffff0, 0xfffffff8, 0xfffffffc,
                          0xfffffffe, 0xffffffff};
    w0 = y * (ints_per_line);
    xx0 = 0;
    w1 = y * (ints_per_line) + x1 / 32;
    xx1 = (x1 & 0x1F);
    w0 = y * (ints_per_line) + x0 / 32;
    xx0 = (x0 & 0x1F);
    w1 = y * (ints_per_line) + x1 / 32;
    xx1 = (x1 & 0x1F);
    if (w1 == w0) {  // special case both inside same word
        x = (a[xx0] & b[xx1]);
        xn = ~x;
        if (f)
            br[w0] |= x;
        else
            br[w0] &= xn;  // turn on the pixel
    } else {
        if (w1 - w0 > 1) {  // first deal with full words
            for (i = w0 + 1; i < w1; i++) {
                // draw the pixel
                br[i] = 0;
                if (f) br[i] = 0xFFFFFFFF;  // turn on the pixels
            }
        }
        x = ~a[xx0];
        br[w0] &= x;
        x = ~x;
        if (f) br[w0] |= x;  // turn on the pixel
        x = ~b[xx1];
        br[w1] &= x;
        x = ~x;
        if (f) br[w1] |= x;  // turn on the pixel
    }
}

MmResult graphics_draw_filled_circle(int x, int y, int radius, int r, int fill, int ints_per_line,
                                     uint32_t* br, MMFLOAT aspect, MMFLOAT aspect2) {
    int a, b, P;
    int A, B, asp;
    x = (int)((MMFLOAT)r * aspect) + radius;
    y = r + radius;
    a = 0;
    b = radius;
    P = 1 - radius;
    asp = aspect2 * (MMFLOAT)(1 << 10);
    do {
        A = (a * asp) >> 10;
        B = (b * asp) >> 10;
        graphics_hline(x - A - radius, x + A - radius, y + b - radius, fill, ints_per_line, br);
        graphics_hline(x - A - radius, x + A - radius, y - b - radius, fill, ints_per_line, br);
        graphics_hline(x - B - radius, x + B - radius, y + a - radius, fill, ints_per_line, br);
        graphics_hline(x - B - radius, x + B - radius, y - a - radius, fill, ints_per_line, br);
        if (P < 0)
            P += 3 + 2 * a++;
        else
            P += 5 + 2 * (a++ - b--);

    } while (a <= b);

    return kOk;
}

#define RoundUptoInt(a) (((a) + (32 - 1)) & (~(32 - 1)))  // round up to the nearest whole integer

MmResult graphics_draw_circle(int x, int y, int radius, int w, int c, int fill, MMFLOAT aspect) {
    int a, b, P;
    int A, B;
    int asp;
    MMFLOAT aspect2;
    if (w > 1) {
        if (fill >= 0) {  // thick border with filled centre
            graphics_draw_circle(x, y, radius, 0, c, c, aspect);
            aspect2 = ((aspect * (MMFLOAT)radius) - (MMFLOAT)w) / ((MMFLOAT)(radius - w));
            graphics_draw_circle(x, y, radius - w, 0, fill, fill, aspect2);
        } else {  // thick border with empty centre
            int r1 = radius - w, r2 = radius, xs = -1, xi = 0, i, j, k, m, ll = radius;
            if (aspect > 1.0) ll = (int)((MMFLOAT)radius * aspect);
            int ints_per_line = RoundUptoInt((ll * 2) + 1) / 32;
            uint32_t* br = (uint32_t*)GetTempMemory(((ints_per_line + 1) * ((r2 * 2) + 1)) * 4);
            graphics_draw_filled_circle(x, y, r2, r2, 1, ints_per_line, br, aspect, aspect);
            aspect2 = ((aspect * (MMFLOAT)r2) - (MMFLOAT)w) / ((MMFLOAT)r1);
            graphics_draw_filled_circle(x, y, r1, r2, 0, ints_per_line, br, aspect, aspect2);
            x = (int)((MMFLOAT)x + (MMFLOAT)r2 * (1.0 - aspect));
            for (j = 0; j < r2 * 2 + 1; j++) {
                for (i = 0; i < ints_per_line; i++) {
                    k = br[i + j * ints_per_line];
                    for (m = 0; m < 32; m++) {
                        if (xs == -1 && (k & 0x80000000)) {
                            xs = m;
                            xi = i;
                        }
                        if (xs != -1 && !(k & 0x80000000)) {
                            graphics_draw_rectangle(x - r2 + xs + xi * 32, y - r2 + j,
                                                    x - r2 + m + i * 32, y - r2 + j, c);
                            xs = -1;
                        }
                        k <<= 1;
                    }
                }
                if (xs != -1) {
                    graphics_draw_rectangle(x - r2 + xs + xi * 32, y - r2 + j, x - r2 + m + i * 32,
                                            y - r2 + j, c);
                    xs = -1;
                }
            }
        }

    } else {  // single thickness outline
        int w1 = w, r1 = radius;
        if (fill >= 0) {
            while (w >= 0 && radius > 0) {
                a = 0;
                b = radius;
                P = 1 - radius;
                asp = aspect * (MMFLOAT)(1 << 10);

                do {
                    A = (a * asp) >> 10;
                    B = (b * asp) >> 10;
                    if (fill >= 0 && w >= 0) {
                        graphics_draw_rectangle(x - A, y + b, x + A, y + b, fill);
                        graphics_draw_rectangle(x - A, y - b, x + A, y - b, fill);
                        graphics_draw_rectangle(x - B, y + a, x + B, y + a, fill);
                        graphics_draw_rectangle(x - B, y - a, x + B, y - a, fill);
                    }
                    if (P < 0)
                        P += 3 + 2 * a++;
                    else
                        P += 5 + 2 * (a++ - b--);

                } while (a <= b);
                w--;
                radius--;
            }
        }
        if (c != fill) {
            w = w1;
            radius = r1;
            while (w >= 0 && radius > 0) {
                a = 0;
                b = radius;
                P = 1 - radius;
                asp = aspect * (MMFLOAT)(1 << 10);
                do {
                    A = (a * asp) >> 10;
                    B = (b * asp) >> 10;
                    if (w) {
                        SET_PIXEL_SAFE(A + x, b + y, c);
                        SET_PIXEL_SAFE(B + x, a + y, c);
                        SET_PIXEL_SAFE(x - A, b + y, c);
                        SET_PIXEL_SAFE(x - B, a + y, c);
                        SET_PIXEL_SAFE(B + x, y - a, c);
                        SET_PIXEL_SAFE(A + x, y - b, c);
                        SET_PIXEL_SAFE(x - A, y - b, c);
                        SET_PIXEL_SAFE(x - B, y - a, c);
                    }
                    if (P < 0)
                        P += 3 + 2 * a++;
                    else
                        P += 5 + 2 * (a++ - b--);

                } while (a <= b);
                w--;
                radius--;
            }
        }
    }

    graphics_current->dirty = true;
    return kOk;
}

MmResult graphics_draw_line(int x1, int y1, int x2, int y2, int w, int c) {
    if (y1 == y2) {
        MmResult result = graphics_draw_rectangle(x1, y1, x2, y2 + w - 1, c);  // horiz line
        // if (Option.Refresh) Display_Refresh();
        return result;
    }
    if (x1 == x2) {
        MmResult result = graphics_draw_rectangle(x1, y1, x2 + w - 1, y2, c);  // vert line
        // if (Option.Refresh) Display_Refresh();
        return result;
    }
    int dx, dy, sx, sy, err, e2;
    dx = abs(x2 - x1);
    sx = x1 < x2 ? 1 : -1;
    dy = -abs(y2 - y1);
    sy = y1 < y2 ? 1 : -1;
    err = dx + dy;
    while (1) {
        graphics_draw_buffered(x1, y1, c, 0);
        e2 = 2 * err;
        if (e2 >= dy) {
            if (x1 == x2) break;
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) {
            if (y1 == y2) break;
            err += dx;
            y1 += sy;
        }
    }
    graphics_draw_buffered(0, 0, 0, 1);
    // if(Option.Refresh)Display_Refresh();
    graphics_current->dirty = true;

    return kOk;
}

MmResult graphics_draw_pixel(int x, int y, int c) {
    SET_PIXEL_SAFE(x, y, c);
    graphics_current->dirty = true;
    return kOk;
}

MmResult graphics_draw_polygon(unsigned char *p, int close) {
    ERROR_UNIMPLEMENTED("graphics_draw_polygon");
    return kOk;
}

MmResult graphics_draw_rbox(int x1, int y1, int x2, int y2, int radius, int c, int fill) {
    // Make sure the coordinates are in the right sequence.
    if (x1 > x2) SWAP(int, x1, x2);
    if (y1 > y2) SWAP(int, y1, y2);

    int f = 1 - radius;
    int ddF_x = 1;
    int ddF_y = -2 * radius;
    int xx = 0;
    int yy = radius;

    while (xx < yy) {
        if (f >= 0) {
            yy -= 1;
            ddF_y += 2;
            f += ddF_y;
        }
        xx += 1;
        ddF_x += 2;
        f += ddF_x;
        SET_PIXEL_SAFE(x2 + xx - radius, y2 + yy - radius, c);  // Bottom Right Corner
        SET_PIXEL_SAFE(x2 + yy - radius, y2 + xx - radius, c);  // ^^^
        SET_PIXEL_SAFE(x1 - xx + radius, y2 + yy - radius, c);  // Bottom Left Corner
        SET_PIXEL_SAFE(x1 - yy + radius, y2 + xx - radius, c);  // ^^^

        SET_PIXEL_SAFE(x2 + xx - radius, y1 - yy + radius, c);  // Top Right Corner
        SET_PIXEL_SAFE(x2 + yy - radius, y1 - xx + radius, c);  // ^^^
        SET_PIXEL_SAFE(x1 - xx + radius, y1 - yy + radius, c);  // Top Left Corner
        SET_PIXEL_SAFE(x1 - yy + radius, y1 - xx + radius, c);  // ^^^
        if (fill >= 0) {
            graphics_draw_line(x2 + xx - radius - 1, y2 + yy - radius, x1 - xx + radius + 1,
                               y2 + yy - radius, 1, fill);
            graphics_draw_line(x2 + yy - radius - 1, y2 + xx - radius, x1 - yy + radius + 1,
                               y2 + xx - radius, 1, fill);
            graphics_draw_line(x2 + xx - radius - 1, y1 - yy + radius, x1 - xx + radius + 1,
                               y1 - yy + radius, 1, fill);
            graphics_draw_line(x2 + yy - radius - 1, y1 - xx + radius, x1 - yy + radius + 1,
                               y1 - xx + radius, 1, fill);
        }
    }
    if (fill >= 0) graphics_draw_rectangle(x1 + 1, y1 + radius, x2 - 1, y2 - radius, fill);
    graphics_draw_rectangle(x1 + radius - 1, y1, x2 - radius + 1, y1, c);  // top side
    graphics_draw_rectangle(x1 + radius - 1, y2, x2 - radius + 1, y2, c);  // bottom side
    graphics_draw_rectangle(x1, y1 + radius, x1, y2 - radius, c);          // left side
    graphics_draw_rectangle(x2, y1 + radius, x2, y2 - radius, c);          // right side
    // if (Option.Refresh) Display_Refresh();

    return kOk;
}

MmResult graphics_draw_rectangle(int x1, int y1, int x2, int y2, int c) {
    // Do not draw anything if entire rectangle is off the screen.
    if ((x1 < 0 && x2 < 0) || (y1 < 0 && y2 < 0) ||
        ((uint32_t)x1 >= graphics_current->width && (uint32_t)x2 >= graphics_current->width) ||
        ((uint32_t)y1 >= graphics_current->height && (uint32_t)y2 >= graphics_current->height)) {
        return kOk;
    }

    x1 = min(max(0, x1), (int)(HRes - 1));
    x2 = min(max(0, x2), (int)(HRes - 1));
    y1 = min(max(0, y1), (int)(VRes - 1));
    y2 = min(max(0, y2), (int)(VRes - 1));
    if (x1 > x2) SWAP(int, x1, x2);
    if (y1 > y2) SWAP(int, y1, y2);
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) SET_PIXEL(x, y, c);
    }
    graphics_current->dirty = true;
    return kOk;
}

void CalcLine(int x1, int y1, int x2, int y2, short* xmin, short* xmax) {
    if (y1 == y2) {
        if (y1 < 0) y1 = 0;
        if (y1 >= 480) y1 = 479;
        if (y2 < 0) y2 = 0;
        if (y2 >= 480) y2 = 479;
        if (x1 < xmin[y1]) xmin[y1] = x1;
        if (x2 < xmin[y1]) xmin[y1] = x2;
        if (x1 > xmax[y1]) xmax[y1] = x1;
        if (x2 > xmax[y1]) xmax[y1] = x2;
        return;
    }
    if (x1 == x2) {
        if (y2 < y1) SWAP(int, y2, y1);
        if (y1 < 0) y1 = 0;
        if (y1 >= 480) y1 = 479;
        if (y2 < 0) y2 = 0;
        if (y2 >= 480) y2 = 479;
        for (int y = y1; y <= y2; y++) {
            if (x1 < xmin[y]) xmin[y] = x1;
            if (x1 > xmax[y]) xmax[y] = x1;
        }
        return;
    }
    // uses a variant of Bresenham's line algorithm:
    //   https://en.wikipedia.org/wiki/Talk:Bresenham%27s_line_algorithm
    if (y1 > y2) {
        SWAP(int, y1, y2);
        SWAP(int, x1, x2);
    }
    if (y1 < 0) y1 = 0;
    if (y1 >= 480) y1 = 479;
    if (y2 < 0) y2 = 0;
    if (y2 >= 480) y2 = 479;
    int absX = abs(x1 - x2);  // absolute value of coordinate distances
    int absY = abs(y1 - y2);
    int offX = x2 < x1 ? 1 : -1;  // line-drawing direction offsets
    int offY = y2 < y1 ? 1 : -1;
    int x = x2;  // incremental location
    int y = y2;
    int err;
    if (x < xmin[y]) xmin[y] = x;
    if (x > xmax[y]) xmax[y] = x;
    if (absX > absY) {
        // line is more horizontal; increment along x-axis
        err = absX / 2;
        while (x != x1) {
            err = err - absY;
            if (err < 0) {
                y += offY;
                err += absX;
            }
            x += offX;
            if (x < xmin[y]) xmin[y] = x;
            if (x > xmax[y]) xmax[y] = x;
        }
    } else {
        // line is more vertical; increment along y-axis
        err = absY / 2;
        while (y != y1) {
            err = err - absX;
            if (err < 0) {
                x += offX;
                err += absY;
            }
            y += offY;
            if (x < xmin[y]) xmin[y] = x;
            if (x > xmax[y]) xmax[y] = x;
        }
    }
}

MmResult graphics_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, int c, int fill) {
    if (x0 * (y1 - y2) + x1 * (y2 - y0) + x2 * (y0 - y1) ==
        0) {  // points are co-linear i.e zero area
        if (y0 > y1) {
            SWAP(int, y0, y1);
            SWAP(int, x0, x1);
        }
        if (y1 > y2) {
            SWAP(int, y2, y1);
            SWAP(int, x2, x1);
        }
        if (y0 > y1) {
            SWAP(int, y0, y1);
            SWAP(int, x0, x1);
        }
        graphics_draw_line(x0, y0, x2, y2, 1, c);
    } else {
        if (fill == -1) {
            // draw only the outline
            graphics_draw_line(x0, y0, x1, y1, 1, c);
            graphics_draw_line(x1, y1, x2, y2, 1, c);
            graphics_draw_line(x2, y2, x0, y0, 1, c);
        } else {
            if (y0 > y1) {
                SWAP(int, y0, y1);
                SWAP(int, x0, x1);
            }
            if (y1 > y2) {
                SWAP(int, y2, y1);
                SWAP(int, x2, x1);
            }
            if (y0 > y1) {
                SWAP(int, y0, y1);
                SWAP(int, x0, x1);
            }
            short* xmin = (short*)GetMemory(480 * sizeof(short));
            short* xmax = (short*)GetMemory(480 * sizeof(short));

            int y;
            for (y = y0; y <= y2; y++) {
                if (y >= 0 && y < 480) {
                    xmin[y] = 32767;
                    xmax[y] = -1;
                }
            }
            CalcLine(x0, y0, x1, y1, xmin, xmax);
            CalcLine(x1, y1, x2, y2, xmin, xmax);
            CalcLine(x2, y2, x0, y0, xmin, xmax);
            for (y = y0; y <= y2; y++) {
                if (y >= 0 && y < (int)VRes) graphics_draw_rectangle(xmin[y], y, xmax[y], y, fill);
            }
            //            if(c!=f){
            graphics_draw_line(x0, y0, x1, y1, 1, c);
            graphics_draw_line(x1, y1, x2, y2, 1, c);
            graphics_draw_line(x2, y2, x0, y0, 1, c);
            //            }
            FreeMemory((unsigned char*)xmin);
            FreeMemory((unsigned char*)xmax);
        }
    }

    return kOk;
}
