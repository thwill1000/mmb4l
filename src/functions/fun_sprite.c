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

/**
 * Parses a sprite ID to get the corresponding MmSurface.
 *
 * @param[in]   p           Parse from this pointer.
 * @param[in]   allow_none  Is "no sprite" value allowed?
 *                           - The "no sprite" value for MMB4L = -1.
 *                           - The "no sprite" value for CMM2/PicoMite = 0.
 * @param[out]  sprite      Pointer to the corresponding sprite/surface, if "no sprite" then NULL.
 * @return                  kOk on success (including if the "no sprite" value was allowed and was
 *                          parsed),
 *                          kGraphicsNotASprite if the ID was valid, but not a sprite.
 */
static MmResult parse_sprite_id(char *p, bool allow_none, MmSurface **sprite) {
    if (*p == '#') p++;
    *sprite = NULL;
    if (mmb_options.simulate == kSimulateMmb4l) {
        MmSurfaceId sprite_id = getint(p, allow_none ? -1 : 0, GRAPHICS_MAX_ID);
        if (sprite_id != -1) *sprite = &graphics_surfaces[sprite_id];
    } else {
        MmSurfaceId sprite_id  = getint(p, allow_none ? 0 : 1, CMM2_SPRITE_COUNT);
        if (sprite_id != 0) *sprite = &graphics_surfaces[sprite_id + CMM2_SPRITE_BASE];
    }
    if (sprite) {
        return ((*sprite)->type == kGraphicsSprite) ? kOk : kGraphicsNotASprite;
    } else if (allow_none) {
        return kOk;
    } else {
        return kInternalFault;
    }
}

/** SPRITE(A, [#]n) */
static MmResult fun_sprite_address(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurface *sprite = NULL;
    MmResult result = parse_sprite_id(argv[2], false, &sprite);
    if (SUCCEEDED(result)) {
        targ = T_INT;
        iret = (MMINTEGER) sprite->pixels;
    }
    return result;
}

/** SPRITE(C, [#]n [, m]) */
static MmResult fun_sprite_collision(int argc, char **argv) {
    if (argc != 3 && argc != 5) return kArgumentCount;
    MmSurface *sprite = NULL;
    MmResult result = parse_sprite_id(argv[2], true, &sprite);
    targ = T_INT;
    if (SUCCEEDED(result)) {
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
                iret = (iret == -1) ? 0 : iret & 0xFF;
            }
        }
    }
    return result;
}

/** SPRITE(D, [#]s1, [#]s2) */
static MmResult fun_sprite_distance(int argc, char **argv) {
    if (argc != 5) return kArgumentCount;
    MmSurface *sprite1 = NULL, *sprite2 = NULL;
    MmResult result = parse_sprite_id(argv[2], false, &sprite1);
    if (SUCCEEDED(result)) result = parse_sprite_id(argv[4], false, &sprite2);
    if (SUCCEEDED(result)) {
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
    }
    return result;
}

/** SPRITE(E, [#]n) */
static MmResult fun_sprite_edges(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurface *sprite = NULL;
    MmResult result = parse_sprite_id(argv[2], false, &sprite);
    bool active = (result == kOk) && sprite->type == kGraphicsSprite;
    targ = T_INT;
    iret = active ? (MMINTEGER) sprite->edge_collisions : 0x0;
    return result == kGraphicsNotASprite ? kOk : result;
}

/** SPRITE(H, [#]n) */
static MmResult fun_sprite_height(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurface *sprite = NULL;
    MmResult result = parse_sprite_id(argv[2], false, &sprite);
    bool active = (result == kOk) && sprite->type == kGraphicsSprite;
    targ = T_INT;
    iret = active ? (MMINTEGER) sprite->height : -1;
    return result == kGraphicsNotASprite ? kOk : result;
}

/** SPRITE(L, [#]n) */
static MmResult fun_sprite_layer(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurface *sprite = NULL;
    MmResult result = parse_sprite_id(argv[2], false, &sprite);
    bool active = (result == kOk) && sprite->type == kGraphicsSprite;
    targ = T_INT;
    iret = active ? (MMINTEGER) sprite->layer : -1;
    return result == kGraphicsNotASprite ? kOk : result;
}

/** SPRITE(N [, n]) */
static MmResult fun_sprite_count(int argc, char **argv) {
    // TODO: Could be optimised so that instead of counting them up each time maintaining
    //       a running count of active sprites and active sprites per layer.
    targ = T_INT;
    iret = 0;
    if (argc == 1) {
        for (size_t id = 0; id <= GRAPHICS_MAX_ID; ++id) {
            if (graphics_surfaces[id].type == kGraphicsSprite) iret++;
        }
        return kOk;
    } else if (argc == 3) {
        uint8_t layer = getint(argv[2], 0, GRAPHICS_MAX_LAYER);
        for (size_t id = 0; id <= GRAPHICS_MAX_ID; ++id) {
            if (graphics_surfaces[id].type == kGraphicsSprite && graphics_surfaces[id].layer == layer) iret++;
        }
        return kOk;
    } else {
        return kArgumentCount;
    }
}

/** SPRITE(S) */
static MmResult fun_sprite_last_collision(int argc, char **argv) {
    if (argc != 1) return kArgumentCount;
    targ = T_INT;
    iret = (MMINTEGER) graphics_sprite_state.sprite_which_collided;
    return kOk;
}

/** SPRITE(V, [#]s1, [#]s2) */
static MmResult fun_sprite_vector(int argc, char **argv) {
    if (argc != 5) return kArgumentCount;
    MmSurface *sprite1 = NULL, *sprite2 = NULL;
    MmResult result = parse_sprite_id(argv[2], false, &sprite1);
    if (SUCCEEDED(result)) result = parse_sprite_id(argv[4], false, &sprite2);
    if (SUCCEEDED(result)) {
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
    MmSurface *sprite = NULL;
    MmResult result = parse_sprite_id(argv[2], false, &sprite);
    bool active = (result == kOk) && sprite->type == kGraphicsSprite;
    targ = T_INT;
    if (active) {
        result = sprite_get_collision_bitset(sprite, CMM2_SPRITE_BASE + 1, (uint64_t *) &iret);
        if (FAILED(result)) return result;
        // memcpy(&iret, sprite->sprite_collisions + 16 / sizeof(int), 8);
        // printf("sizeof(int) = %ld\n", sizeof(int));
        // printf("sprite->id = %d\n", sprite->id - 128);
        // printf("  0: 0b%010b\n", sprite->sprite_collisions[0]);
        // printf("  1: 0b%010b\n", sprite->sprite_collisions[1]);
        // printf("  2: 0b%010b\n", sprite->sprite_collisions[2]);
        // printf("  3: 0b%010b\n", sprite->sprite_collisions[3]);
        // printf("  4: 0b%010b\n", sprite->sprite_collisions[4]);
        // printf("  5: 0b%010b\n", sprite->sprite_collisions[5]);
        // printf("  6: 0b%010b\n", sprite->sprite_collisions[6]);
        // printf("  7: 0b%010b\n", sprite->sprite_collisions[7]);
        // printf("  *: 0b%010b\n", sprite->edge_collisions);
    } else {
        iret = 0x0;
    }
    return result == kGraphicsNotASprite ? kOk : result;
}

/** SPRITE(W, [#]n) */
static MmResult fun_sprite_width(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurface *sprite = NULL;
    MmResult result = parse_sprite_id(argv[2], false, &sprite);
    bool active = (result == kOk) && sprite->type == kGraphicsSprite;
    targ = T_INT;
    iret = active ? (MMINTEGER) sprite->width : -1;
    return result == kGraphicsNotASprite ? kOk : result;
}

/** SPRITE(X, [#]n) */
static MmResult fun_sprite_x(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurface *sprite = NULL;
    MmResult result = parse_sprite_id(argv[2], false, &sprite);
    bool active = (result == kOk) && sprite->type == kGraphicsSprite;
    targ = T_INT;
    iret = active ? (MMINTEGER) sprite->x : 10000;
    return result == kGraphicsNotASprite ? kOk : result;
}

/** SPRITE(Y, [#]n) */
static MmResult fun_sprite_y(int argc, char **argv) {
    if (argc != 3) return kArgumentCount;
    MmSurface *sprite = NULL;
    MmResult result = parse_sprite_id(argv[2], false, &sprite);
    bool active = (result == kOk) && sprite->type == kGraphicsSprite;
    targ = T_INT;
    iret = active ? (MMINTEGER) sprite->y : 10000;
    return result == kGraphicsNotASprite ? kOk : result;
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
