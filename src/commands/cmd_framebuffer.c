/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_framebuffer.c

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

#include <strings.h>

#include "../common/graphics.h"
#include "../common/mmb4l.h"

/** FRAMEBUFFER CLOSE [page_id] */
static void cmd_framebuffer_close(const char *p) {
    skipspace(p);
    MmResult result = kOk;
    MmSurface *surfaceF = &graphics_surfaces[GRAPHICS_SURFACE_F];
    MmSurface *surfaceL = &graphics_surfaces[GRAPHICS_SURFACE_L];
    if (parse_is_end(p)) {
        result = graphics_surface_destroy(surfaceF);
        if (SUCCEEDED(result)) result = graphics_surface_destroy(surfaceL);
    } else {
        MmSurfaceId page_id = -1;
        result = parse_page(p, &page_id);
        if (SUCCEEDED(result)) {
            switch (page_id) {
                case GRAPHICS_SURFACE_N:
                    result = kGraphicsInvalidSurface;
                    break;
                case GRAPHICS_SURFACE_F:
                    result = graphics_surface_destroy(surfaceF);
                    break;
                case GRAPHICS_SURFACE_L:
                    result = graphics_surface_destroy(surfaceL);
                    break;
                default:
                    result = kInternalFault;
                    break;
            }
        }
    }

    ERROR_ON_FAILURE(result);
}

/** FRAMEBUFFER COPY from, to [,B] */
static void cmd_framebuffer_copy(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 3 && argc != 5) {
        ERROR_ARGUMENT_COUNT;
        return;
    }

    MmSurfaceId read_id = -1;
    ERROR_ON_FAILURE(parse_read_page(argv[0], &read_id));
    MmSurface* read_surface = &graphics_surfaces[read_id];

    MmSurfaceId write_id = -1;
    ERROR_ON_FAILURE(parse_write_page(argv[2], &write_id));
    MmSurface* write_surface = &graphics_surfaces[write_id];

    // MMB4L ignores the background flag B for the moment.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    bool background = false;
#pragma GCC diagnostic pop
    if (argc == 5) {
        const char *tp;
        if ((tp = checkstring(argv[4], "B"))) {
            background = true;
        } else { // Allow string expression.
            const char *s= getCstring(argv[4]);
            if (strcasecmp(s, "B")) {
                background = true;
            } else {
                ERROR_ON_FAILURE(kSyntax);
            }
        }
    }

    if (read_surface->width != write_surface->width || read_surface->height != write_surface->height) {
        ERROR_ON_FAILURE(kGraphicsSurfaceSizeMismatch);
    }

    if (read_surface == write_surface) {
        ERROR_ON_FAILURE(kGraphicsReadAndWriteSurfaceSame);
    }

    ERROR_ON_FAILURE(
            graphics_blit(0, 0, 0, 0, read_surface->width, read_surface->height, read_surface,
                          write_surface, 0x0, -1));
}

/** FRAMEBUFFER CREATE */
static void cmd_framebuffer_create(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) error_throw(kUnexpectedText);
    MmResult result = graphics_buffer_create(GRAPHICS_SURFACE_F, 320, 240);
    if (FAILED(result)) error_throw(result);
}

/** FRAMEBUFFER LAYER [transparent]*/
static void cmd_framebuffer_layer(const char *p) {
    skipspace(p);
    uint8_t transparent = parse_is_end(p) ? 0 : getint(p, 0, 15);

    MmResult result = graphics_buffer_create(GRAPHICS_SURFACE_L, 320, 240);
    if (FAILED(result)) error_throw(result);

    MmSurface *layer = &graphics_surfaces[GRAPHICS_SURFACE_L];
    layer->transparent = GRAPHICS_RGB121_COLOURS[transparent];
    result = graphics_draw_rectangle(layer, 0, 0, layer->width - 1, layer->height - 1,
                                     layer->transparent);
    if (FAILED(result)) error_throw(result);
}

/** FRAMEBUFFER MERGE [colour] [, mode] [, update rate] */
static void cmd_framebuffer_merge(const char *p) {
    getargs(&p, 5, ",");
    if (argc == 2 || argc == 4) {
        ERROR_ARGUMENT_COUNT;
        return;
    }
    uint8_t transparent = (argc > 0) ? getint(argv[0], 0, 15) : 0;
    // MMB4L ignores the 'mode' and 'update rate' arguments for the moment.

    // Copy F to N.
    MmSurface* read_surface = &graphics_surfaces[GRAPHICS_SURFACE_F];
    MmSurface* write_surface = &graphics_surfaces[GRAPHICS_SURFACE_N];
    MmResult result = graphics_blit(0, 0, 0, 0, read_surface->width, read_surface->height,
                                    read_surface, write_surface, 0x0, RGB_BLACK);
    if (FAILED(result)) error_throw(result);

    // Merge L with N.
    read_surface = &graphics_surfaces[GRAPHICS_SURFACE_L];
    result = graphics_blit(0, 0, 0, 0, read_surface->width, read_surface->height, read_surface,
                           write_surface, 0x4, GRAPHICS_RGB121_COLOURS[transparent]);
    if (FAILED(result)) error_throw(result);
}

/** FRAMEBUFFER WAIT */
static void cmd_framebuffer_wait(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) error_throw(kUnexpectedText);
    // Empty implementation for now.
    // On the real PicoMiteVGA this waits until the start of the next frame blanking period.
}

/** FRAMEBUFFER WRITE { N | F | L } */
static void cmd_framebuffer_write(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ON_FAILURE(kArgumentCount);

    MmSurfaceId write_id = -1;
    ERROR_ON_FAILURE(parse_write_page(argv[0], &write_id));
    ERROR_ON_FAILURE(graphics_surface_write(write_id));
}

void cmd_framebuffer(void) {
    if (mmb_options.simulate != kSimulateGameMite
            && mmb_options.simulate != kSimulatePicoMiteVga) {
        ERROR_ON_FAILURE(kUnsupportedOnCurrentDevice);
    }

    const char *p;
    if ((p = checkstring(cmdline, "CLOSE"))) {
        cmd_framebuffer_close(p);
    } else if ((p = checkstring(cmdline, "COPY"))) {
        cmd_framebuffer_copy(p);
    } else if ((p = checkstring(cmdline, "CREATE"))) {
        cmd_framebuffer_create(p);
    } else if ((p = checkstring(cmdline, "LAYER"))) {
        cmd_framebuffer_layer(p);
    } else if ((p = checkstring(cmdline, "MERGE"))) {
        cmd_framebuffer_merge(p);
    } else if ((p = checkstring(cmdline, "WAIT"))) {
        cmd_framebuffer_wait(p);
    } else if ((p = checkstring(cmdline, "WRITE"))) {
        cmd_framebuffer_write(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("FRAMEBUFFER");
    }
}
