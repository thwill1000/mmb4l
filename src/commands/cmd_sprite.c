/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_sprite.c

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

#include "../common/error.h"
#include "../common/interrupt.h"
#include "../common/mmb4l.h"
#include "../common/sprite.h"

MmResult cmd_blit_compressed(const char *p);
MmResult cmd_blit_framebuffer(const char *p);
MmResult cmd_blit_memory(const char *p);

/** SPRITE CLOSE [#]id */
static MmResult cmd_sprite_close(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) return kArgumentCount;
    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[0], true, &sprite_id);
    if (SUCCEEDED(result)) result = sprite_destroy(&graphics_surfaces[sprite_id]);
    return result;
}

/** SPRITE CLOSE ALL */
static MmResult cmd_sprite_close_all(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) return kUnexpectedText;
    return sprite_destroy_all();
}

/** SPRITE HIDE [#]id */
static MmResult cmd_sprite_hide(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[0], true, &sprite_id);
    if (FAILED(result)) return result;
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    return sprite_hide(sprite);
}

/** SPRITE INTERRUPT interrupt */
static MmResult cmd_sprite_interrupt(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) return kArgumentCount;
    const char* interrupt_addr = GetIntAddress(argv[0]);
    interrupt_enable(kInterruptSpriteCollision, interrupt_addr);
    return kOk;
}

/** SPRITE HIDE ALL */
static inline MmResult cmd_sprite_hide_all(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) return kUnexpectedText;
    return sprite_hide_all();
}

/** SPRITE HIDE SAFE [#]id */
static MmResult cmd_sprite_hide_safe(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[0], true, &sprite_id);
    if (FAILED(result)) return result;
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    return sprite_hide_safe(sprite);
}

/** SPRITE LOAD file$ [, start_sprite] [, colour_mode] */
static MmResult cmd_sprite_load(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 1 && argc !=3 && argc != 5) return kArgumentCount;

    const char *file = getCstring(argv[0]);
    uint8_t start_sprite_id = 0;
    if (mmb_options.simulate == kSimulateMmb4l) {
        if (argc >= 3) start_sprite_id = getint(argv[2], 0, GRAPHICS_MAX_ID);
    } else {
        start_sprite_id = 1;
        if (argc >= 3) start_sprite_id = getint(argv[2], 1, CMM2_SPRITE_COUNT);
        start_sprite_id += CMM2_SPRITE_BASE;
    }
    uint8_t colour_mode = (argc >= 5) ? getint(argv[4], 0, 1) : 0;
    return graphics_load_sprite(file, start_sprite_id, colour_mode);
}

/** SPRITE MOVE */
static MmResult cmd_sprite_move(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) return kUnexpectedText;
    return sprite_move();
}

/** SPRITE NEXT [#]id, x, y */
static MmResult cmd_sprite_next(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 5) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[0], false, &sprite_id);
    if (FAILED(result)) return result;
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    sprite->next_x = getint(argv[2], 1 - sprite->width, graphics_current->width - 1);
    sprite->next_y = getint(argv[4], 1 - sprite->height, graphics_current->height - 1);

    return kOk;
}

/** SPRITE NOINTERRUPT */
static MmResult cmd_sprite_nointerrupt(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) return kUnexpectedText;
    interrupt_disable(kInterruptSpriteCollision);
    return kOk;
}

/** SPRITE READ [#]id, x, y, w, h [, src_id] */
static MmResult cmd_sprite_read(const char *p) {
    getargs(&p, 11, ",");
    if (argc != 9 && argc != 11) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[0], false, &sprite_id);
    if (FAILED(result)) return result;
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    int x = getint(argv[2], 0, WINDOW_MAX_X);
    int y = getint(argv[4], 0, WINDOW_MAX_Y);
    int w = getint(argv[6], 0, WINDOW_MAX_WIDTH);
    int h = getint(argv[8], 0, WINDOW_MAX_HEIGHT);

    MmSurfaceId src_id = -1;
    result = (argc == 11) ? parse_read_page(argv[10], &src_id) : kOk;
    if (FAILED(result)) return result;
    MmSurface *src_surface = (src_id == -1) ? graphics_current : &graphics_surfaces[src_id];
    if (src_surface->type == kGraphicsNone) return kGraphicsInvalidReadSurface;

    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > src_surface->width) w = src_surface->width - x;
    if (y + w > src_surface->height) h = src_surface->height - y;
    if (w < 1 || h < 1 || x < 0 || x + w > src_surface->width || y < 0
            || y + h > src_surface->height) return kOk;

    switch (sprite->type) {
        case kGraphicsNone: {
            MmResult result = graphics_sprite_create(sprite_id, w, h);
            if (FAILED(result)) return result;
            break;
        }

        case kGraphicsSprite:
        case kGraphicsInactiveSprite: {
            if (sprite->width != w || sprite->height != h) {
                return kGraphicsSurfaceSizeMismatch;
            }
            break;
        }

        case kGraphicsBuffer:
        case kGraphicsWindow: {
            return kGraphicsInvalidSprite;
        }

        default: {
            return kInternalFault;
        }
    }

    return graphics_blit(x, y, 0, 0, w, h, src_surface, sprite, 0x0, RGB_BLACK);
}

/** SPRITE RESTORE */
static MmResult cmd_sprite_restore(const char *p) {
    return sprite_restore_all();
}

/** SPRITE SCROLL x, y [, colour] */
MmResult cmd_sprite_scroll(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 3 && argc != 5) return kArgumentCount;
    const int maxW = graphics_current->width;
    const int maxH = graphics_current->height;
    const int x = getint(argv[0], -maxW / 2 - 1, maxW);
    const int y = getint(argv[2], -maxH / 2 - 1, maxH);
    MmGraphicsColour colour = -2; // Background wraps around.
    if (argc == 5) {
        switch (mmb_options.simulate) {
            case kSimulateGameMite:
            case kSimulatePicoMiteVga:
                colour = getint(argv[4], -2, 15);
                if (colour >= 0) colour = GRAPHICS_RGB121_COLOURS[colour];
                break;
            default:
                colour = getint(argv[4], -2, RGB_WHITE);
                break;
        }
    }
    return sprite_scroll(x, y, colour);
}

/** SPRITE SET TRANSPARENT rgb121_colour */
static MmResult cmd_sprite_set_transparent(const char *p) {
    uint8_t rgb121_colour = getint(p, 0, 15);
    return sprite_set_transparent_colour(GRAPHICS_RGB121_COLOURS[rgb121_colour]);
}

/**
 * SPRITE SHOW [#]id, x, y, layer [, flags]
 *
 * 'flags' is a bitwise AND of:
 *     0x01 = mirrored left to right.
 *     0x02 = mirrored top to bottom.
 *     0x04 = no transparency, all pixels opaque.
 */
static MmResult cmd_sprite_show(const char *p) {
    getargs(&p, 9, ",");
    if (argc != 7 && argc != 9) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[0], true, &sprite_id);
    if (FAILED(result)) return result;
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    const int x = getint(argv[2], -sprite->width + 1, graphics_current->width - 1);
    const int y = getint(argv[4], -sprite->height + 1, graphics_current->height - 1);
    const unsigned layer = getint(argv[6], 0, GRAPHICS_MAX_LAYER);
    const unsigned flags = (argc == 9) ? getint(argv[8], 0, 7) : 0;

    result = sprite_show(sprite, graphics_current, x, y, layer, flags);
    if (SUCCEEDED(result)) {
        result = sprite_update_collisions(sprite);
    }

    return result;
}

/**
 * SPRITE SHOW SAFE [#]id, x, y, layer [, flags] [, ontop]
 *
 * 'flags' is a bitwise AND of:
 *     0x01 = mirrored left to right.
 *     0x02 = mirrored top to bottom.
 *     0x04 = no transparency, all pixels opaque.
 */
static MmResult cmd_sprite_show_safe(const char *p) {
    getargs(&p, 11, ",");
    if (argc != 7 && argc != 9 && argc != 11) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[0], true, &sprite_id);
    if (FAILED(result)) return result;
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    const int x = getint(argv[2], -sprite->width + 1, graphics_current->width - 1);
    const int y = getint(argv[4], -sprite->height + 1, graphics_current->height - 1);
    const unsigned layer = getint(argv[6], 0, GRAPHICS_MAX_LAYER);
    const unsigned flags = (argc == 9) ? getint(argv[8], 0, 7) : 0;
    const unsigned ontop = (argc == 11) ? getint(argv[10], 0, 1) : 0;

    result = sprite_show_safe(sprite, graphics_current, x, y, layer, flags, ontop ? true : false);
    if (SUCCEEDED(result)) result = sprite_update_collisions(sprite);

    return result;
}

/**
 * SPRITE WRITE [#]id, x, y [, flags]
 *
 * 'flags' is a Bitwise AND of:
 *     0x01 = mirrored left to right.
 *     0x02 = mirrored top to bottom.
 *     0x04 = don't copy transparent pixels
 * Where 0x04 is the default when unspecified.
 */
MmResult cmd_sprite_write(const char *p) {
    getargs(&p, 7, ",");
    if (argc != 5 && argc != 7) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[0], true, &sprite_id);
    if (FAILED(result)) return result;
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    const int x = getint(argv[2], -sprite->width + 1, WINDOW_MAX_X);
    const int y = getint(argv[4], -sprite->height + 1, WINDOW_MAX_Y);
    unsigned flags = (argc == 7) ? getint(argv[6], 0, 7) : 0x04;

    return graphics_blit(0, 0, x, y, sprite->width, sprite->height, sprite, graphics_current, flags,
                         sprite_transparent_colour);
}

#define ELSE_IF_UNIMPLEMENTED(s) \
    else if ((p = checkstring(cmdline, s))) { \
        ERROR_UNIMPLEMENTED("SPRITE " s); \
    }

void cmd_sprite(void) {
    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(cmdline, "CLOSE ALL"))) {
        result = cmd_sprite_close_all(p);
    } else if ((p = checkstring(cmdline, "CLOSE"))) {
        result = cmd_sprite_close(p);
    } else if ((p = checkstring(cmdline, "COMPRESSED"))) {
        result = cmd_blit_compressed(p);
    } else if ((p = checkstring(cmdline, "FRAMEBUFFER"))) {
        result = cmd_blit_framebuffer(p);
    } else if ((p = checkstring(cmdline, "LOAD"))) {
        result = cmd_sprite_load(p);
    } else if ((p = checkstring(cmdline, "HIDE ALL"))) {
        result = cmd_sprite_hide_all(p);
    } else if ((p = checkstring(cmdline, "HIDE SAFE"))) {
        result = cmd_sprite_hide_safe(p);
    } else if ((p = checkstring(cmdline, "HIDE"))) {
        result = cmd_sprite_hide(p);
    } else if ((p = checkstring(cmdline, "INTERRUPT"))) {
        result = cmd_sprite_interrupt(p);
    } else if ((p = checkstring(cmdline, "MEMORY"))) {
        result = cmd_blit_memory(p);
    } else if ((p = checkstring(cmdline, "MOVE"))) {
        result = cmd_sprite_move(p);
    } else if ((p = checkstring(cmdline, "NEXT"))) {
        result = cmd_sprite_next(p);
    } else if ((p = checkstring(cmdline, "NOINTERRUPT"))) {
        result = cmd_sprite_nointerrupt(p);
    } else if ((p = checkstring(cmdline, "READ"))) {
        result = cmd_sprite_read(p);
    } else if ((p = checkstring(cmdline, "RESTORE"))) {
        result = cmd_sprite_restore(p);
    } else if ((p = checkstring(cmdline, "SCROLL"))) {
        result = cmd_sprite_scroll(p);
    } else if ((p = checkstring(cmdline, "SET TRANSPARENT"))) {
        result = cmd_sprite_set_transparent(p);
    } else if ((p = checkstring(cmdline, "SHOW SAFE"))) {
        result = cmd_sprite_show_safe(p);
    } else if ((p = checkstring(cmdline, "SHOW"))) {
        result = cmd_sprite_show(p);
    } else if ((p = checkstring(cmdline, "WRITE"))) {
        result = cmd_sprite_write(p);
    }
    ELSE_IF_UNIMPLEMENTED("COPY")
    ELSE_IF_UNIMPLEMENTED("LOADARRAY")
    ELSE_IF_UNIMPLEMENTED("LOADPNG")
    ELSE_IF_UNIMPLEMENTED("SCROLLR")
    ELSE_IF_UNIMPLEMENTED("SWAP")
    ELSE_IF_UNIMPLEMENTED("TRANSPARENCY")
    else {
        ERROR_UNKNOWN_SUBCOMMAND("SPRITE");
    }

    ERROR_ON_FAILURE(result);
}
