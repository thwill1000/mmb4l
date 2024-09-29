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
#include "interrupt.h"
#include "sprite.h"
#include "stack.h"

MmSurfaceId sprite_last_collision = SPRITE_NO_COLLISION;
MmGraphicsColour sprite_transparent_colour = RGB_BLACK;

static bool sprite_initialised = false;
static bool sprite_all_hidden = false;
static Stack sprite_stack[2];

MmResult sprite_init() {
    if (sprite_initialised) return kOk;
    MmResult result = kOk;
    for (size_t ii = 0; SUCCEEDED(result) && ii < 2; ++ii) {
        result = stack_init(&sprite_stack[ii], MmSurfaceId, GRAPHICS_MAX_SURFACES, NULL);
    }
    sprite_all_hidden = false;
    sprite_initialised = true;
    sprite_last_collision = SPRITE_NO_COLLISION;
    return kOk;
}

MmResult sprite_term() {
    if (!sprite_initialised) return kOk;
    for (size_t ii = 0; ii < 2; ++ii) {
        (void) stack_term(&sprite_stack[ii]);
    }
    sprite_initialised = false;
    return kOk;
}

static inline MmSurface *sprite_first_active() {
    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        if (graphics_surfaces[id].type == kGraphicsSprite) return &graphics_surfaces[id];
    }
    return NULL;
}

static inline MmSurface *sprite_next_active(MmSurface *sprite) {
    for (MmSurfaceId id = sprite->id + 1; id <= GRAPHICS_MAX_ID; ++id) {
        if (graphics_surfaces[id].type == kGraphicsSprite) return &graphics_surfaces[id];
    }
    return NULL;
}

MmResult sprite_count(size_t *total) {
    *total = 0;
    MmResult result = kOk;
    for (size_t i = 0; SUCCEEDED(result) && i < 2; ++i) {
        *total += stack_size(&sprite_stack[i]);
    }
    return result;
}

MmResult sprite_count_on_layer(unsigned layer, size_t *total) {
    MmResult result = kOk;
    if (layer == 0) {
        *total = stack_size(&sprite_stack[0]);
    } else {
        *total = 0;
        const MmSurfaceId *base = (MmSurfaceId *) sprite_stack[1].storage;
        const MmSurfaceId *top = (MmSurfaceId *) sprite_stack[1].top;
        for (const MmSurfaceId *pid = top - 1; SUCCEEDED(result) && pid >= base; --pid) {
            MmSurface *s = &graphics_surfaces[*pid];
            if (s->type == kGraphicsSprite && s->layer == layer) (*total)++;
        }
    }
    return result;
}

MmResult sprite_destroy(MmSurface *sprite) {
    MmResult result = sprite_hide(sprite);
    if (result == kOk || result == kSpriteInactive) result = graphics_surface_destroy(sprite);
    return result;
}

MmResult sprite_destroy_all() {
    MmResult result = kOk;
    for (MmSurfaceId id = 0; SUCCEEDED(result) && id <= GRAPHICS_MAX_ID; ++id) {
        MmSurface *sprite = &graphics_surfaces[id];
        switch (sprite->type) {
            case kGraphicsSprite:
            case kGraphicsInactiveSprite:
                result = sprite_destroy(sprite);
                break;
            default:
                // Do nothing.
                break;
        }
    }
    return result;
}

/**
 * Renders a sprite.
 *
 * @param  sprite  The sprite to render.
 * @param  dst     The surface to render the sprite to.
 */
static inline MmResult sprite_render(MmSurface *sprite, MmSurface *dst) {
    assert(sprite);
    assert(sprite->type == kGraphicsSprite || sprite->type == kGraphicsInactiveSprite);
    assert(dst);
    assert(dst->type != kGraphicsNone);
    // TODO: sprite should remember show flags!
    return graphics_blit(0, 0, sprite->x, sprite->y, sprite->width, sprite->height, sprite, dst,
                         0x04, sprite_transparent_colour);
}

/**
 * Renders a sprite background.
 *
 * @param  sprite  The sprite whose background to render.
 * @param  dst     The surface to render the sprite to.
 */
static inline MmResult sprite_render_background(MmSurface *sprite, MmSurface *dst) {
    assert(sprite);
    assert(sprite->type == kGraphicsSprite || sprite->type == kGraphicsInactiveSprite);
    assert(dst);
    assert(dst->type != kGraphicsNone);
    MmSurface background =
        { .type = kGraphicsBuffer, .width = sprite->width, .height = sprite->height,
          .pixels = sprite->background };
    return graphics_blit(0, 0, sprite->x, sprite->y, sprite->width, sprite->height, &background,
                         dst, 0x0, -1);
}

/**
 * Updates a sprites 'background' property.
 *
 * @param  sprite  The sprite to update.
 * @param  src     The surface to copy the background from.
 */
static inline MmResult sprite_update_background(MmSurface *sprite, MmSurface *src) {
    assert(sprite);
    assert(sprite->type == kGraphicsSprite || sprite->type == kGraphicsInactiveSprite);
    assert(src);
    assert(src->type != kGraphicsNone);
    MmSurface background =
        { .type = kGraphicsBuffer, .width = sprite->width, .height = sprite->height,
          .pixels = sprite->background };
    return graphics_blit(sprite->x, sprite->y, 0, 0, sprite->width, sprite->height, src,
                         &background, 0x0, -1);
}

MmResult sprite_get_num_collisions(MmSurface *sprite, uint32_t *count) {
    *count = 0;
    for (MmSurface *other = sprite_first_active(); other; other = sprite_next_active(other)) {
        *count += bitset_get(sprite->sprite_collisions, other->id) ? 1 : 0;
    }
    if (sprite->edge_collisions) (*count)++;
    return kOk;
}

MmResult sprite_get_collision(MmSurface *sprite, uint32_t n, MmSurfaceId *id) {
    assert(n != 0);
    *id = -1;
    uint32_t count = 0;
    for (MmSurface *other = sprite_first_active(); other; other = sprite_next_active(other)) {
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
    for (MmSurface *sprite = sprite_first_active(); sprite; sprite = sprite_next_active(sprite)) {
        *count += sprite_has_collided(sprite) ? 1 : 0;
    }
    return kOk;
}

MmResult sprite_get_collided_sprite(uint32_t n, MmSurfaceId *id) {
    assert(n != 0);
    uint32_t count = 0;
    for (MmSurface *sprite = sprite_first_active(); sprite; sprite = sprite_next_active(sprite)) {
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

static inline void sprite_clear_collision_data(MmSurface *sprite) {
    bitset_reset(sprite->sprite_collisions, 256);
    sprite->edge_collisions = 0x0;
}

MmResult sprite_hide(MmSurface *sprite) {
    if (sprite_all_hidden) return kSpritesAreHidden;
    switch (sprite->type) {
        case kGraphicsSprite: break;
        case kGraphicsInactiveSprite: return kSpriteInactive;
        default: return kGraphicsInvalidSprite;
    }

    MmResult result = sprite_render_background(sprite, graphics_current);
    if (FAILED(result)) return result;
    // layer_in_use[spritebuff[bnbr].layer]--;
    sprite->x = GRAPHICS_OFF_SCREEN;
    sprite->y = GRAPHICS_OFF_SCREEN;
    result = stack_remove(&sprite_stack[sprite->layer == 0 ? 0 : 1], sprite->id);
    if (FAILED(result)) return result;
    sprite->layer = 0xFF;
    sprite->next_x = GRAPHICS_OFF_SCREEN;
    sprite->next_y = GRAPHICS_OFF_SCREEN;
    sprite_clear_collision_data(sprite);
    sprite->type = kGraphicsInactiveSprite;
    return result;
}

/**
 * Temporarily hides sprites in a Stack.
 *
 * Specifically restores their background and changes them to type == kGraphicsInactiveSprite.
 *
 * @param  stack   The Stack of MmSurfaceIds to process.
 * @param  sprite  If not NULL then stop hiding sprites when we reach this sprite.
 *                 WITHOUT hiding that sprite.
 */
static MmResult sprite_tmp_hide(Stack *stack, MmSurface *sprite) {
    MmResult result = kOk;
    const MmSurfaceId *base = (MmSurfaceId *) stack->storage;
    const MmSurfaceId *top = (MmSurfaceId *) stack->top;
    for (const MmSurfaceId *pid = top - 1; SUCCEEDED(result) && pid >= base; --pid) {
        MmSurface *s = &graphics_surfaces[*pid];
        if (s == sprite) break;
        result = sprite_render_background(s, graphics_current);
        s->type = kGraphicsInactiveSprite;
    }
    return result;
}

/**
 * Restores sprites temporarily hidden by calling stack_tmp_hide().
 *
 * Specifically re-blits them and changes them to type == kGraphicsSprite.
 */
static MmResult sprite_tmp_restore(Stack *stack) {
    MmResult result = kOk;
    const MmSurfaceId *base = (MmSurfaceId *) stack->storage;
    const MmSurfaceId *top = (MmSurfaceId *) stack->top;
    for (const MmSurfaceId *pid = base; SUCCEEDED(result) && pid < top; ++pid) {
        MmSurface *s = &graphics_surfaces[*pid];
        if (s->type == kGraphicsInactiveSprite) {
            s->type = kGraphicsSprite;
            result = sprite_update_background(s, graphics_current);
            if (SUCCEEDED(result)) result = sprite_render(s, graphics_current);
        }
    }
    return result;
}

MmResult sprite_hide_safe(MmSurface *sprite) {
    if (sprite_all_hidden) return kSpritesAreHidden;
    switch (sprite->type) {
        case kGraphicsSprite: break;
        case kGraphicsInactiveSprite: return kSpriteInactive;
        default: return kGraphicsInvalidSprite;
    }

    unsigned layer = sprite->layer;

    // 1) Temporarily hide all sprites shown after this sprite.
    MmResult result = sprite_tmp_hide(&sprite_stack[1], sprite);
    if (SUCCEEDED(result) && layer == 0) result = sprite_tmp_hide(&sprite_stack[0], sprite);

    // 2) Properly hide this sprite.
    if (SUCCEEDED(result)) result = sprite_hide(sprite);

    // 3) Restore sprites from step (1).
    if (SUCCEEDED(result) && layer == 0) result = sprite_tmp_restore(&sprite_stack[0]);
    if (SUCCEEDED(result)) result = sprite_tmp_restore(&sprite_stack[1]);

    return result;
}

MmResult sprite_hide_all() {
    if (sprite_all_hidden) return kSpritesAreHidden;
    MmResult result = sprite_tmp_hide(&sprite_stack[1], NULL);
    if (SUCCEEDED(result)) result = sprite_tmp_hide(&sprite_stack[0], NULL);
    if (SUCCEEDED(result)) sprite_all_hidden = true;
    return result;
}

static inline MmResult sprite_update_all_positions(Stack *stack) {
    const MmSurfaceId *base = (MmSurfaceId *) stack->storage;
    const MmSurfaceId *top = (MmSurfaceId *) stack->top;
    for (const MmSurfaceId *pid = base; pid < top; ++pid) {
        MmSurface *sprite = &graphics_surfaces[*pid];
        if (sprite->next_x != GRAPHICS_OFF_SCREEN) {
            sprite->x = sprite->next_x;
            sprite->next_x = GRAPHICS_OFF_SCREEN;
        }
        if (sprite->next_y != GRAPHICS_OFF_SCREEN) {
            sprite->y = sprite->next_y;
            sprite->next_y = GRAPHICS_OFF_SCREEN;
        }
    }
    return kOk;
}

MmResult sprite_move() {
    MmResult result = sprite_hide_all();
    if (SUCCEEDED(result)) {
        for (int i = 0; SUCCEEDED(result) && i < 2; ++i) {
            result = sprite_update_all_positions(&sprite_stack[i]);
        }
    }
    if (SUCCEEDED(result)) result = sprite_restore_all();
    return result;
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

    // if (sprite->edge_collisions) {
    //     printf("Sprite %d has edge collision\n", sprite->id);
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
    bool collision = !(other->x + other->width < sprite->x ||
        other->x > sprite->x + sprite->width ||
        other->y + other->height < sprite->y ||
        other->y > sprite->y + sprite->height);

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
    assert(sprite->type == kGraphicsSprite);

    //printf("Update collisions for %d\n", sprite->id - 128);

    // Clear 'global' sprite state.
    //sprite_clear_state();

    // Clear collision data for this sprite.
    sprite_clear_collision_data(sprite);

    bool has_collision = false;

    // Check for collisions with other sprites.
    for (MmSurface *other = sprite_first_active(); other; other = sprite_next_active(other)) {
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

    if (has_collision) {
        sprite_last_collision = sprite->id;
        interrupt_fire(kInterruptSpriteCollision);
    } else {
        sprite_last_collision = SPRITE_NO_COLLISION;
    }

    return kOk;
}

MmResult sprite_update_all_collisions() {
    // Clear collision data for all active sprites.
    for (MmSurface *sprite = sprite_first_active(); sprite; sprite = sprite_next_active(sprite)) {
        sprite_clear_collision_data(sprite);
    }

    bool has_collision = false;

    for (MmSurface *sprite = sprite_first_active(); sprite; sprite = sprite_next_active(sprite)) {
        // Check for collisions with other sprites.
        for (MmSurface *other = sprite_next_active(sprite); other; other = sprite_next_active(other)) {
            has_collision |= sprite_check_for_sprite_collision(sprite, other);
        }

        // Check for collisions with surface edge.
        has_collision |= sprite_check_for_edge_collision(sprite, graphics_current);
    }

    if (has_collision) {
        sprite_last_collision = SPRITE_SCROLL_COLLISION;
        interrupt_fire(kInterruptSpriteCollision);
    } else {
        sprite_last_collision = SPRITE_NO_COLLISION;
    }

    return kOk;
}

MmResult sprite_get_collision_bitset(MmSurface *sprite, uint8_t start, uint64_t *bitset) {
    if (sprite->type != kGraphicsSprite && sprite->type != kGraphicsInactiveSprite)
        return kGraphicsNotASprite;
    if (start % 64 != 0) return kInternalFault;
    *bitset = ((uint64_t *) sprite->sprite_collisions)[start / 64];
    return kOk;
}

MmResult sprite_restore_all() {
    if (!sprite_all_hidden) return kSpritesNotHidden;
    MmResult result = sprite_tmp_restore(&sprite_stack[0]);
    if (SUCCEEDED(result)) result = sprite_tmp_restore(&sprite_stack[1]);
    if (SUCCEEDED(result)) sprite_all_hidden = false;
    if (SUCCEEDED(result)) result = sprite_update_all_collisions();
    return result;
}

MmResult sprite_scroll(int x, int y, MmGraphicsColour colour) {
    if (x == 0 && y == 0) return kOk;

    // Temporarily hide all sprites.
    MmResult result = sprite_tmp_hide(&sprite_stack[1], NULL);
    if (SUCCEEDED(result)) result = sprite_tmp_hide(&sprite_stack[0], NULL);

    // Translate all layer 0 sprites.
    {
        const MmSurfaceId *base = (MmSurfaceId *) sprite_stack[0].storage;
        const MmSurfaceId *top = (MmSurfaceId *) sprite_stack[0].top;
        for (const MmSurfaceId *pid = top - 1; pid >= base; --pid) {
            MmSurface *s = &graphics_surfaces[*pid];

            int xs = s->x + (s->width >> 1); // Add half the width.
            xs += x;
            if (xs >= graphics_current->width) {
                xs -= graphics_current->width;
            } else if (xs < 0) {
                xs += graphics_current->width;
            }
            s->x = xs - (s->width >> 1);

            int ys = s->y + (s->height >> 1); // Add half the height.
            ys += y;
            if (ys >= graphics_current->height) {
                ys -= graphics_current->height;
            } else if (ys < 0) {
                ys += graphics_current->height;
            }
            s->y = ys - (s->height >> 1);
        }
    }

    // Scroll the background.
    if (SUCCEEDED(result)) {
        if (colour < -2) colour = RGB_WHITE;
        result = graphics_scroll(graphics_current, x, y, colour);
    }

    // Apply any pending updates to positions of sprites not on layer 0.
    if (SUCCEEDED(result)) result = sprite_update_all_positions(&sprite_stack[1]);

    // Restore temporarily hidden sprites.
    if (SUCCEEDED(result)) result = sprite_tmp_restore(&sprite_stack[0]);
    if (SUCCEEDED(result)) result = sprite_tmp_restore(&sprite_stack[1]);

    // Update collision state.
    if (SUCCEEDED(result)) result = sprite_update_all_collisions();

    return result;
}

MmResult sprite_show(MmSurface *sprite, MmSurface *dst_surface, int x, int y,
                     unsigned layer, unsigned flags) {
    if (sprite_all_hidden) return kSpritesAreHidden;
    if (sprite->type != kGraphicsSprite
            && sprite->type != kGraphicsInactiveSprite) return kGraphicsNotASprite;
    if (dst_surface->type == kGraphicsNone) return kGraphicsInvalidWriteSurface;

    MmResult result = kOk;

    // If the sprite is already visible then ...
    if (sprite->type == kGraphicsSprite) {
        // ... redraw the background where it was.
        result = sprite_render_background(sprite, dst_surface);

        // ... and remove it from the appropriate old layer stack.
        if (SUCCEEDED(result)) {
            result = stack_remove(&sprite_stack[sprite->layer == 0 ? 0 : 1], sprite->id);
        }
    }

    sprite->x = x;
    sprite->y = y;

    // Save the background at the destination.
    if (SUCCEEDED(result)) result = sprite_update_background(sprite, dst_surface);

    // Add the sprite to the appropriate new layer stack.
    if (SUCCEEDED(result)) {
        sprite->layer = layer;
        result = stack_push(&sprite_stack[sprite->layer == 0 ? 0 : 1], sprite->id);
    }

    // Display the sprite.
    // Note inversion of the 3rd bit of 'flags'.
    if (SUCCEEDED(result)) {
        result = graphics_blit(0, 0, sprite->x, sprite->y, sprite->width, sprite->height, sprite,
                               dst_surface, flags ^ 0x04, sprite_transparent_colour);
    }

    // Flag the sprite as active / visible.
    if (SUCCEEDED(result)) sprite->type = kGraphicsSprite;

    return result;
}

MmResult sprite_show_safe(MmSurface *sprite, MmSurface *dst_surface, int x, int y,
                          unsigned layer, unsigned flags, bool ontop) {
    if (sprite->type == kGraphicsInactiveSprite) {
        return sprite_show(sprite, dst_surface, x, y, layer, flags);
    }

    if (sprite_all_hidden) return kSpritesAreHidden;
    if (sprite->type != kGraphicsSprite) return kGraphicsNotASprite;
    if (dst_surface->type == kGraphicsNone) return kGraphicsInvalidWriteSurface;

    // 1) Temporarily hide all sprites shown after this sprite.
    MmResult result = sprite_tmp_hide(&sprite_stack[1], sprite);
    if (SUCCEEDED(result) && layer == 0) result = sprite_tmp_hide(&sprite_stack[0], sprite);

    // 2) If ontop != true then properly show this sprite.
    if (SUCCEEDED(result) && !ontop) result = sprite_show(sprite, dst_surface, x, y, layer, flags);

    // 3) Restore sprites from step (1).
    if (SUCCEEDED(result) && layer == 0) result = sprite_tmp_restore(&sprite_stack[0]);
    if (SUCCEEDED(result)) result = sprite_tmp_restore(&sprite_stack[1]);

    // 4) If ontop == true then properly show this sprite.
    if (SUCCEEDED(result) && ontop) result = sprite_show(sprite, dst_surface, x, y, layer, flags);

    return result;
}

MmResult sprite_set_transparent_colour(MmGraphicsColour colour) {
    if (colour < 0) return kInternalFault;
    sprite_transparent_colour = colour;
    return kOk;
}
