/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <string.h>

#include "../../graphics.h"

MmSurface graphics_surfaces[GRAPHICS_MAX_SURFACES] = { 0 };

const char *graphics_last_error() { return ""; }

MmResult graphics_surface_destroy(MmSurface *surface) { return kOk; }

MmResult graphics_term(void) {
    memset(graphics_surfaces, 0x0, sizeof(graphics_surfaces));
    return kOk;
}
