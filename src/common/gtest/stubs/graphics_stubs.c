/*
 * Copyright (c) 2024-2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <string.h>

#include "../../graphics.h"

MmSurface graphics_surfaces[GRAPHICS_MAX_SURFACES] = { 0 };
unsigned graphics_mode = 0;

MmResult graphics_init() {
    return kOk;
}

MmResult graphics_reset() {
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

