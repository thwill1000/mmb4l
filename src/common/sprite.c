/*-*****************************************************************************

MMBasic for Linux (MMB4L)

sprite.h

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

#include <assert.h>

#include "bitset.h"
#include "sprite.h"

static MmGraphicsColour graphics_sprite_transparent_colour = RGB_BLACK;

static inline MmSurface *sprite_get_first_active() {
    return graphics_surfaces[0].type == kGraphicsSprite
            ? &graphics_surfaces[0]
            : graphics_surfaces[0].next_active_sprite;
}

MmResult sprite_get_num_collisions(MmSurface *sprite, uint32_t *count) {
    *count = 0;
    for (MmSurface *other = sprite_get_first_active(); other; other = other->next_active_sprite) {
        *count += bitset_get(sprite->sprite_collisions, other->id) ? 1 : 0;
    }
    if (sprite->edge_collisions) (*count)++;
    return kOk;
}

MmResult sprite_get_collision(MmSurface *sprite, uint32_t n, MmSurfaceId *id) {
    assert(n != 0);
    *id = -1;
    uint32_t count = 0;
    for (MmSurface *other = sprite_get_first_active(); other; other = other->next_active_sprite) {
        if (bitset_get(sprite->sprite_collisions, other->id)) {
            count++;
            if (count == n) {
                *id = other->id;
                break;
            }
        }
    }
    if (*id == -1 && sprite->edge_collisions) {
        count++;
        if (count == n) {
            *id = 0xFFF0 | sprite->edge_collisions;
        }
    }
    return kOk;
}

static inline bool sprite_has_collided(MmSurface *sprite) {
    if (sprite->edge_collisions) return true;
    const size_t limit = 32 / sizeof(int);
    for (size_t i = 0; i < limit; ++i) {
        if (sprite->sprite_collisions[i]) return true;
    }
    return false;
}

MmResult sprite_get_num_collided_sprites(uint32_t *count) {
    *count = 0;
    for (MmSurface *sprite = sprite_get_first_active(); sprite; sprite = sprite->next_active_sprite) {
        *count += sprite_has_collided(sprite) ? 1 : 0;
    }
    return kOk;
}

MmResult sprite_get_collided_sprite(uint32_t n, MmSurfaceId *id) {
    assert(n != 0);
    uint32_t count = 0;
    for (MmSurface *sprite = sprite_get_first_active(); sprite; sprite = sprite->next_active_sprite) {
        if (sprite_has_collided(sprite)) {
            count++;
            if (count == n) {
                *id = sprite->id;
                return kOk;
            }
        }
    }
    *id = -1;
    return kOk;
}

static inline void sprite_clear_data(MmSurface *sprite) {
    bitset_reset(sprite->sprite_collisions, 256);
    sprite->edge_collisions = 0x0;
}

static inline bool sprite_check_for_edge_collision(MmSurface *sprite, MmSurface *surface) {
    assert(sprite);
    assert(surface);

    sprite->edge_collisions = 0x0;
    if (sprite->type != kGraphicsSprite) return false;
    if (sprite->x < 0) sprite->edge_collisions |= kSpriteEdgeLeft;
    if (sprite->y < 0) sprite->edge_collisions |= kSpriteEdgeTop;
    if (sprite->x + sprite->width > surface->width) sprite->edge_collisions |= kSpriteEdgeRight;
    if (sprite->y + sprite->height > surface->height) sprite->edge_collisions |= kSpriteEdgeBottom;
    return sprite->edge_collisions > 0;
}

static inline bool sprite_check_for_sprite_collision(MmSurface *sprite, MmSurface *other) {
    assert(sprite->type == kGraphicsSprite);
    assert(other->type == kGraphicsSprite);

//    printf("Check for collision %d with %d\n", sprite->id, other->id);

    // 1. A sprite cannot collide with itself.
    // 2. Only active sprites can collide.
    // 3. Collisions can only occur between sprites on the same layer, or on layer zero.
    if ((sprite == other) ||
        (sprite->layer != other->layer && sprite->layer != 0 && other->layer != 0)) {
        bitset_clear(sprite->sprite_collisions, other->id);
        bitset_clear(other->sprite_collisions, sprite->id);
        return false;
    }

    // It is sufficient for the sprites (bounding boxes) to touch
    // for a collision to be registered, they do not have to overlap.
    bool collision = !(other->x + (int32_t) other->width < sprite->x ||
        other->x > sprite->x + (int32_t) sprite->width ||
        other->y + (int32_t) other->height < sprite->y ||
        other->y > sprite->y + (int32_t) sprite->height);

    if (collision) {
        bitset_set(sprite->sprite_collisions, other->id);
        bitset_set(other->sprite_collisions, sprite->id);
    } else {
        bitset_clear(sprite->sprite_collisions, other->id);
        bitset_clear(other->sprite_collisions, sprite->id);
    }

    return collision;
}

MmResult sprite_update_collisions(MmSurface *sprite) {
    assert(sprite);

    //printf("Update collisions for %d\n", sprite->id - 128);

    // Clear 'global' sprite state.
    //sprite_clear_state();

    // Clear collision data for this sprite.
    sprite_clear_data(sprite);

    bool has_collision = false;

    // Check for collisions with other sprites.   
    for (MmSurface *other = sprite_get_first_active(); other; other = other->next_active_sprite) {
        //printf("  %d\n", other->id - 128);
        has_collision |= sprite_check_for_sprite_collision(sprite, other);
    }

    //if (has_collision) printf("Sprite %d has collision\n", sprite->id - 128);

    // Check for collisions with surface edge.
    has_collision |= sprite_check_for_edge_collision(sprite, graphics_current);

    // if (has_collision) {
    //     printf("Sprite %d has collision\n", sprite->id);
    //     printf("  0: 0b%010b\n", sprite->sprite_collisions[0]);
    //     printf("  1: 0b%010b\n", sprite->sprite_collisions[1]);
    //     printf("  2: 0b%010b\n", sprite->sprite_collisions[2]);
    //     printf("  3: 0b%010b\n", sprite->sprite_collisions[3]);
    //     printf("  4: 0b%010b\n", sprite->sprite_collisions[4]);
    //     printf("  5: 0b%010b\n", sprite->sprite_collisions[5]);
    //     printf("  6: 0b%010b\n", sprite->sprite_collisions[6]);
    //     printf("  7: 0b%010b\n", sprite->sprite_collisions[7]);
    //     printf("  *: 0b%010b\n", sprite->edge_collisions);
    // }

    return kOk;
}

MmResult sprite_update_all_collisions() {
    // Clear 'global' sprite state.
    //sprite_clear_state();

    // Clear collision data for all active sprites.
    for (MmSurface *sprite = sprite_get_first_active(); sprite; sprite = sprite->next_active_sprite) {
        sprite_clear_data(sprite);
    }

    for (MmSurface *sprite = sprite_get_first_active(); sprite; sprite = sprite->next_active_sprite) {
        // Check for collisions with other sprites.
        for (MmSurface *other = sprite->next_active_sprite; other; other = other->next_active_sprite) {
            (void) sprite_check_for_sprite_collision(sprite, other);
        }

        // Check for collisions with surface edge.
        (void) sprite_check_for_edge_collision(sprite, graphics_current);

        // Record collision in global state,
        // if (graphics_sprite_state.num_collisions < GRAPHICS_MAX_COLLISIONS &&
        //     (sprite->num_collisions > 0 || sprite->edge_collisions != 0x0)) {
        //     graphics_sprite_state.collisions[graphics_sprite_state.num_collisions++] = id;
        // }
    }

    // TODO: Setup data for sprite interrupt.
    // graphics_sprite_state.collision_found = graphics_sprite_state.num_collisions > 0;

    return kOk;
}

MmResult sprite_activate(MmSurface *sprite, bool activate) {
    switch (sprite->type) {
        case kGraphicsSprite:
            if (!activate) { // Deactivating an active sprite.
                sprite->type = kGraphicsInactiveSprite;
                for (MmSurfaceId id = sprite->id - 1; id >= 0; --id) {
                    if (graphics_surfaces[id].next_active_sprite == sprite) {
                        graphics_surfaces[id].next_active_sprite = sprite->next_active_sprite;
                    } else {
                        break;
                    }
                }
                // TODO: Clear collisions ?
            }
            return kOk;
        case kGraphicsInactiveSprite:
            if (activate) { // Activating an inactive sprite.
                sprite->type = kGraphicsSprite;
                for (MmSurfaceId id = sprite->id - 1; id >= 0; --id) {
                    if (graphics_surfaces[id].next_active_sprite == sprite->next_active_sprite) {
                        graphics_surfaces[id].next_active_sprite = sprite;
                    } else {
                        break;
                    }
                }
                // TODO: Recalculate collisions ?
            }
            return kOk;
        default:
            return kGraphicsNotASprite;
    }
}

MmResult sprite_get_collision_bitset(MmSurface *sprite, uint8_t start, uint64_t *bitset) {
    if (sprite->type != kGraphicsSprite && sprite->type != kGraphicsInactiveSprite)
        return kGraphicsNotASprite;
    if (start % 64 != 0) return kInternalFault;
    *bitset = ((uint64_t *) sprite->sprite_collisions)[start / 64];
    return kOk;
}

MmResult sprite_scroll(int32_t x, int32_t y, MmGraphicsColour colour) {
    if (x == 0 && y == 0) return kOk;
#if 0
    m = ((maxW * (y > 0 ? y : -y)+1) >>1);
    n = ((maxH * (x > 0 ? x : -x)+1) >>1);
    if (n > m)m = n;
    if (blank == -2)current = (char *)GetMemory(m);
    for (i = LIFOpointer - 1; i >= 0; i--) blithide(LIFO[i], 0);

    for (i = zeroLIFOpointer - 1; i >= 0; i--) {
        int xs = spritebuff[zeroLIFO[i]].x + (spritebuff[zeroLIFO[i]].w >> 1);
        int ys = spritebuff[zeroLIFO[i]].y + (spritebuff[zeroLIFO[i]].h >> 1);
        blithide(zeroLIFO[i], 0);
        xs += x;
        if (xs >= maxW)xs -= maxW;
        if (xs < 0)xs += maxW;
        spritebuff[zeroLIFO[i]].x = xs - (spritebuff[zeroLIFO[i]].w >> 1);
        ys -= y;
        if (ys >= maxH)ys -= maxH;
        if (ys < 0)ys += maxH;
        spritebuff[zeroLIFO[i]].y = ys - (spritebuff[zeroLIFO[i]].h >> 1);
    }

    if (x > 0) {
        if (blank == -2)ReadBufferFast(maxW - x, 0, maxW - 1, maxH - 1, (unsigned char *)current);
        ScrollBufferH(x);
        if (blank == -2)DrawBufferFast(0, 0, x - 1, maxH - 1, -1, (unsigned char *)current);
        else if (blank != -1)DrawRectangle(0, 0, x - 1, maxH - 1, blank);
    }
    else if (x < 0) {
        x = -x;
        if (blank == -2)ReadBufferFast(0, 0, x - 1, maxH - 1, (unsigned char *)current);
        ScrollBufferH(-x);
        if (blank == -2)DrawBufferFast(maxW - x, 0, maxW - 1, maxH - 1, -1, (unsigned char *)current);
        else if (blank != -1)DrawRectangle(maxW - x, 0, maxW - 1, maxH - 1, blank);
    }

    if (y > 0) {
        if (blank == -2)ReadBufferFast(0, 0, maxW - 1, y - 1, (unsigned char *)current);
        ScrollBufferV(y, 0);
        if (blank == -2)DrawBufferFast(0, maxH - y, maxW - 1, maxH - 1, -1, (unsigned char *)current);
        else if (blank != -1)DrawRectangle(0, maxH - y, maxW - 1, maxH - 1, blank);
    }
    else if (y < 0) {
        y = -y;
        if (blank == -2)ReadBufferFast(0, maxH - y, maxW - 1, maxH - 1, (unsigned char *)current);
        ScrollBufferV(-y, 0);
        if (blank == -2)DrawBufferFast(0, 0, maxW - 1, y - 1, -1, (unsigned char *)current);
        else if (blank != -1)DrawRectangle(0, 0, maxW - 1, y - 1, blank);
    }

    for (i = 0; i < zeroLIFOpointer; i++) {
        BlitShowBuffer(zeroLIFO[i], spritebuff[zeroLIFO[i]].x, spritebuff[zeroLIFO[i]].y, 0);
    }

    for (i = 0; i < LIFOpointer; i++) {
        if (spritebuff[LIFO[i]].next_x != 10000) {
            spritebuff[LIFO[i]].x = spritebuff[LIFO[i]].next_x;
            spritebuff[LIFO[i]].next_x = 10000;
        }
        if (spritebuff[LIFO[i]].next_y != 10000) {
            spritebuff[LIFO[i]].y = spritebuff[LIFO[i]].next_y;
            spritebuff[LIFO[i]].next_y = 10000;
        }

        BlitShowBuffer(LIFO[i], spritebuff[LIFO[i]].x, spritebuff[LIFO[i]].y, 0);
    }

    ProcessCollisions(0);
    if (current)FreeMemory((unsigned char *)current);
#endif
    return kOk;
}

// mode 0x1 - draw the sprite.
MmResult sprite_show(MmSurface *sprite, MmSurface *write_surface, int32_t x, int32_t y,
                     uint8_t mode) {
    //printf("mode = %d\n", mode);
    if (sprite->type != kGraphicsSprite && sprite->type != kGraphicsInactiveSprite) return kGraphicsNotASprite;
    if (write_surface->type == kGraphicsNone) return kGraphicsSurfaceNotFound;

    MmResult result = kOk;
    MmSurface restore_surface =
        { .width = sprite->width, .height = sprite->height, .pixels = sprite->restore };

    if (mode == 0x1) {
        // printf("Doing shit\n");
        if (sprite->type == kGraphicsSprite) {
            // printf("Active\n");
            // Restore previous background.
            result = graphics_blit(0, 0, sprite->x, sprite->y, sprite->width, sprite->height, &restore_surface,
                                   write_surface, 0x0, -1);
            if (FAILED(result)) return result;
        }

        // Save the current background.
        result = graphics_blit(x, y, 0, 0, sprite->width, sprite->height, write_surface,
                               &restore_surface, 0x0, -1);
        if (FAILED(result)) return result;
        sprite->x = x;
        sprite->y = y;
    }

    result = graphics_blit(0, 0, x, y, sprite->width, sprite->height, sprite,
                           write_surface, 0x4, graphics_sprite_transparent_colour);
    if (FAILED(result)) return result;

    if (mode == 0x1) (void) sprite_activate(sprite, true);

    return kOk;
}

MmResult sprite_set_transparent_colour(MmGraphicsColour colour) {
    if (colour < 0) return kInternalFault;
    graphics_sprite_transparent_colour = colour;
    return kOk;
}
