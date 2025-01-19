/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_sprite.c

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

#include <math.h>
#include <string.h>

#include "../common/mmb4l.h"
#include "../common/sprite.h"
#include "../common/utility.h"

/** SPRITE(A, [#]id) */
static MmResult fun_sprite_address(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[2], kParseSpriteIdMustExist, &sprite_id));

    targ = T_INT;
    iret = (uintptr_t) graphics_surfaces[sprite_id].pixels;

    return kOk;
}

/**
 * SPRITE(C, [#]sprite_id [, m])
 *
 * If m == 0 (unset) returns the number of currently active collisions for sprite_id.
 *   If sprite_id == 0 returns the number of sprites that have a currently
 *   active collision following a SPRITE SCROLL command
 *
 * If m != 0 returns the id of the sprite which caused the "m”th collision of sprite_id.
 *   If sprite_id == 0 returns the id of “m”th sprite that has a currently
 *   active collision following a SPRITE SCROLL command.
 *
 *   If the collision was with the edge of the screen then the return value will be:
 *     0xFFF1 - collision with left of screen.
 *     0xFFF2 - collision with top of screen.
 *     0xFFF4 - collision with right of screen.
 *     0xFFF8 - collision with bottom of screen.
 */
static MmResult fun_sprite_collision(int argc, char **argv) {
    if (argc != 3 && argc != 5) return kArgumentCount;

    MmSurfaceId surface_id = -1;
    MmResult result = parse_sprite_id(argv[2], kParseSpriteIdMustExist | kParseSpriteIdAllowZero,
                                      &surface_id);

    const int m = has_arg(4) ? getint(argv[4], 1, GRAPHICS_MAX_SURFACES) : -1;

#if defined(SPRITE_DEBUG)
    if (m == -1) {
        if (surface_id == 0) {
            printf("SPRITE(C, 0)\n");
        } else {
            printf("SPRITE(C, #%d (%d))\n", sprite_id_from_surface_id(surface_id), surface_id);
        }
    } else {
        if (surface_id == 0) {
            printf("SPRITE(C, 0, %d)\n", m);
        } else {
            printf("SPRITE(C, #%d (%d), %d)\n",
                   sprite_id_from_surface_id(surface_id), surface_id, m);
        }
    }
#endif

    targ = T_INT;
    MmSurfaceId id = -1;
    if (SUCCEEDED(result)) {
        MmSurface *sprite = (surface_id == 0) ? NULL : &graphics_surfaces[surface_id];

        if (m == -1) { // Number of collisions.
            uint32_t count = -1;
            if (sprite) { // Collision with a specific sprite
                result = sprite_get_num_collisions(sprite, &count);
            } else { // Collision after a SCROLL.
                result = sprite_get_num_collided_sprites(&count);
            }
            if (SUCCEEDED(result)) iret = (MMINTEGER) count;

#if defined(SPRITE_DEBUG)
            printf("  count = %d\n", count);
            for (uint32_t i = 1; i <= count; ++i) {
                if (sprite) {
                    sprite_get_collision(sprite, i, &id);
                } else {
                    sprite_get_collided_sprite(i, &id);
                }
                if (id == -1) {
                    printf("    %d) -1 (none)\n", i);
                } else if (id & 0xFF00) {
                    printf("    %d) 0x%X\n", i, id);
                } else {
                    printf("    %d) %d\n", i, id);
                }
            }
#endif

        } else { // m'th collision.
            if (sprite) { // Collision with a specific sprite
                result = sprite_get_collision(sprite, m, &id);
            } else { // Collision after a SCROLL.
                result = sprite_get_collided_sprite(m, &id);
            }
            // NOTE: id == -1 means there was no m'th collided sprite.
            if (SUCCEEDED(result)) {
                if (id == -1) {
                    iret = 0;
                } else if (id & 0xFF00) {
                    // Edge collision.
                    if (mmb_options.simulate == kSimulateMmb4l) {
                        iret = (MMINTEGER) id;
                    } else {
                        iret = (MMINTEGER) (id & 0xFF);
                    }
                } else {
                    iret = (MMINTEGER) sprite_id_from_surface_id(id);
                }
            }

#if defined(SPRITE_DEBUG)
            if (id == -1) {
                printf("  collision = -1 (none)\n");
            } else if (id & 0xFF00) {
                printf("  collision = 0x%X (#%lX)\n", id, iret);
            } else {
                printf("  collision = %d (#%ld)\n", id, iret);
            }
#endif
        }
    }

    // For compatibility with other MMBasic implementations despite
    // my initial inclination being that this should be an error.
    if (result == kGraphicsInvalidSprite) {
        iret = 0;
        result = kOk;
#if defined(SPRITE_DEBUG)
        printf("  Invalid sprite, returning %ld\n", iret);
#endif
    }

    return result;
}

/** SPRITE(D, [#]id1, [#]id2) */
static MmResult fun_sprite_distance(int argc, char **argv) {
    if (argc != 5) return kArgumentCount;

    MmSurfaceId sprite_id1 = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[2], kParseSpriteIdMustExist, &sprite_id1));
    MmSurface *sprite1 = &graphics_surfaces[sprite_id1];

    MmSurfaceId sprite_id2 = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[4], kParseSpriteIdMustExist, &sprite_id2));
    MmSurface *sprite2 = &graphics_surfaces[sprite_id2];

    targ = T_NBR;
    if (sprite1->type == kGraphicsSprite && sprite2->type == kGraphicsSprite) {
        const int32_t x1 = sprite1->x + (sprite1->width / 2);
        const int32_t y1 = sprite1->y + (sprite1->height / 2);
        const int32_t x2 = sprite2->x + (sprite2->width / 2);
        const int32_t y2 = sprite2->y + (sprite2->height / 2);
        fret = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    } else {
        fret = (MMFLOAT) -1.0;
    }

    return kOk;
}

/**
 * SPRITE(E, [#]id)
 *
 * Gets a bitmask of the edges that a sprite is in collision with.
 *
 * If not a sprite, or not active then returns 0.
 */
static MmResult fun_sprite_edges(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[2], kParseSpriteIdMustExist, &sprite_id));
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    targ = T_INT;
    iret = (sprite->type == kGraphicsSprite) ? (MMINTEGER) sprite->edge_collisions : 0x0;

    return kOk;
}

/**
 * SPRITE(H, [#]id)
 *
 * Gets the height of a sprite.
 *
 * If not a sprite, or not active then returns -1.
 */
static MmResult fun_sprite_height(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[2], 0x0, &sprite_id));
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    targ = T_INT;
    iret = (sprite->type == kGraphicsSprite) ? sprite->height : -1;

    return kOk;
}

/**
 * SPRITE(L, [#]id)
 *
 * Gets the layer of a sprite.
 *
 * If not a sprite, or not active then returns -1.
 */
static MmResult fun_sprite_layer(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[2], kParseSpriteIdMustExist, &sprite_id));
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    targ = T_INT;
    iret = (sprite->type == kGraphicsSprite) ? (MMINTEGER) sprite->layer : -1;

    return kOk;
}

/** SPRITE(N [, layer]) */
static MmResult fun_sprite_count(int argc, char **argv) {
    targ = T_INT;
    MmResult result = kOk;
    size_t total = 0;
    if (argc == 1) {
        result = sprite_count(&total);
    } else if (argc == 3) {
        int layer = getint(argv[2], 0, GRAPHICS_MAX_LAYER);
        result = sprite_count_on_layer(layer, &total);
    } else {
        result = kArgumentCount;
    }
    if (SUCCEEDED(result)) iret = total;
    return result;
}

/**
 * SPRITE(S)
 *
 * Returns the id of the last sprite which caused a collision.
 *   If -1 then no collision recorded.
 *   If 0 then the collision was the result of a SPRITE SCROLL command and the
 *   SPRITE(C ...) function should be used to find out how many and which sprites collided.
 */
static MmResult fun_sprite_last_collision(int argc, char **argv) {
    if (argc != 1) return kArgumentCount;
    targ = T_INT;
    switch (sprite_last_collision) {
        case SPRITE_NO_COLLISION:
            iret = -1;
            break;
        case SPRITE_SCROLL_COLLISION:
            iret = 0;
            break;
        default:
            iret = (MMINTEGER) sprite_id_from_surface_id(sprite_last_collision);
            break;
    }
    return kOk;
}

/** SPRITE(V, [#]id1, [#]id2) */
static MmResult fun_sprite_vector(int argc, char **argv) {
    if (argc != 5) return kArgumentCount;

    MmSurfaceId sprite_id1 = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[2], kParseSpriteIdMustExist, &sprite_id1));
    MmSurface *sprite1 = &graphics_surfaces[sprite_id1];

    MmSurfaceId sprite_id2 = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[4], kParseSpriteIdMustExist, &sprite_id2));
    MmSurface *sprite2 = &graphics_surfaces[sprite_id2];

    targ = T_NBR;
    if (sprite1->type == kGraphicsSprite && sprite2->type == kGraphicsSprite) {
        const int32_t x1 = sprite1->x + (sprite1->width / 2);
        const int32_t y1 = sprite1->y + (sprite1->height / 2);
        const int32_t x2 = sprite2->x + (sprite2->width / 2);
        const int32_t y2 = sprite2->y + (sprite2->height / 2);
        double vector = atan2(y2 - y1, x2 - x1) + M_PI_2;
        if (vector < 0) vector += (2 * M_PI);
        fret = (MMFLOAT) vector;
    } else {
        fret = (MMFLOAT) -1.0;
    }

    return kOk;
}

/** SPRITE(T, [#]id) */
static MmResult fun_sprite_touching(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[2], kParseSpriteIdMustExist, &sprite_id));
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    targ = T_INT;
    if (sprite->type == kGraphicsSprite) {
        ON_FAILURE_RETURN(sprite_get_collision_bitset(sprite, CMM2_SPRITE_BASE + 1,
                                                    (uint64_t *) &iret));
    } else {
        iret = 0x0;
    }

    return kOk;
}

/**
 * SPRITE(W, [#]id)
 *
 * Gets the width of a sprite.
 *
 * If not a sprite, or not active then returns -1.
 */
static MmResult fun_sprite_width(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[2], 0x0, &sprite_id));
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    targ = T_INT;
    iret = (sprite->type == kGraphicsSprite) ? (MMINTEGER) sprite->width : -1;

    return kOk;
}

/**
 * SPRITE(X, [#]id)
 *
 * Gets the x-coordinate of a sprite.
 *
 * If not a sprite, or not active then returns 10000.
 */
static MmResult fun_sprite_x(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[2], 0x0, &sprite_id));
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    targ = T_INT;
    iret = (sprite->type == kGraphicsSprite) ? (MMINTEGER) sprite->x : GRAPHICS_OFF_SCREEN;

    return kOk;
}

/**
 * SPRITE(Y, [#]id)
 *
 * Gets the y-coordinate of a sprite.
 *
 * If not a sprite, or not active then returns 10000.
 */
static MmResult fun_sprite_y(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;

    MmSurfaceId sprite_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[2], 0x0, &sprite_id));
    MmSurface *sprite = &graphics_surfaces[sprite_id];

    targ = T_INT;
    iret = (sprite->type == kGraphicsSprite) ? (MMINTEGER) sprite->y : GRAPHICS_OFF_SCREEN;

    return kOk;
}

void fun_sprite(void) {
    MmResult result = kOk;
    getargs(&ep, 5, ",");
    if (checkstring(argv[0], "A")) {
        result = fun_sprite_address(argc, argv);
    } else if (checkstring(argv[0], "C")) {
        result = fun_sprite_collision(argc, argv);
    } else if (checkstring(argv[0], "D")) {
        result = fun_sprite_distance(argc, argv);
    } else if (checkstring(argv[0], "E")) {
        result = fun_sprite_edges(argc, argv);
    } else if (checkstring(argv[0], "H")) {
        result = fun_sprite_height(argc, argv);
    } else if (checkstring(argv[0], "L")) {
        result = fun_sprite_layer(argc, argv);
    } else if (checkstring(argv[0], "N")) {
        result = fun_sprite_count(argc, argv);
    } else if (checkstring(argv[0], "S")) {
        result = fun_sprite_last_collision(argc, argv);
    } else if (checkstring(argv[0], "T")) {
        result = fun_sprite_touching(argc, argv);
    } else if (checkstring(argv[0], "V")) {
        result = fun_sprite_vector(argc, argv);
    } else if (checkstring(argv[0], "W")) {
        result = fun_sprite_width(argc, argv);
    } else if (checkstring(argv[0], "X")) {
        result = fun_sprite_x(argc, argv);
    } else if (checkstring(argv[0], "Y")) {
        result = fun_sprite_y(argc, argv);
    } else {
        ERROR_UNKNOWN_SUBFUNCTION("SPRITE");
    }

    ON_FAILURE_ERROR(result);
}
