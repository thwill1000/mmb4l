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
