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

#include "events.h"
#include "graphics.h"
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

MmResult graphics_draw_rectangle(MmSurface *surface, int x1, int y1, int x2, int y2, MmGraphicsColour colour) {
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
        for (int x = x1; x <= x2; x++) graphics_set_pixel(surface, x, y, colour);
    }
    surface->dirty = true;
    return kOk;
}