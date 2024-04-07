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

#include "console.h"
#include "cstring.h"
#include "error.h"
#include "events.h"
#include "fonttbl.h"
#include "graphics.h"
#include "memory.h"
#include "upng.h"
#include "utility.h"

#include <stdbool.h>

#include <SDL.h>

#define HRes graphics_current->width
#define VRes graphics_current->height

static const char* NO_ERROR = "";
static bool graphics_initialised = false;
MmSurface graphics_surfaces[GRAPHICS_MAX_SURFACES] = { 0 };
MmSurface* graphics_current = NULL;
MmGraphicsColour graphics_fcolour = RGB_WHITE;
MmGraphicsColour graphics_bcolour = RGB_BLACK;
uint32_t graphics_font = 1;
static uint64_t frameEnd = 0;

MmResult graphics_init() {
    if (graphics_initialised) return kOk;
    MmResult result = events_init();
    if (FAILED(result)) return result;
    frameEnd = 0;
    graphics_initialised = true;
    return kOk;
}

const char *graphics_last_error() {
    const char* emsg = SDL_GetError();
    return emsg && *emsg ? emsg : NO_ERROR;
}

void graphics_term() {
    graphics_surface_destroy_all();
    graphics_fcolour = RGB_WHITE;
    graphics_bcolour = RGB_BLACK;
}

void graphics_refresh_windows() {
    // if (SDL_GetTicks64() > frameEnd) {
    if (SDL_GetTicks() > frameEnd) {
        // TODO: Optimise by using linked-list of windows.
        for (int id = 0; id <= GRAPHICS_MAX_ID; ++id) {
            MmSurface* w = &graphics_surfaces[id];
            if (w->type == kGraphicsWindow && w->dirty) {
                SDL_UpdateTexture((SDL_Texture *) w->texture, NULL, w->pixels, w->width * 4);
                SDL_RenderCopy((SDL_Renderer *) w->renderer, (SDL_Texture *) w->texture, NULL,
                               NULL);
                SDL_RenderPresent((SDL_Renderer *) w->renderer);
                w->dirty = false;
            }
        }
        // frameEnd = SDL_GetTicks64() + 15;
        frameEnd = SDL_GetTicks() + 15;
    }
}

MmResult graphics_buffer_create(MmSurfaceId id, uint32_t width, uint32_t height) {
    MmResult result = graphics_init();
    if (FAILED(result)) return result;

    if (id < 0 || id > GRAPHICS_MAX_ID) return kGraphicsInvalidId;
    if (graphics_surfaces[id].type != kGraphicsNone) return kGraphicsSurfaceExists;
    if (width > WINDOW_MAX_WIDTH || height > WINDOW_MAX_HEIGHT) return kGraphicsSurfaceTooLarge;

    MmSurface *s = &graphics_surfaces[id];
    s->type = kGraphicsBuffer;
    s->window = NULL;
    s->renderer = NULL;
    s->texture = NULL;
    s->dirty = false;
    s->pixels = calloc(width * height, sizeof(uint32_t));
    s->height = height;
    s->width = width;

    return kOk;
}

MmResult graphics_window_create(MmSurfaceId id, int x, int y, uint32_t width, uint32_t height,
                                uint32_t scale) {
    MmResult result = graphics_init();
    if (FAILED(result)) return result;

    if (id < 0 || id > GRAPHICS_MAX_ID) return kGraphicsInvalidId;
    if (graphics_surfaces[id].type != kGraphicsNone) return kGraphicsSurfaceExists;
    if (width > WINDOW_MAX_WIDTH || height > WINDOW_MAX_HEIGHT) return kGraphicsSurfaceTooLarge;

    // Reduce scale to fit display.
    float fscale = scale;
    SDL_DisplayMode dm;
    if (FAILED(SDL_GetCurrentDisplayMode(0, &dm))) return kGraphicsSurfaceNotCreated;
    while (width * fscale > dm.w * 0.9 || height * fscale > dm.h * 0.9) {
        fscale -= fscale > 2.0 ? 1.0 : 0.1;
        if (fscale < 0.1) return kGraphicsSurfaceTooLarge;
    }

    char name[256];
    sprintf(name, "MMB4L: %d", id);
    SDL_Window *window = SDL_CreateWindow(name, x == -1 ? (int)SDL_WINDOWPOS_CENTERED : x,
                                          y == -1 ? (int)SDL_WINDOWPOS_CENTERED : y, width * fscale,
                                          height * fscale, SDL_WINDOW_SHOWN);
    if (!window) return kGraphicsSurfaceNotCreated;

    // Create a renderer with V-Sync enabled.
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

    // Create a streaming texture.
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING, width, height);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    MmSurface *s = &graphics_surfaces[id];
    s->type = kGraphicsWindow;
    s->window = window;
    s->renderer = renderer;
    s->texture = texture;
    s->dirty = false;
    s->pixels = calloc(width * height, sizeof(uint32_t));
    s->height = height;
    s->width = width;

    return kOk;
}

MmResult graphics_surface_destroy(MmSurfaceId id) {
    if (id < 0 || id > GRAPHICS_MAX_ID) return kGraphicsInvalidId;

    MmSurface* surface = &graphics_surfaces[id];
    if (surface->type != kGraphicsNone) {
        SDL_DestroyTexture((SDL_Texture *) surface->texture);
        SDL_DestroyRenderer((SDL_Renderer *) surface->renderer);
        SDL_DestroyWindow((SDL_Window *) surface->window);
        free(surface->pixels);
        memset(surface, 0, sizeof(MmSurface));
        if (graphics_current == surface) graphics_current = NULL;
    }
    return kOk;
}

MmResult graphics_surface_destroy_all() {
    MmResult result = kOk;
    MmResult new_result = kOk;
    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        new_result = graphics_surface_destroy(id);
        result = SUCCEEDED(result) ? new_result : result;
    }
    return result;
}

MmResult graphics_surface_write(MmSurfaceId id) {
    if (id == GRAPHICS_NONE) {
        graphics_current = NULL;
    } else if (!graphics_surface_exists(id)) {
        return kGraphicsSurfaceNotFound;
    } else {
        graphics_current = &graphics_surfaces[id];
    }
    return kOk;
}

static inline void graphics_set_pixel(MmSurface *surface, int x, int y, MmGraphicsColour colour) {
    surface->pixels[y*surface->width + x] = colour;
}

static inline void graphics_set_pixel_safe(MmSurface *surface, int x, int y, MmGraphicsColour colour) {
    if (x >= 0 && y >= 0 && (uint32_t) x < surface->width && (uint32_t) y < surface->height) {
        surface->pixels[y*surface->width + x] = colour;
    }
}

MmResult graphics_draw_pixel(MmSurface *surface, int x, int y, MmGraphicsColour colour) {
    graphics_set_pixel_safe(surface, x, y, colour);
    surface->dirty = true;
    return kOk;
}

MmResult graphics_draw_aa_line(MmSurface *surface, MMFLOAT x0, MMFLOAT y0, MMFLOAT x1, MMFLOAT y1,
                               MmGraphicsColour colour, uint32_t w) {
    ERROR_UNIMPLEMENTED("graphics_draw_aa_line");
    return kOk;
}

MmResult graphics_draw_bitmap(MmSurface *surface, int x1, int y1, uint32_t width, uint32_t height,
                              uint32_t scale, MmGraphicsColour fcolour, MmGraphicsColour bcolour,
                              const unsigned char* bitmap) {
    const int32_t hres = surface->width;
    const int32_t vres = surface->height;
    // if (optiony) y1 = VRes - 1 - y1;
    if (x1 >= hres
            || y1 >= vres
            || x1 + (int32_t) (width * scale) < 0
            || y1 + (int32_t) (height * scale) < 0) return kOk;

    int x, y;
    uint32_t *dst = surface->pixels;
    for (uint32_t i = 0; i < height; i++) {           // step thru the bitmap scan line by line
        for (uint32_t j = 0; j < scale; j++) {          // repeat lines to scale the bitmap
            y = y1 + i * scale + j;
            for (uint32_t k = 0; k < width; k++) {      // step through each bit in a scan line
                for (uint32_t m = 0; m < scale; m++) {  // repeat pixels to scale in the x axis
                    x = x1 + k * scale + m;
                    if (x >= 0 && x < hres && y >= 0 && y < vres) {  // if the coordinates are valid
                        if ((bitmap[((i * width) + k) / 8] >> (((height * width) - ((i * width) + k) - 1) % 8)) & 1) {
                            dst[y * hres + x] = fcolour; // (uint32_t)fc | (ARGBenabled ? 0 : 0xFF000000);
                        } else {
                            if (bcolour !=-1) {
                                dst[y * hres + x] = bcolour; // (uint32_t)bc | (ARGBenabled ? 0 : 0xFF000000);
                            }
                        }
                    }
                }
            }
        }
    }
    surface->dirty = true;
    return kOk;
}

MmResult graphics_draw_box(MmSurface *surface, int x1, int y1, int x2, int y2, uint32_t w,
                           MmGraphicsColour colour, MmGraphicsColour fill) {
    // Make sure the coordinates are in the right sequence.
    if (x1 > x2) SWAP(int, x1, x2);
    if (y1 > y2) SWAP(int, x1, x2);

    w = min(min(w, (uint32_t) (x2 - x1)), (uint32_t) (y2 - y1));
    if (w > 0) {
        w--;
        graphics_draw_rectangle(surface, x1, y1, x2, y1 + w, colour);  // Draw the top horiz line.
        graphics_draw_rectangle(surface, x1, y2 - w, x2, y2, colour);  // Draw the bottom horiz line.
        graphics_draw_rectangle(surface, x1, y1, x1 + w, y2, colour);  // Draw the left vert line.
        graphics_draw_rectangle(surface, x2 - w, y1, x2, y2, colour);  // Draw the right vert line.
        w++;
    }

    if (fill >= 0) graphics_draw_rectangle(surface, x1 + w, y1 + w, x2 - w, y2 - w, fill);

    return kOk;
}

MmResult graphics_draw_buffered(MmSurface *surface, int xti, int yti, MmGraphicsColour colour, int complete) {
    static unsigned char pos = 0;
    static unsigned char movex, movey, movec;
    static short xtilast[8];
    static short ytilast[8];
    static MmGraphicsColour clast[8];
    xtilast[pos] = xti;
    ytilast[pos] = yti;
    clast[pos] = colour;
    if (complete == 1) {
        if (pos == 1) {
            graphics_set_pixel_safe(surface, xtilast[0], ytilast[0], clast[0]);
        } else {
            graphics_draw_line(surface, xtilast[0], ytilast[0], xtilast[pos - 1], ytilast[pos - 1],
                               1, clast[0]);
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
            if (colour == clast[0])
                movec = 0;
            else
                movec = 1;
            if (movec == 0 && (movex == 0 || movey == 0) && pos < 6)
                pos += 1;
            else {
                if (pos == 1) {
                    graphics_set_pixel_safe(surface, xtilast[0], ytilast[0], clast[0]);
                } else {
                    graphics_draw_line(surface, xtilast[0], ytilast[0], xtilast[pos - 1],
                                       ytilast[pos - 1], 1, clast[0]);
                }
                movex = movey = movec = 1;
                xtilast[0] = xti;
                ytilast[0] = yti;
                clast[0] = colour;
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

MmResult graphics_draw_filled_circle(int x, int y, int radius, int r,
                                     MmGraphicsColour fill, int ints_per_line, uint32_t* br,
                                     MMFLOAT aspect, MMFLOAT aspect2) {
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

MmResult graphics_draw_circle(MmSurface *surface, int x, int y, int radius, uint32_t w,
                              MmGraphicsColour colour, MmGraphicsColour fill, MMFLOAT aspect) {
    if (!surface || surface->type == kGraphicsNone) return kGraphicsInvalidWriteSurface;
    int a, b, P;
    int A, B;
    int asp;
    MMFLOAT aspect2;
    if (w > 1) {
        if (fill >= 0) {  // thick border with filled centre
            graphics_draw_circle(surface, x, y, radius, 0, colour, colour, aspect);
            aspect2 = ((aspect * (MMFLOAT)radius) - (MMFLOAT)w) / ((MMFLOAT)(radius - w));
            graphics_draw_circle(surface, x, y, radius - w, 0, fill, fill, aspect2);
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
                            graphics_draw_rectangle(surface, x - r2 + xs + xi * 32, y - r2 + j,
                                                    x - r2 + m + i * 32, y - r2 + j, colour);
                            xs = -1;
                        }
                        k <<= 1;
                    }
                }
                if (xs != -1) {
                    graphics_draw_rectangle(surface, x - r2 + xs + xi * 32, y - r2 + j, x - r2 + m + i * 32,
                                            y - r2 + j, colour);
                    xs = -1;
                }
            }
        }

    } else {  // single thickness outline
        int w1 = w, r1 = radius;
        if (fill >= 0) {
            while (radius > 0) {
                a = 0;
                b = radius;
                P = 1 - radius;
                asp = aspect * (MMFLOAT)(1 << 10);

                do {
                    A = (a * asp) >> 10;
                    B = (b * asp) >> 10;
                    if (fill >= 0) {
                        graphics_draw_rectangle(surface, x - A, y + b, x + A, y + b, fill);
                        graphics_draw_rectangle(surface, x - A, y - b, x + A, y - b, fill);
                        graphics_draw_rectangle(surface, x - B, y + a, x + B, y + a, fill);
                        graphics_draw_rectangle(surface, x - B, y - a, x + B, y - a, fill);
                    }
                    if (P < 0)
                        P += 3 + 2 * a++;
                    else
                        P += 5 + 2 * (a++ - b--);

                } while (a <= b);
                if (w == 0) break;
                w--;
                radius--;
            }
        }
        if (colour != fill) {
            w = w1;
            radius = r1;
            while (radius > 0) {
                a = 0;
                b = radius;
                P = 1 - radius;
                asp = aspect * (MMFLOAT)(1 << 10);
                do {
                    A = (a * asp) >> 10;
                    B = (b * asp) >> 10;
                    if (w) {
                        graphics_set_pixel_safe(surface, A + x, b + y, colour);
                        graphics_set_pixel_safe(surface, B + x, a + y, colour);
                        graphics_set_pixel_safe(surface, x - A, b + y, colour);
                        graphics_set_pixel_safe(surface, x - B, a + y, colour);
                        graphics_set_pixel_safe(surface, B + x, y - a, colour);
                        graphics_set_pixel_safe(surface, A + x, y - b, colour);
                        graphics_set_pixel_safe(surface, x - A, y - b, colour);
                        graphics_set_pixel_safe(surface, x - B, y - a, colour);
                    }
                    if (P < 0)
                        P += 3 + 2 * a++;
                    else
                        P += 5 + 2 * (a++ - b--);

                } while (a <= b);
                if (w == 0) break;
                w--;
                radius--;
            }
        }
    }

    surface->dirty = true;
    return kOk;
}

MmResult graphics_draw_line(MmSurface *surface, int x1, int y1, int x2, int y2, uint32_t w, MmGraphicsColour colour) {
    if (y1 == y2) {
        return graphics_draw_rectangle(surface, x1, y1, x2, y2 + w - 1, colour);  // horiz line
    }
    if (x1 == x2) {
        return graphics_draw_rectangle(surface, x1, y1, x2 + w - 1, y2, colour);  // vert line
    }
    int dx, dy, sx, sy, err, e2;
    dx = abs(x2 - x1);
    sx = x1 < x2 ? 1 : -1;
    dy = -abs(y2 - y1);
    sy = y1 < y2 ? 1 : -1;
    err = dx + dy;
    while (1) {
        graphics_draw_buffered(surface, x1, y1, colour, 0);
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
    graphics_draw_buffered(surface, 0, 0, 0, 1);
    // if (Option.Refresh)Display_Refresh();
    surface->dirty = true;

    return kOk;
}

MmResult graphics_draw_polygon(MmSurface *surface, unsigned char *p, int close) {
    ERROR_UNIMPLEMENTED("graphics_draw_polygon");
    return kOk;
}

MmResult graphics_draw_rbox(MmSurface *surface, int x1, int y1, int x2, int y2, int radius,
                            MmGraphicsColour colour, MmGraphicsColour fill) {
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
        graphics_set_pixel_safe(surface, x2 + xx - radius, y2 + yy - radius, colour);  // Bottom Right Corner
        graphics_set_pixel_safe(surface, x2 + yy - radius, y2 + xx - radius, colour);  // ^^^
        graphics_set_pixel_safe(surface, x1 - xx + radius, y2 + yy - radius, colour);  // Bottom Left Corner
        graphics_set_pixel_safe(surface, x1 - yy + radius, y2 + xx - radius, colour);  // ^^^

        graphics_set_pixel_safe(surface, x2 + xx - radius, y1 - yy + radius, colour);  // Top Right Corner
        graphics_set_pixel_safe(surface, x2 + yy - radius, y1 - xx + radius, colour);  // ^^^
        graphics_set_pixel_safe(surface, x1 - xx + radius, y1 - yy + radius, colour);  // Top Left Corner
        graphics_set_pixel_safe(surface, x1 - yy + radius, y1 - xx + radius, colour);  // ^^^
        if (fill >= 0) {
            graphics_draw_line(surface, x2 + xx - radius - 1, y2 + yy - radius, x1 - xx + radius + 1,
                               y2 + yy - radius, 1, fill);
            graphics_draw_line(surface, x2 + yy - radius - 1, y2 + xx - radius, x1 - yy + radius + 1,
                               y2 + xx - radius, 1, fill);
            graphics_draw_line(surface, x2 + xx - radius - 1, y1 - yy + radius, x1 - xx + radius + 1,
                               y1 - yy + radius, 1, fill);
            graphics_draw_line(surface, x2 + yy - radius - 1, y1 - xx + radius, x1 - yy + radius + 1,
                               y1 - xx + radius, 1, fill);
        }
    }
    if (fill >= 0) graphics_draw_rectangle(surface, x1 + 1, y1 + radius, x2 - 1, y2 - radius, fill);
    graphics_draw_rectangle(surface, x1 + radius - 1, y1, x2 - radius + 1, y1, colour);  // top side
    graphics_draw_rectangle(surface, x1 + radius - 1, y2, x2 - radius + 1, y2, colour);  // bottom side
    graphics_draw_rectangle(surface, x1, y1 + radius, x1, y2 - radius, colour);          // left side
    graphics_draw_rectangle(surface, x2, y1 + radius, x2, y2 - radius, colour);          // right side
    // if (Option.Refresh) Display_Refresh();

    return kOk;
}

MmResult graphics_draw_rectangle(MmSurface *surface, int x1, int y1, int x2, int y2,
                                 MmGraphicsColour colour) {
    // Do not draw anything if entire rectangle is off the screen.
    if ((x1 < 0 && x2 < 0) || (y1 < 0 && y2 < 0) ||
        ((uint32_t)x1 >= surface->width && (uint32_t)x2 >= surface->width) ||
        ((uint32_t)y1 >= surface->height && (uint32_t)y2 >= surface->height)) {
        return kOk;
    }

    x1 = min(max(0, x1), (int)(HRes - 1));
    x2 = min(max(0, x2), (int)(HRes - 1));
    y1 = min(max(0, y1), (int)(VRes - 1));
    y2 = min(max(0, y2), (int)(VRes - 1));
    if (x1 > x2) SWAP(int, x1, x2);
    if (y1 > y2) SWAP(int, y1, y2);
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) graphics_set_pixel(surface, x, y, colour);
    }
    surface->dirty = true;
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

MmResult graphics_draw_triangle(MmSurface *surface, int x0, int y0, int x1, int y1, int x2, int y2,
                                MmGraphicsColour colour, MmGraphicsColour fill) {
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
        return graphics_draw_line(surface, x0, y0, x2, y2, 1, colour);
    } else {
        if (fill == -1) {
            // draw only the outline
            graphics_draw_line(surface, x0, y0, x1, y1, 1, colour);
            graphics_draw_line(surface, x1, y1, x2, y2, 1, colour);
            graphics_draw_line(surface, x2, y2, x0, y0, 1, colour);
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
                if (y >= 0 && y < (int)VRes) graphics_draw_rectangle(surface, xmin[y], y, xmax[y], y, fill);
            }
            //            if (c!=f){
            graphics_draw_line(surface, x0, y0, x1, y1, 1, colour);
            graphics_draw_line(surface, x1, y1, x2, y2, 1, colour);
            graphics_draw_line(surface, x2, y2, x0, y0, 1, colour);
            //            }
            FreeMemory((unsigned char*)xmin);
            FreeMemory((unsigned char*)xmax);
        }
    }

    return kOk;
}

void graphics_draw_buffer(MmSurface *surface, int x1, int y1, int x2, int y2,
                          const unsigned char* buffer, int skip) {
    const unsigned char *psrc = buffer;
    union colourmap
    {
        char rgbbytes[4];
        uint32_t rgb;
    } c;
    int scale = 1; // (PageTable[WritePage].expand ? 2 : 1);
    //if (optiony)y1=maxH-1-y1;
    //if (optiony)y2=maxH-1-y2;
    // make sure the coordinates are kept within the display area
    if (x2 <= x1) SWAP(int, x1, x2);
    if (y2 <= y1) SWAP(int, y1, y2);
    // int cursorhidden=0;
    // if (cursoron)
    //     if ( !(xcursor + wcursor < x1 ||
    //         xcursor > x2 ||
    //         ycursor + hcursor < y1 ||
    //         ycursor > y2)){
    //     hidecursor(0);
    //     cursorhidden=1;
    //     }
    if (scale==1){
        for (int32_t y = y1; y <= y2; y++){
            // routinechecks(1);
            uint32_t *pdst = surface->pixels + (y * surface->width + x1);
            for (int32_t x = x1; x <= x2; x++){
                if (x >= 0 && x < (int32_t) surface->width && y >= 0 && y < (int32_t) surface->height) {
                    if (skip & 2) {
                        c.rgbbytes[3] = 0xFF; //assume solid colour
                        c.rgbbytes[2] = *psrc++; //this order swaps the bytes to match the .BMP file
                        c.rgbbytes[1] = *psrc++;
                        c.rgbbytes[0] = *psrc++;
                        if (skip & 1) c.rgbbytes[3] = *psrc++; //ARGB8888 so set transparency
                    } else {
                        c.rgbbytes[3] = 0;
                        c.rgbbytes[0] = *psrc++; //this order swaps the bytes to match the .BMP file
                        c.rgbbytes[1] = *psrc++;
                        c.rgbbytes[2] = *psrc++;
                        if (skip & 1) psrc++;
                    }
                    *pdst = c.rgb;
                } else {
                    psrc += (skip & 1) ? 4 : 3;
                }
                pdst++;
            }
        }
    }
    // } else {
    //     uint32_t *s1;
    //     for(y=y1*2;y<=y2*2;y+=2){
    //         routinechecks(1);
    //         sc=(uint32_t *)((y * maxW + x1) * 4 + wpa);
    //         s1=(uint32_t *)(((y+1) * maxW + x1) * 4 + wpa);
    //         for(x=x1;x<=x2;x++){
    //             if (x>=0 && x<maxW && y>=0 && y<maxH*2){
    //                 if (skip & 2){
    //                     c.rgbbytes[3]=0xFF;
    //                     c.rgbbytes[2]=*p++; //this order swaps the bytes to match the .BMP file
    //                     c.rgbbytes[1]=*p++;
    //                     c.rgbbytes[0]=*p++;
    //                     if (skip & 1)c.rgbbytes[3]=*p++; //ARGB8888 so set transparency
    //                 } else {
    //                     c.rgbbytes[3]=0;
    //                     c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
    //                     c.rgbbytes[1]=*p++;
    //                     c.rgbbytes[2]=*p++;
    //                     if (skip & 1)p++;
    //                 }
    //                 *sc=c.rgb;
    //                 *s1=*sc;
    //             } else {
    //                 p+=(skip & 1) ? 4 : 3;
    //             }
    //             sc++;
    //             s1++;
    //         }
    //     }
    // }
    // if (cursorhidden)showcursor(0, xcursor,ycursor);
}

MmResult graphics_load_png(MmSurface *surface, char *filename, int x, int y, int transparent, int force) {
    if (strchr(filename, '.') == NULL) cstring_cat(filename, ".PNG", STRINGSIZE);
    upng_t *upng = upng_new_from_file(filename);
    // routinechecks(1);
    upng_header(upng);
    unsigned w = upng_get_width(upng);
    unsigned h = upng_get_height(upng);
    if (x + w > graphics_current->width || y + h > graphics_current->height) {
        upng_free(upng);
        return kImageTooLarge;
    }
    if (!(upng_get_format(upng)==1 || upng_get_format(upng)==3)){
        upng_free(upng);
        return kImageInvalidFormat;
    }
    // routinechecks(1);
    upng_decode(upng);
    // routinechecks(1);
    const unsigned char *buffer = upng_get_buffer(upng);
    // int savey = optiony;
    // optiony = 0;
    if (upng_get_format(upng) ==3) {
        graphics_draw_buffer(surface, x, y, x + w - 1, y + h - 1, buffer, 3 | transparent | force);
    } else {
        graphics_draw_buffer(surface, x, y, x + w - 1, y + h - 1, buffer, 2 | transparent | force);
    }
    // optiony = savey;
    upng_free(upng);
    // clearrepeat();
    surface->dirty = true;
    return kOk;
}

/*
 * FLIP parameter
0 - normal display (default if omitted)
1 - mirrored left to right
2 - mirrored top to bottom
3 - rotated 180 degrees (= 1+2)
4 - transparent normal display (default if omitted)
5 - transparent mirrored left to right
6 - transparent mirrored top to bottom
7 - transparent rotated 180 degrees (= 1+2)
*/
MmResult graphics_blit(int x1, int y1, int x2, int y2, uint32_t w, uint32_t h,
                       MmSurface *read_surface, MmSurface *write_surface, int flags){
    uint32_t *src = read_surface->pixels + (y1 * read_surface->width) + x1;
    uint32_t *dst = write_surface->pixels + (y2 * write_surface->width) + x2;
    for (uint32_t i = 0; i < h; i++) {
        memcpy(dst, src, w << 2);
        src += read_surface->width;
        dst += write_surface->width;
    }
    write_surface->dirty = true;
    return kOk;
}

MmResult graphics_draw_char(MmSurface *surface,  int *x, int *y, uint32_t font,
                            MmGraphicsColour fcolour, MmGraphicsColour bcolour, char c,
                            TextOrientation orientation) {
    unsigned char *p, *fp, *np = NULL;
    int BitNumber, BitPos, newx, newy, modx, mody, scale = font & 0b1111;

    uint32_t PrintPixelMode = 0; // Currently always 0.
    switch (PrintPixelMode) {
        case 0:
            break;
        case 1:
            bcolour = -1;
            break;
        case 2:
            SWAP(int64_t, fcolour, bcolour);
            break;
        case 5:
            fcolour = bcolour;
            bcolour = -1;
            break;
        default:
            return kInternalFault;
    }

    // to get the +, - and = chars for font 6 we fudge them by scaling up font 1
    if ((font & 0xf0) == 0x50 && (c == '-' || c == '+' || c == '=')) {
        fp = (unsigned char *) FontTable[0];
        scale = scale * 4;
    }
    else
        fp = (unsigned char *) FontTable[font >> 4];

    uint32_t height = fp[1];
    uint32_t width = fp[0];
    modx = mody = 0;
    if (orientation > kOrientVert) {
        np = (unsigned char *)GetTempMemory(width * height);
        if (orientation == kOrientInverted) {
            modx -= width * scale - 1;
            mody -= height * scale - 1;
        } else if (orientation == kOrientCounterClock) {
            mody -= width * scale;
        } else if (orientation == kOrientClockwise) {
            modx -= height * scale - 1;
        }
    }

    if (c >= fp[2] && c < fp[2] + fp[3]) {
        p = fp + 4 + (int)(((c - fp[2]) * height * width) / 8);

        if (orientation > kOrientVert) {                             // non-standard orientation
            if (orientation == kOrientInverted) {
                for (uint32_t y = 0; y < height; y++) {
                    newy = height - y - 1;
                    for (uint32_t x = 0; x < width; x++) {
                        newx = width - x - 1;
                        if ((p[((y * width) + x) / 8] >> (((height * width) - ((y * width) + x) - 1) % 8)) & 1) {
                            BitNumber = ((newy * width) + newx);
                            BitPos = 128 >> (BitNumber % 8);
                            np[BitNumber / 8] |= BitPos;
                        }
                    }
                }
            }
            else if (orientation == kOrientCounterClock) {
                for (uint32_t y = 0; y < height; y++) {
                    newx = y;
                    for (uint32_t x = 0; x < width; x++) {
                        newy = width - x - 1;
                        if ((p[((y * width) + x) / 8] >> (((height * width) - ((y * width) + x) - 1) % 8)) & 1) {
                            BitNumber = ((newy * height) + newx);
                            BitPos = 128 >> (BitNumber % 8);
                            np[BitNumber / 8] |= BitPos;
                        }
                    }
                }
            }
            else if (orientation == kOrientClockwise) {
                for (uint32_t y = 0; y < height; y++) {
                    newx = height - y - 1;
                    for (uint32_t x = 0; x < width; x++) {
                        newy = x;
                        if ((p[((y * width) + x) / 8] >> (((height * width) - ((y * width) + x) - 1) % 8)) & 1) {
                            BitNumber = ((newy * height) + newx);
                            BitPos = 128 >> (BitNumber % 8);
                            np[BitNumber / 8] |= BitPos;
                        }
                    }
                }
            }
        }
        else np = p;

        if (orientation < kOrientCounterClock) {
            graphics_draw_bitmap(surface, *x + modx, *y + mody, width, height, scale, fcolour,
                                 bcolour, np);
        } else {
            graphics_draw_bitmap(surface, *x + modx, *y + mody, height, width, scale, fcolour,
                                 bcolour, np);
        }
    }
    else {
        if (orientation < kOrientCounterClock) {
            graphics_draw_rectangle(surface, *x + modx, *y + mody, *x + modx + (width * scale),
                                    *y + mody + (height * scale), bcolour);
        } else {
            graphics_draw_rectangle(surface, *x + modx, *y + mody, *x + modx + (height * scale),
                                    *y + mody + (width * scale), bcolour);
        }
    }

    // to get the . and degree symbols for font 6 we draw a small circle
    if ((font & 0xf0) == 0x50) {
        if (orientation > kOrientVert) {
            if (orientation == kOrientInverted) {
                if (c == '.') {
                    graphics_draw_circle(surface, *x + modx + (width * scale) / 2,
                                         *y + mody + 7 * scale, 4 * scale, 0, fcolour, fcolour,
                                         1.0);
                } else if (c == 0x60) {
                    graphics_draw_circle(surface, *x + modx + (width * scale) / 2,
                                        *y + mody + (height * scale) - 9 * scale, 6 * scale,
                                        2 * scale, fcolour, -1, 1.0);
                }
            } else if (orientation == kOrientCounterClock) {
                if (c == '.') {
                    graphics_draw_circle(surface, *x + modx + (height * scale) - 7 * scale,
                                         *y + mody + (width * scale) / 2, 4 * scale, 0, fcolour,
                                         fcolour, 1.0);
                } else if (c == 0x60) {
                    graphics_draw_circle(surface, *x + modx + 9 * scale,
                                         *y + mody + (width * scale) / 2, 6 * scale, 2 * scale,
                                         fcolour, -1, 1.0);
                }
            } else if (orientation == kOrientClockwise) {
                if (c == '.') {
                    graphics_draw_circle(surface, *x + modx + 7 * scale,
                                         *y + mody + (width * scale) / 2, 4 * scale, 0, fcolour,
                                         fcolour, 1.0);
                } else if (c == 0x60) {
                    graphics_draw_circle(surface, *x + modx + (height * scale) - 9 * scale,
                                         *y + mody + (width * scale) / 2, 6 * scale, 2 * scale,
                                         fcolour, -1, 1.0);
                }
            }
        }
        else {
            if (c == '.') {
                graphics_draw_circle(surface, *x + modx + (width * scale) / 2,
                                     *y + mody + (height * scale) - 7 * scale, 4 * scale, 0,
                                     fcolour, fcolour, 1.0);
            } else if (c == 0x60) {
                graphics_draw_circle(surface, *x + modx + (width * scale) / 2,
                                     *y + mody + 9 * scale, 6 * scale, 2 * scale, fcolour, -1, 1.0);
            }
        }
    }

    switch (orientation) {
        case kOrientNormal:
            *x += width * scale;
            break;
        case kOrientVert:
            *y += height * scale;
            break;
        case kOrientInverted:
            *x -= width * scale;
            break;
        case kOrientCounterClock:
            *y -= width * scale;
            break;
        case kOrientClockwise:
            *y += width * scale;
            break;
        default:
            return kInternalFault;
    }

    return kOk;
}

uint32_t graphics_font_width(uint32_t font) {
    const uint32_t font_number = font >> 4;
    const uint32_t scaling = font & 0b1111;
    if (font_number >= FONT_TABLE_SIZE || !FontTable[font_number]) return 0;
    return FontTable[font_number][0] * scaling;
}

uint32_t graphics_font_height(uint32_t font) {
    const uint32_t font_number = font >> 4;
    const uint32_t scaling = font & 0b1111;
    if (font_number >= FONT_TABLE_SIZE || !FontTable[font_number]) return 0;
    return FontTable[font_number][1] * scaling;
}

MmResult graphics_draw_string(MmSurface *surface, int x, int y, uint32_t font, TextHAlign jh,
                              TextVAlign jv, TextOrientation jo, MmGraphicsColour fcolour,
                              MmGraphicsColour bcolour, const char *s) {
    switch (jo) {
        case kOrientNormal:
            if (jh == kAlignCenter) x -= (strlen(s) * graphics_font_width(font)) / 2;
            if (jh == kAlignRight)  x -= (strlen(s) * graphics_font_width(font));
            if (jv == kAlignMiddle) y -= graphics_font_height(font) / 2;
            if (jv == kAlignBottom) y -= graphics_font_height(font);
            break;
        case kOrientVert:
            if (jh == kAlignCenter) x -= graphics_font_width(font) / 2;
            if (jh == kAlignRight)  x -= graphics_font_width(font);
            if (jv == kAlignMiddle) y -= (strlen(s) * graphics_font_height(font)) / 2;
            if (jv == kAlignBottom) y -= (strlen(s) * graphics_font_height(font));
            break;
        case kOrientInverted:
            if (jh == kAlignCenter) x += (strlen(s) * graphics_font_width(font)) / 2;
            if (jh == kAlignRight)  x += (strlen(s) * graphics_font_width(font));
            if (jv == kAlignMiddle) y += graphics_font_height(font) / 2;
            if (jv == kAlignBottom) y += graphics_font_height(font);
            break;
        case kOrientCounterClock:
            if (jh == kAlignCenter) x -= graphics_font_height(font) / 2;
            if (jh == kAlignRight)  x -= graphics_font_height(font);
            if (jv == kAlignMiddle) y += (strlen(s) * graphics_font_width(font)) / 2;
            if (jv == kAlignBottom) y += (strlen(s) * graphics_font_width(font));
            break;
        case kOrientClockwise:
            if (jh == kAlignCenter) x += graphics_font_height(font) / 2;
            if (jh == kAlignRight)  x += graphics_font_height(font);
            if (jv == kAlignMiddle) y -= (strlen(s) * graphics_font_width(font)) / 2;
            if (jv == kAlignBottom) y -= (strlen(s) * graphics_font_width(font));
            break;
        default:
            return kInternalFault;
    }

    MmResult result = kOk;
    while (*s && !FAILED(result)) {
        result = graphics_draw_char(surface, &x, &y, font, fcolour, bcolour, *s++, jo);
    }

    return result;
}

MmResult graphics_set_font(uint32_t font_number, uint32_t scale) {
    if (font_number > FONT_TABLE_SIZE - 1) {
        return kInvalidFont;
    } else if (!FontTable[font_number - 1]) {
        return kInvalidFont;
    } else if (scale > 15) {
        return kInvalidFontScaling;
    }
    graphics_font = (font_number << 4) | scale;
    return kOk;
}
