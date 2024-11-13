/*-*****************************************************************************

MMBasic for Linux (MMB4L)

sprite.c

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
#include "error.h"
#include "interrupt.h"
#include "sprite.h"
#include "utility.h"

MmSurfaceId sprite_last_collision = SPRITE_NO_COLLISION;
MmGraphicsColour sprite_transparent_colour = RGB_BLACK;
Stack sprite_z_stack;

static bool sprite_initialised = false;
static bool sprite_all_hidden = false;

MmResult sprite_init() {
    if (sprite_initialised) return kOk;
    ON_FAILURE_RETURN(stack_init(&sprite_z_stack, MmSurfaceId, GRAPHICS_MAX_SURFACES, NULL);)
    sprite_all_hidden = false;
    sprite_initialised = true;
    sprite_last_collision = SPRITE_NO_COLLISION;
    return kOk;
}

MmResult sprite_term() {
    if (!sprite_initialised) return kOk;
    ON_FAILURE_RETURN(stack_term(&sprite_z_stack));
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
    *total = stack_size(&sprite_z_stack);
    return kOk;
}

MmResult sprite_count_on_layer(unsigned layer, size_t *total) {
    *total = 0;
    const MmSurfaceId *base = (MmSurfaceId *) sprite_z_stack.storage;
    const MmSurfaceId *top = (MmSurfaceId *) sprite_z_stack.top;
    for (const MmSurfaceId *pid = top - 1; pid >= base; --pid) {
        MmSurface *s = &graphics_surfaces[*pid];
        if (s->type == kGraphicsSprite && s->layer == layer) (*total)++;
    }
    return kOk;
}

MmResult sprite_destroy(MmSurface *sprite) {
    return graphics_surface_destroy(sprite);
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
    return graphics_blit(0, 0, sprite->x, sprite->y, sprite->width, sprite->height, sprite, dst,
                         sprite->blit_flags, sprite_transparent_colour);
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

static inline MmResult sprite_hide_internal(MmSurface *sprite, MmSurface *dst_surface) {
    assert(sprite->type == kGraphicsSprite);

    ON_FAILURE_RETURN(sprite_render_background(sprite, dst_surface));
    ON_FAILURE_RETURN(stack_remove(&sprite_z_stack, sprite->id));
    sprite->x = GRAPHICS_OFF_SCREEN;
    sprite->y = GRAPHICS_OFF_SCREEN;
    sprite->layer = 0xFF;
    sprite->next_x = GRAPHICS_OFF_SCREEN;
    sprite->next_y = GRAPHICS_OFF_SCREEN;
    sprite->type = kGraphicsInactiveSprite;

    return kOk;
}

MmResult sprite_hide(MmSurface *sprite) {
    if (sprite_all_hidden) return kSpritesAreHidden;
    switch (sprite->type) {
        case kGraphicsSprite: break;
        case kGraphicsInactiveSprite: return kSpriteInactive;
        default: return mmresult_ex(kGraphicsInvalidSprite, "Invalid sprite: %d", sprite->id);
    }
    ON_FAILURE_RETURN(sprite_hide_internal(sprite, graphics_current));
    return sprite_update_collisions(sprite);
}

/**
 * Temporarily hides sprites.
 *
 * Specifically restores their background and changes them to type == kGraphicsInactiveSprite.
 *
 * @param  sprite  If not NULL then stop hiding sprites when we reach this sprite.
 *                 WITHOUT hiding this sprite.
 */
static MmResult sprite_tmp_hide(MmSurface *sprite) {
    const Stack *stack = &sprite_z_stack;
    const MmSurfaceId *base = (MmSurfaceId *) stack->storage;
    const MmSurfaceId *top = (MmSurfaceId *) stack->top;
    MmResult result = kOk;
    for (const MmSurfaceId *pid = top - 1; SUCCEEDED(result) && pid >= base; --pid) {
        MmSurface *s = &graphics_surfaces[*pid];
        if (s == sprite) break;
        result = sprite_render_background(s, graphics_current);
        s->type = kGraphicsInactiveSprite;
    }
    return result;
}

/**
 * Restores sprites temporarily hidden by calling sprite_tmp_hide().
 *
 * Specifically re-blits them and changes them to type == kGraphicsSprite.
 */
static MmResult sprite_tmp_restore() {
    const Stack *stack = &sprite_z_stack;
    const MmSurfaceId *base = (MmSurfaceId *) stack->storage;
    const MmSurfaceId *top = (MmSurfaceId *) stack->top;
    MmResult result = kOk;
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
        default: return mmresult_ex(kGraphicsInvalidSprite, "Invalid sprite: %d", sprite->id);
    }

    // 1) Temporarily hide all sprites shown after this sprite.
    ON_FAILURE_RETURN(sprite_tmp_hide(sprite));

    // 2) Properly hide this sprite.
    ON_FAILURE_RETURN(sprite_hide(sprite));

    // 3) Restore sprites from step (1).
    ON_FAILURE_RETURN(sprite_tmp_restore());

    // 4) Update collisions for this sprite.
    return sprite_update_collisions(sprite);
}

MmResult sprite_hide_all() {
    if (sprite_all_hidden) return kSpritesAreHidden;
    ON_FAILURE_RETURN(sprite_tmp_hide(NULL));
    sprite_all_hidden = true;
    return kOk;
}

static inline void sprite_update_position(MmSurface *sprite) {
    if (sprite->next_x != GRAPHICS_OFF_SCREEN) {
        sprite->x = sprite->next_x;
        sprite->next_x = GRAPHICS_OFF_SCREEN;
    }
    if (sprite->next_y != GRAPHICS_OFF_SCREEN) {
        sprite->y = sprite->next_y;
        sprite->next_y = GRAPHICS_OFF_SCREEN;
    }
}

static inline MmResult sprite_update_all_positions() {
    const Stack *stack = &sprite_z_stack;
    const MmSurfaceId *base = (MmSurfaceId *) stack->storage;
    const MmSurfaceId *top = (MmSurfaceId *) stack->top;
    for (const MmSurfaceId *pid = base; pid < top; ++pid) {
        sprite_update_position(&graphics_surfaces[*pid]);
    }
    return kOk;
}

MmResult sprite_move() {
    ON_FAILURE_RETURN(sprite_hide_all());
    ON_FAILURE_RETURN(sprite_update_all_positions());
    return sprite_restore_all();
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

static inline void sprite_clear_collisions(MmSurface *sprite) {
    // Clear this sprites collisions with other sprites.
    bitset_reset(sprite->sprite_collisions, 256);

    // Clear other sprite's collisions with this sprite.
    // There may be more 'elegant' ways to do this, but I suspect just clearing the appropriate bit
    // for all surfaces without any checking logic may be fastest.
    for (MmSurfaceId id = 0; id < GRAPHICS_MAX_ID; ++id) {
        bitset_clear(graphics_surfaces[id].sprite_collisions, sprite->id);
    }

    // Clear this sprites collisions with edges.
    sprite->edge_collisions = 0x0;
}

MmResult sprite_update_collisions(MmSurface *sprite) {
    assert(sprite);

    sprite_clear_collisions(sprite);

    switch (sprite->type) {
        case kGraphicsSprite: break;
        case kGraphicsInactiveSprite: return kOk;
        default: return mmresult_ex(kGraphicsInvalidSprite, "Invalid sprite: %d", sprite->id);
    }
    assert(sprite->type == kGraphicsSprite);

    //printf("Update collisions for %d\n", sprite->id - 128);

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

/** Clears all collisions for active sprites. */
static inline void sprite_clear_all_collisions() {
    for (MmSurface *sprite = sprite_first_active(); sprite; sprite = sprite_next_active(sprite)) {
        bitset_reset(sprite->sprite_collisions, 256);
        sprite->edge_collisions = 0x0;
    }
}

MmResult sprite_update_all_collisions() {
    sprite_clear_all_collisions();

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
    if (sprite->type != kGraphicsSprite && sprite->type != kGraphicsInactiveSprite) {
        return mmresult_ex(kGraphicsInvalidSprite, "Invalid sprite: %d", sprite->id);
    }
    if (start % 64 != 0) return kInternalFault;
    *bitset = ((uint64_t *) sprite->sprite_collisions)[start / 64];
    return kOk;
}

MmResult sprite_restore_all() {
    if (!sprite_all_hidden) return kSpritesNotHidden;
    ON_FAILURE_RETURN(sprite_tmp_restore());
    sprite_all_hidden = false;
    return sprite_update_all_collisions();
}

static inline void sprite_translate(MmSurface *sprite, int dx, int dy) {
    int xs = sprite->x + (sprite->width >> 1); // Add half the width.
    xs += dx;
    if (xs >= graphics_current->width) {
        xs -= graphics_current->width;
    } else if (xs < 0) {
        xs += graphics_current->width;
    }
    sprite->x = xs - (sprite->width >> 1);

    int ys = sprite->y + (sprite->height >> 1); // Add half the height.
    ys += dy;
    if (ys >= graphics_current->height) {
        ys -= graphics_current->height;
    } else if (ys < 0) {
        ys += graphics_current->height;
    }
    sprite->y = ys - (sprite->height >> 1);
}

MmResult sprite_scroll(int dx, int dy, MmGraphicsColour colour) {
    if (dx == 0 && dy == 0) return kOk;

    // Temporarily hide all sprites.
    ON_FAILURE_RETURN(sprite_tmp_hide(NULL));

    const MmSurfaceId *base = (MmSurfaceId *) sprite_z_stack.storage;
    const MmSurfaceId *top = (MmSurfaceId *) sprite_z_stack.top;
    for (const MmSurfaceId *pid = top - 1; pid >= base; --pid) {
        MmSurface *sprite = &graphics_surfaces[*pid];
        if (sprite->layer == 0) {
            // Translate all layer 0 sprites.
            sprite_translate(sprite, dx, dy);
        } else {
            // Update position of other sprites.
            sprite_update_position(sprite);
        }
    }

    // Scroll the background.
    if (colour < -2) colour = RGB_WHITE;
    ON_FAILURE_RETURN(graphics_scroll(graphics_current, dx, dy, colour));

    // Restore temporarily hidden sprites.
    ON_FAILURE_RETURN(sprite_tmp_restore());

    // Update collision state.
    return sprite_update_all_collisions();
}

static inline MmResult sprite_show_internal(MmSurface *sprite, MmSurface *dst_surface, int x, int y,
                                            unsigned layer, int blit_flags, bool add_to_stack) {
    assert(add_to_stack || stack_contains(&sprite_z_stack, sprite->id));

    sprite->x = x;
    sprite->y = y;
    sprite->layer = layer;
    if (blit_flags != -1) sprite->blit_flags = (unsigned) blit_flags;
    ON_FAILURE_RETURN(sprite_update_background(sprite, dst_surface));
    if (add_to_stack) ON_FAILURE_RETURN(stack_push(&sprite_z_stack, sprite->id));
    ON_FAILURE_RETURN(sprite_render(sprite, dst_surface));
    sprite->type = kGraphicsSprite;
    return kOk;
}

MmResult sprite_show(MmSurface *sprite, MmSurface *dst_surface, int x, int y,
                     unsigned layer, int blit_flags) {
    if (sprite_all_hidden) return kSpritesAreHidden;
    if (sprite->type != kGraphicsSprite && sprite->type != kGraphicsInactiveSprite) {
        return mmresult_ex(kGraphicsInvalidSprite, "Invalid sprite: %d", sprite->id);
    }
    if (dst_surface->type == kGraphicsNone) return kGraphicsInvalidWriteSurface;

    // Hide already visible sprite.
    if (sprite->type == kGraphicsSprite) sprite_hide_internal(sprite, dst_surface);

    ON_FAILURE_RETURN(sprite_show_internal(sprite, dst_surface, x, y, layer, blit_flags, true));
    return sprite_update_collisions(sprite);

}

MmResult sprite_show_safe(MmSurface *sprite, MmSurface *dst_surface, int x, int y,
                          unsigned layer, int blit_flags, bool ontop) {
    if (sprite->type == kGraphicsInactiveSprite) {
        return sprite_show(sprite, dst_surface, x, y, layer, blit_flags);
    }

    if (sprite_all_hidden) return kSpritesAreHidden;
    if (sprite->type != kGraphicsSprite) {
        return mmresult_ex(kGraphicsInvalidSprite, "Invalid sprite: %d", sprite->id);
    }
    if (dst_surface->type == kGraphicsNone) return kGraphicsInvalidWriteSurface;

    // 1) Temporarily hide all sprites shown after this sprite.
    ON_FAILURE_RETURN(sprite_tmp_hide(sprite));

    // 2) Restore the background for this sprite.
    ON_FAILURE_RETURN(sprite_render_background(sprite, dst_surface));

    if (ontop) {
        // 4) Remove the sprite from the Z-order stack.
        ON_FAILURE_RETURN(stack_remove(&sprite_z_stack, sprite->id));
    } else {
        // 4) Properly show this sprite.
        ON_FAILURE_RETURN(sprite_show_internal(sprite, dst_surface, x, y, layer, blit_flags,
                                               false));
    }

    // 5) Restore sprites from step (1).
    ON_FAILURE_RETURN(sprite_tmp_restore());

    // 6) If ontop == true then properly show this sprite.
    if (ontop) ON_FAILURE_RETURN(sprite_show_internal(sprite, dst_surface, x, y, layer, blit_flags,
                                                      true));

    return sprite_update_collisions(sprite);
}

MmResult sprite_set_transparent_colour(MmGraphicsColour colour) {
    if (colour < 0) return kInternalFault;
    sprite_transparent_colour = colour;
    return kOk;
}
