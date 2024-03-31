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

#include "cstring.h"
#include "error.h"
#include "events.h"
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

MmResult graphics_term() {
    MmResult result = graphics_surface_destroy_all();
    if (SUCCEEDED(result)) {
        graphics_fcolour = RGB_WHITE;
        graphics_bcolour = RGB_BLACK;
    }
    return result;
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

MmResult graphics_buffer_create(MmSurfaceId id, int width, int height) {
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

MmResult graphics_window_create(MmSurfaceId id, int x, int y, int width, int height, int scale) {
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

MmResult graphics_surface_destroy(MmSurface *surface) {
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
    for (MmSurfaceId id = 0; SUCCEEDED(result) && id <= GRAPHICS_MAX_ID; ++id) {
        result = graphics_surface_destroy(&graphics_surfaces[id]);
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
    if (x >= 0 && y >= 0 && x < surface->width && y < surface->height) {
        surface->pixels[y*surface->width + x] = colour;
    }
}

MmResult graphics_draw_pixel(MmSurface *surface, int x, int y, MmGraphicsColour colour) {
    graphics_set_pixel_safe(surface, x, y, colour);
    surface->dirty = true;
    return kOk;
}

MmResult graphics_draw_aa_line(MmSurface *surface, MMFLOAT x0, MMFLOAT y0, MMFLOAT x1, MMFLOAT y1,
                               int width, MmGraphicsColour colour) {
    ERROR_UNIMPLEMENTED("graphics_draw_aa_line");
    return kUnimplemented;
}

MmResult graphics_draw_box(MmSurface *surface, int x1, int y1, int x2, int y2, uint32_t width,
                           MmGraphicsColour colour, MmGraphicsColour fill) {
    MmResult result = kOk;

    // Make sure the coordinates are in the right sequence.
    if (x1 > x2) SWAP(int, x1, x2);
    if (y1 > y2) SWAP(int, x1, x2);

    width = min(min(width, (uint32_t) (x2 - x1)), (uint32_t) (y2 - y1));
    if (width > 0) {
        width--;
        result = graphics_draw_rectangle(surface, x1, y1, x2, y1 + width, colour);  // Top border.
        if (SUCCEEDED(result)) result = graphics_draw_rectangle(surface, x1, y2 - width, x2, y2,
                                                                colour);  // Bottom border.
        if (SUCCEEDED(result)) result = graphics_draw_rectangle(surface, x1, y1, x1 + width, y2,
                                                                colour);  // Left border.
        if (SUCCEEDED(result)) result = graphics_draw_rectangle(surface, x2 - width, y1, x2, y2,
                                                                colour);  // Right border.
        width++;
    }

    if (SUCCEEDED(result) && fill >= 0) graphics_draw_rectangle(surface, x1 + width, y1 + width,
                                                                x2 - width, y2 - width, fill);

    return result;
}

MmResult graphics_draw_buffered(MmSurface *surface, int xti, int yti, MmGraphicsColour colour,
                                int complete) {
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

MmResult graphics_draw_circle(MmSurface *surface, int x, int y, int radius, int w,
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

MmResult graphics_draw_line(MmSurface *surface, int x1, int y1, int x2, int y2, int w,
                            MmGraphicsColour colour) {
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
    // if(Option.Refresh)Display_Refresh();
    surface->dirty = true;

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

    return kOk;
}

MmResult graphics_draw_rectangle(MmSurface *surface, int x1, int y1, int x2, int y2,
                                 MmGraphicsColour colour) {
    // Do not draw anything if entire rectangle is off the screen.
    if ((x1 < 0 && x2 < 0) || (y1 < 0 && y2 < 0) ||
        (x1 >= surface->width && x2 >= surface->width) ||
        (y1 >= surface->height && y2 >= surface->height)) {
        return kOk;
    }

    x1 = min(max(0, x1), HRes - 1);
    x2 = min(max(0, x2), HRes - 1);
    y1 = min(max(0, y1), VRes - 1);
    y2 = min(max(0, y2), VRes - 1);
    if (x1 > x2) SWAP(int, x1, x2);
    if (y1 > y2) SWAP(int, y1, y2);
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) graphics_set_pixel(surface, x, y, colour);
    }
    surface->dirty = true;
    return kOk;
}

MmResult graphics_draw_triangle(MmSurface *surface, int x0, int y0, int x1, int y1, int x2, int y2,
                                MmGraphicsColour colour, MmGraphicsColour fill) {
    if (x0 * (y1 - y2) + x1 * (y2 - y0) + x2 * (y0 - y1) == 0) {
        // points are co-linear i.e zero area
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
        graphics_draw_line(surface, x0, y0, x2, y2, 1, colour);
    }
    else {
        if (fill == -1) {
            // draw only the outline
            graphics_draw_line(surface, x0, y0, x1, y1, 1, colour);
            graphics_draw_line(surface, x1, y1, x2, y2, 1, colour);
            graphics_draw_line(surface, x2, y2, x0, y0, 1, colour);
        }
        else {
            //we are drawing a filled triangle which may also have an outline
            int a, b, y, last;

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

            // We only care about what is visible.
            y0 = min(max(0, y0), (int) (surface->height - 1));
            y1 = min(max(0, y1), (int) (surface->height - 1));
            y2 = min(max(0, y2), (int) (surface->height - 1));

            if (y1 == y2) {
                last = y1;                                          //Include y1 scanline
            }
            else {
                last = y1 - 1;                                      // Skip it
            }
            for (y = y0; y <= last; y++) {
                if (y1 == y0 || y2 == y0) continue; // Impose sanity.
                a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
                b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
                if (a > b)SWAP(int, a, b);
                graphics_draw_rectangle(surface, a, y, b, y, fill);
            }
            while (y <= y2) {
                if (y2 == y1 || y2 == y0) continue; // Impose sanity.
                a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
                b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
                if (a > b) SWAP(int, a, b);
                graphics_draw_rectangle(surface, a, y, b, y, fill);
                y = y + 1;
            }
            // we also need an outline but we do this last to overwrite the edge of the fill area
            if (colour != fill) {
                graphics_draw_line(surface, x0, y0, x1, y1, 1, colour);
                graphics_draw_line(surface, x1, y1, x2, y2, 1, colour);
                graphics_draw_line(surface, x2, y2, x0, y0, 1, colour);
            }
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
    //if(optiony)y1=maxH-1-y1;
    //if(optiony)y2=maxH-1-y2;
    // make sure the coordinates are kept within the display area
    if (x2 <= x1) SWAP(int, x1, x2);
    if (y2 <= y1) SWAP(int, y1, y2);
    // int cursorhidden=0;
    // if(cursoron)
    //     if( !(xcursor + wcursor < x1 ||
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
    //             if(x>=0 && x<maxW && y>=0 && y<maxH*2){
    //                 if(skip & 2){
    //                     c.rgbbytes[3]=0xFF;
    //                     c.rgbbytes[2]=*p++; //this order swaps the bytes to match the .BMP file
    //                     c.rgbbytes[1]=*p++;
    //                     c.rgbbytes[0]=*p++;
    //                     if(skip & 1)c.rgbbytes[3]=*p++; //ARGB8888 so set transparency
    //                 } else {
    //                     c.rgbbytes[3]=0;
    //                     c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
    //                     c.rgbbytes[1]=*p++;
    //                     c.rgbbytes[2]=*p++;
    //                     if(skip & 1)p++;
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
    // if(cursorhidden)showcursor(0, xcursor,ycursor);
}

MmResult graphics_load_png(MmSurface *surface, char *filename, int x, int y, int transparent,
                           int force) {
    if (strchr(filename, '.') == NULL) cstring_cat(filename, ".PNG", STRINGSIZE);
    upng_t *upng = upng_new_from_file(filename);
    // routinechecks(1);
    upng_header(upng);
    const int w = upng_get_width(upng);
    const int h = upng_get_height(upng);
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
