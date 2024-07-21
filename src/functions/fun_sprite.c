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

/** SPRITE(A, [#]n) */
static MmResult fun_sprite_address(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[2], true, &sprite_id);
    if (SUCCEEDED(result)) {
        targ = T_INT;
        iret = (MMINTEGER) graphics_surfaces[sprite_id].pixels;
    }
    return result;
}

/** SPRITE(C, [#]n [, m]) */
static MmResult fun_sprite_collision(int argc, char **argv) {
    if (argc != 3 && argc != 5) return kArgumentCount;

    // We allow n == -1 (or 0 on CMM2/PicoMite/etc.)
    MmSurfaceId sprite_id = getinteger(argv[2]);
    if (mmb_options.simulate == kSimulateMmb4l) {
        if (sprite_id < -1 || sprite_id > GRAPHICS_MAX_ID) return kGraphicsInvalidSprite;
    } else {
        if (sprite_id < 0 || sprite_id > 64) return kGraphicsInvalidSprite;
        if (sprite_id == 0) sprite_id = -1;
    }

    MmResult result = kOk;
    if (sprite_id != -1) result = parse_sprite_id(argv[2], true, &sprite_id);
    targ = T_INT;
    if (SUCCEEDED(result)) {
        MmSurface *sprite = (sprite_id == -1) ? NULL : &graphics_surfaces[sprite_id];
        if (argc == 3) {
            uint32_t count = 0;
            result = sprite
                    ? sprite_get_num_collisions(sprite, &count)
                    : sprite_get_num_collided_sprites(&count);
            if (SUCCEEDED(result)) iret = (MMINTEGER) count;
        } else {
            MmSurfaceId id = -1;
            const uint32_t m = getint(argv[4], 1, GRAPHICS_MAX_SURFACES);
            result = sprite
                    ? sprite_get_collision(sprite, m, &id)
                    : sprite_get_collided_sprite(m, &id);
            if (SUCCEEDED(result)) iret = (MMINTEGER) id;
            if (mmb_options.simulate != kSimulateMmb4l) {
                iret = (iret == -1) ? 0 : iret - CMM2_SPRITE_BASE;
            }
        }
    }
    return result;
}

/** SPRITE(D, [#]s1, [#]s2) */
static MmResult fun_sprite_distance(int argc, char **argv) {
    if (argc != 5) return kArgumentCount;
    MmSurfaceId sprite_id1 = -1, sprite_id2 = -1;
    MmResult result = parse_sprite_id(argv[2], true, &sprite_id1);
    if (SUCCEEDED(result)) result = parse_sprite_id(argv[4], true, &sprite_id2);
    if (SUCCEEDED(result)) {
        targ = T_NBR;
        MmSurface *sprite1 = &graphics_surfaces[sprite_id1];
        MmSurface *sprite2 = &graphics_surfaces[sprite_id2];
        if (sprite1->type == kGraphicsSprite && sprite2->type == kGraphicsSprite) {
            const int32_t x1 = sprite1->x + (sprite1->width / 2);
            const int32_t y1 = sprite1->y + (sprite1->height / 2);
            const int32_t x2 = sprite2->x + (sprite2->width / 2);
            const int32_t y2 = sprite2->y + (sprite2->height / 2);
            fret = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
        } else {
            fret = (MMFLOAT) -1.0;
        }
    }
    return result;
}

/**
 * SPRITE(E, [#]n)
 *
 * Gets a bitmask of the edges that a sprite is in collision with.
 *
 * If not a sprite, or not active then returns 0.
 */
static MmResult fun_sprite_edges(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[2], false, &sprite_id);
    if (SUCCEEDED(result)) {
        MmSurface *sprite = &graphics_surfaces[sprite_id];
        targ = T_INT;
        iret = (sprite->type == kGraphicsSprite) ? (MMINTEGER) sprite->edge_collisions : 0x0;
    }
    return result;
}

/**
 * SPRITE(H, [#]n)
 *
 * Gets the height of a sprite.
 *
 * If not a sprite, or not active then returns -1.
 */
static MmResult fun_sprite_height(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[2], false, &sprite_id);
    if (SUCCEEDED(result)) {
        MmSurface *sprite = &graphics_surfaces[sprite_id];
        targ = T_INT;
        iret = (sprite->type == kGraphicsSprite) ? sprite->height : -1;
    }
    return result;
}

/**
 * SPRITE(L, [#]n)
 *
 * Gets the layer of a sprite.
 *
 * If not a sprite, or not active then returns -1.
 */
static MmResult fun_sprite_layer(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[2], false, &sprite_id);
    if (SUCCEEDED(result)) {
        MmSurface *sprite = &graphics_surfaces[sprite_id];
        targ = T_INT;
        iret = (sprite->type == kGraphicsSprite) ? (MMINTEGER) sprite->layer : -1;
    }
    return result;
}

/** SPRITE(N [, n]) */
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

/** SPRITE(S) */
static MmResult fun_sprite_last_collision(int argc, char **argv) {
    if (argc != 1) return kArgumentCount;
    targ = T_INT;
    iret = (MMINTEGER) sprite_last_collision;
    if (mmb_options.simulate != kSimulateMmb4l) {
        switch (iret) {
            case SPRITE_NO_COLLISION:
                iret = -1;
                break;
            case SPRITE_SCROLL_COLLISION:
                iret = 0;
                break;
            default:
                iret -= CMM2_SPRITE_BASE;
                break;
        }
    }
    return kOk;
}

/** SPRITE(V, [#]s1, [#]s2) */
static MmResult fun_sprite_vector(int argc, char **argv) {
    if (argc != 5) return kArgumentCount;
    MmSurfaceId sprite_id1 = -1, sprite_id2 = -1;
    MmResult result = parse_sprite_id(argv[2], true, &sprite_id1);
    if (SUCCEEDED(result)) result = parse_sprite_id(argv[4], true, &sprite_id2);
    if (SUCCEEDED(result)) {
        MmSurface *sprite1 = &graphics_surfaces[sprite_id1];
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
    }
    return result;
}

/** SPRITE(T, [#]n) */
static MmResult fun_sprite_touching(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[2], true, &sprite_id);
    if (SUCCEEDED(result)) {
        targ = T_INT;
        MmSurface *sprite = &graphics_surfaces[sprite_id];
        if (sprite->type == kGraphicsSprite) {
            result = sprite_get_collision_bitset(sprite, CMM2_SPRITE_BASE + 1, (uint64_t *) &iret);
        } else {
            iret = 0x0;
        }
    }
    return result;
}

/**
 * SPRITE(W, [#]n)
 *
 * Gets the width of a sprite.
 *
 * If not a sprite, or not active then returns -1.
 */
static MmResult fun_sprite_width(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[2], false, &sprite_id);
    if (SUCCEEDED(result)) {
        MmSurface *sprite = &graphics_surfaces[sprite_id];
        targ = T_INT;
        iret = (sprite->type == kGraphicsSprite) ? (MMINTEGER) sprite->width : -1;
    }
    return result;
}

/**
 * SPRITE(X, [#]n)
 *
 * Gets the x-coordinate of a sprite.
 *
 * If not a sprite, or not active then returns 10000.
 */
static MmResult fun_sprite_x(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[2], false, &sprite_id);
    if (SUCCEEDED(result)) {
        MmSurface *sprite = &graphics_surfaces[sprite_id];
        targ = T_INT;
        iret = (sprite->type == kGraphicsSprite) ? (MMINTEGER) sprite->x : GRAPHICS_OFF_SCREEN;
    }
    return result;
}

/**
 * SPRITE(Y, [#]n)
 *
 * Gets the y-coordinate of a sprite.
 *
 * If not a sprite, or not active then returns 10000.
 */
static MmResult fun_sprite_y(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurfaceId sprite_id = -1;
    MmResult result = parse_sprite_id(argv[2], false, &sprite_id);
    if (SUCCEEDED(result)) {
        MmSurface *sprite = &graphics_surfaces[sprite_id];
        targ = T_INT;
        iret = (sprite->type == kGraphicsSprite) ? (MMINTEGER) sprite->y : GRAPHICS_OFF_SCREEN;
    }
    return result;
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

    ERROR_ON_FAILURE(result);
}
