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
#include "../common/parse.h"
#include "../common/sprite.h"
#include "../common/utility.h"

MmResult cmd_blit_compressed(const char *p);
MmResult cmd_blit_framebuffer(const char *p);
MmResult cmd_blit_memory(const char *p);
MmResult cmd_blit_read(const char *p, bool sprite);
MmResult cmd_blit_write(const char *p, bool sprite);

/** SPRITE CLOSE [#]id */
static MmResult cmd_sprite_close(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) return kArgumentCount;

    MmSurfaceId surface_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[0], kParseSpriteIdMustExist, &surface_id));
    MmSurface *sprite = &graphics_surfaces[surface_id];

#if defined(SPRITE_DEBUG)
    printf("SPRITE CLOSE #%d (%d)\n", sprite_id_from_surface_id(surface_id), surface_id);
#endif

    return sprite_destroy(sprite);
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

    MmSurfaceId surface_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[0], kParseSpriteIdMustExist, &surface_id));
    MmSurface *sprite = &graphics_surfaces[surface_id];

#if defined(SPRITE_DEBUG)
    printf("SPRITE HIDE #%d (%d)\n", sprite_id_from_surface_id(surface_id), surface_id);
#endif

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

    MmSurfaceId surface_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[0], kParseSpriteIdMustExist, &surface_id));
    MmSurface *sprite = &graphics_surfaces[surface_id];

#if defined(SPRITE_DEBUG)
    printf("SPRITE HIDE SAFE #%d (%d)\n", sprite_id_from_surface_id(surface_id), surface_id);
#endif

    return sprite_hide_safe(sprite);
}

/** SPRITE LOAD file$ [, start_sprite] [, colour_mode] */
static MmResult cmd_sprite_load(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 1 && argc !=3 && argc != 5) return kArgumentCount;

    char *filename = GetTempStrMemory();
    ON_FAILURE_RETURN(parse_filename(argv[0], filename, STRINGSIZE));

    const MmSurfaceId start_sprite_id = sprite_id_to_surface_id(
            has_arg(2) ? getint(argv[2], 1, sprite_max_id()) : 1);

    uint8_t colour_mode = has_arg(4) ? getint(argv[4], 0, 1) : 0;

    return graphics_load_sprite(filename, start_sprite_id, colour_mode);
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

    MmSurfaceId surface_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[0], kParseSpriteIdMustExist, &surface_id));
    MmSurface *sprite = &graphics_surfaces[surface_id];

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

/**
 * SPRITE READ [#]id, x, y, width, height [, src_id] 
 *  - <src_id> parameter unsupported on PicoMite{VGA}.
 */
static inline MmResult cmd_sprite_read(const char *p) {
    return cmd_blit_read(p, true);
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
 * 'flags' is a bitwise OR of:
 *     0x01 = mirrored left to right.
 *     0x02 = mirrored top to bottom.
 *     0x04 = no transparency, all pixels opaque.
 */
static MmResult cmd_sprite_show(const char *p) {
    getargs(&p, 9, ",");
    if (argc != 7 && argc != 9) return kArgumentCount;

    MmSurfaceId surface_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[0], kParseSpriteIdMustExist, &surface_id));
    MmSurface *sprite = &graphics_surfaces[surface_id];

    const int x = getint(argv[2], -sprite->width + 1, graphics_current->width - 1);
    const int y = getint(argv[4], -sprite->height + 1, graphics_current->height - 1);
    const unsigned layer = getint(argv[6], 0, GRAPHICS_MAX_LAYER);
    const int flags = has_arg(8) ? getint(argv[8], 0, 7) : -1;

#if defined(SPRITE_DEBUG)
    printf("SPRITE SHOW #%d (%d), %d, %d, layer = %d, flags = %x - surface %d\n",
           sprite_id_from_surface_id(surface_id), surface_id, x, y, layer, flags,
           graphics_current->id);
#endif

    // Invert transparency flag to match blit.
    ON_FAILURE_RETURN(sprite_show(sprite, graphics_current, x, y, layer,
                                  flags == -1 ? -1 : flags ^ kBlitWithTransparency));

    return sprite_update_collisions(sprite);
}

/**
 * SPRITE SHOW SAFE [#]id, x, y, layer [, flags] [, ontop]
 *
 * 'flags' is a bitwise OR of:
 *     0x01 = mirrored left to right.
 *     0x02 = mirrored top to bottom.
 *     0x04 = no transparency, all pixels opaque.
 */
static MmResult cmd_sprite_show_safe(const char *p) {
    getargs(&p, 11, ",");
    if (argc != 7 && argc != 9 && argc != 11) return kArgumentCount;

    MmSurfaceId surface_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[0], kParseSpriteIdMustExist, &surface_id));
    MmSurface *sprite = &graphics_surfaces[surface_id];

    const int x = getint(argv[2], -sprite->width + 1, graphics_current->width - 1);
    const int y = getint(argv[4], -sprite->height + 1, graphics_current->height - 1);
    const unsigned layer = getint(argv[6], 0, GRAPHICS_MAX_LAYER);
    const int flags = has_arg(8) ? getint(argv[8], 0, 7) : -1;
    const unsigned ontop = has_arg(10) ? getint(argv[10], 0, 1) : 0;

#if defined(SPRITE_DEBUG)
    if (surface_id != 128) {
        printf("SPRITE SHOW SAFE #%d (%d), %d, %d, layer = %d, flags = %x, ontop = %d - surface %d\n",
            sprite_id_from_surface_id(surface_id), surface_id, x, y, layer, flags, ontop,
            graphics_current->id);
    }
#endif

    // Invert transparency flag to match blit.
    ON_FAILURE_RETURN(sprite_show_safe(sprite, graphics_current, x, y, layer,
                                       flags == -1 ? -1 : flags ^ kBlitWithTransparency,
                                       ontop ? true : false));

    return sprite_update_collisions(sprite);
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
static inline MmResult cmd_sprite_write(const char *p) {
    return cmd_blit_write(p, true);
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
