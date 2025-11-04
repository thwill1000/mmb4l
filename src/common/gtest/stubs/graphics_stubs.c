/*
 * Copyright (c) 2024-2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <string.h>

#include "../../graphics.h"

MmSurface graphics_surfaces[GRAPHICS_MAX_SURFACES] = { 0 };
unsigned graphics_mode = 0;
MmSurface *graphics_current = NULL;
MmGraphicsColour graphics_bcolour;
MmGraphicsColour graphics_fcolour;
uint32_t graphics_font = (1 << 4) + 1; // Font 1, Scale 1.

MmResult graphics_cls(MmSurface *surface, MmGraphicsColour colour) {
    return kOk;
}

MmResult graphics_draw_box(MmSurface *surface, int x1, int y1, int x2, int y2, int width,
                           MmGraphicsColour colour, MmGraphicsColour fill) {
    return kOk;
}

MmResult graphics_draw_char(MmSurface *surface,  int *x, int *y, uint32_t font,
                            MmGraphicsColour fcolour, MmGraphicsColour bcolour, char c,
                            TextOrientation orientation) {
    return kOk;
}

MmResult graphics_draw_line(MmSurface *surface, int x1, int y1, int x2, int y2, int width,
                            MmGraphicsColour colour) {
    return kOk;
}

MmResult graphics_init() {
    return kOk;
}

void graphics_refresh_windows() {
}

MmResult graphics_reset() {
    return kOk;
}

MmResult graphics_scroll(MmSurface *surface, int x, int y, MmGraphicsColour fill) {
    return kOk;
}

MmResult graphics_term() {
    memset(graphics_surfaces, 0x0, sizeof(graphics_surfaces));
    return kOk;
}

MmResult graphics_set_mode(unsigned mode, unsigned colour_depth, MmGraphicsColour background) {
    return kOk;
}

MmResult graphics_surface_destroy(MmSurface *surface) {
    return kOk;
}

