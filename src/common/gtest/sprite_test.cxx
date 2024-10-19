/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gmock/gmock.h>  // Needed for EXPECT_THAT.
#include <gtest/gtest.h>

extern "C" {

#include <SDL.h>

#include "../bitset.h"
#include "../error.h"
#include "../interrupt.h"
#include "../sprite.h"
#include "../../third_party/spbmp.h"

char error_msg[256];

// Defined in "main.c"
char *CFunctionFlash;
char *CFunctionLibrary;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Options mmb_options;
ErrorState mmb_normal_error_state;
uint8_t mmb_exit_code = 0;

void CheckAbort(void) {}
int MMgetchar(void) { return 0; }
void MMgetline(int fnbr, char *p) {}

// Defined in "common/audio.c"
const char *audio_last_error() { return NULL; }

// Defined in "common/console.c"
int console_kbhit(void) { return 0; }
char console_putc(char c) { return c; }
void console_puts(const char *s) {}
void console_set_title(const char *title) {}
size_t console_write(const char *buf, size_t sz) { return 0; }

// Defined in "common/error.c"
void error_init(ErrorState *error_state) {}
MmResult error_throw(MmResult result) { return error_throw_ex(result, mmresult_to_string(result)); }
MmResult error_throw_ex(MmResult result, const char *msg, ...) {
    strcpy(error_msg, msg);
    return result;
}
MmResult error_throw_legacy(const char *msg, ...) { return error_throw_ex(kError, msg); }

// Defined in "common/keyboard.c"
MmResult keyboard_key_down(const SDL_Keysym *keysym) { return kError; }
MmResult keyboard_key_up(const SDL_Keysym *keysym) { return kError; }

// Defined in "common/program.c"
char CurrentFile[STRINGSIZE];

// Defined in "common/serial.c"
MmResult serial_close(int fnbr) { return kError; }
int serial_eof(int fnbr) { return -1; }
int serial_getc(int fnbr) { return -1; }
int serial_putc(int fnbr, int ch) { return -1; }
int serial_rx_queue_size(int fnbr) { return -1; }
int serial_write(int fnbr, const char *buf, size_t sz) { return -1; }

// Defined in "common/spbmp.c"
void spbmp_init(
    SpBmpFileReadCb file_read_cb,
    SpBmpSetPixelCb set_pixel_cb,
    SpBmpAbortCheckCb abort_check_cb) {}
SpBmpResult spbmp_load(void *userdata, int x, int y, void *file) { return kSpBmpError; }

// Defined in "core/MMBasic.c"
const char *CurrentLinePtr = NULL;
int LocalIndex = 0;

long long int getinteger(char *p) { return 0; }
int getint(char *p, int min, int max) { return 0; }
void makeargs(const char **tp, int maxargs, char *argbuf, char *argv[], int *argc,
              const char *delim) {}

}  // extern "C"

class SpriteTest : public ::testing::Test {
   protected:
    void SetUp() override {
        graphics_init();

        // Create a surface for the sprites to sit on.
        (void) graphics_buffer_create(0, 640, 480);
        graphics_current = &graphics_surfaces[0];

        // Show 10 sprites 10x10 at (x,y) = (20, 20), (40, 40), (60, 60), ...
        for (MmSurfaceId id = 1; id <= 10; ++id) {
            EXPECT_EQ(kOk, graphics_sprite_create(id, 10, 10));
            // (void) sprite_activate(&graphics_surfaces[id], true);
            MmSurface *sprite = &graphics_surfaces[id];
            const uint8_t layer = 1;
            EXPECT_EQ(kOk, sprite_show(sprite, graphics_current, id * 20, id * 20, layer, 0x0));
        }

        GivenNoCollisions();
    }

    void TearDown() override { graphics_term(); }

    void GivenNoCollisions() {
        for (MmSurfaceId id = 1; id <= 10; ++id) {
            graphics_surfaces[id].x = id * 20;
            graphics_surfaces[id].y = id * 20;
        }
    }

    void GivenSurfacesOverlapping(MmSurfaceId id1, MmSurfaceId id2) {
        MmSurface *sprite1 = &graphics_surfaces[id1];
        MmSurface *sprite2 = &graphics_surfaces[id2];
        sprite2->x = sprite1->x + 8;
        sprite2->y = sprite1->y + 8;
    }

    void GivenSurfaceOverlappingEdge(MmSurfaceId id, uint16_t edges) {
        MmSurface *sprite = &graphics_surfaces[id];
        if (edges & kSpriteEdgeLeft) sprite->x = -5;
        if (edges & kSpriteEdgeRight) sprite->x = graphics_current->width - (sprite->width / 2);
        if (edges & kSpriteEdgeTop) sprite->y = -5;
        if (edges & kSpriteEdgeBottom) sprite->y = graphics_current->height - (sprite->height / 2);
    }

    void GivenSpriteOverlappingAllOtherSpritesAndEdge() {
        GivenSurfaceOverlappingEdge(1, kSpriteEdgeLeft | kSpriteEdgeTop);
        for (MmSurfaceId id = 2; id <= 10; ++id) GivenSurfacesOverlapping(1, id);
    }

    void GivenSpriteAt(MmSurfaceId id, int32_t x, int32_t y) {
        MmSurface *sprite = &graphics_surfaces[id];
        sprite->x = x;
        sprite->y = y;
    }

    void GivenAllSpritesHidden() {
        // Hide all the sprites and check the stacks are empty.
        for (MmSurfaceId id = 1; id <= 10; ++id) {
            EXPECT_EQ(kOk, sprite_hide(&graphics_surfaces[id]));
        }
        EXPECT_EQ(0, stack_size(sprite_get_stack(0)));
        EXPECT_EQ(0, stack_size(sprite_get_stack(1)));
    }
};

TEST_F(SpriteTest, Create) {
    const MmSurfaceId id = 11;

    EXPECT_EQ(kOk, graphics_sprite_create(id, 10, 20));

    MmSurface *sprite = &graphics_surfaces[id];
    EXPECT_EQ(id, sprite->id);
    EXPECT_EQ(kGraphicsInactiveSprite, sprite->type);
    EXPECT_EQ(false, sprite->dirty);
    EXPECT_EQ(10, sprite->width);
    EXPECT_EQ(20, sprite->height);
    EXPECT_EQ(-1, sprite->transparent);
    EXPECT_EQ(GRAPHICS_OFF_SCREEN, sprite->x);
    EXPECT_EQ(GRAPHICS_OFF_SCREEN, sprite->y);
    EXPECT_EQ(0xFF, sprite->layer);
    EXPECT_EQ(kBlitWithTransparency, sprite->blit_flags);
    EXPECT_EQ(0x0, sprite->edge_collisions);
    for (size_t ii = 0; ii < 32 / sizeof(int); ++ii) {
        EXPECT_EQ(0x0, sprite->sprite_collisions[ii]);
    }
}

TEST_F(SpriteTest, GetNumCollisions_Returns_0_GivenSpriteNotOverlapping) {
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    MmSurface *sprite1 = &graphics_surfaces[1];
    uint32_t count = 0;

    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(0, count);
}

TEST_F(SpriteTest, GetNumCollisions_Returns_1_GivenSpriteOverlappingLeft) {
    GivenSurfaceOverlappingEdge(1, kSpriteEdgeLeft);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    MmSurface *sprite1 = &graphics_surfaces[1];
    uint32_t count = 0;

    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(1, count);
}

TEST_F(SpriteTest, GetNumCollisions_Returns_1_GivenSpriteOverlappingBottomAndRight) {
    GivenSurfaceOverlappingEdge(1, kSpriteEdgeBottom | kSpriteEdgeRight);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    MmSurface *sprite1 = &graphics_surfaces[1];
    uint32_t count = 0;

    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(1, count);
}

TEST_F(SpriteTest, GetNumCollisions_Returns_1_GivenSurfacesOverlapping) {
    GivenSurfacesOverlapping(1, 3);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    MmSurface *sprite1 = &graphics_surfaces[1];
    MmSurface *sprite3 = &graphics_surfaces[3];
    uint32_t count = 0;

    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(1, count);

    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite3, &count));
    EXPECT_EQ(1, count);
}

TEST_F(SpriteTest, GetNumCollisions_Returns_10_GivenSpriteOverlappingAllOtherSpritesAndEdge) {
    GivenSpriteOverlappingAllOtherSpritesAndEdge();
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    MmSurface *sprite1 = &graphics_surfaces[1];
    uint32_t count = 0;

    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(10, count); // 9 other sprites + edge
}

TEST_F(SpriteTest, GetCollision_GivenSpriteNotOverlapping) {
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    MmSurface *sprite1 = &graphics_surfaces[1];
    MmSurfaceId other_id = -2;

    EXPECT_EQ(kOk, sprite_get_collision(sprite1, 1, &other_id));
    EXPECT_EQ(-1, other_id);

    EXPECT_EQ(kOk, sprite_get_collision(sprite1, 2, &other_id));
    EXPECT_EQ(-1, other_id);
}

TEST_F(SpriteTest, GetCollision_GivenSpriteOverlappingLeft) {
    GivenSurfaceOverlappingEdge(1, kSpriteEdgeLeft);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    MmSurface *sprite1 = &graphics_surfaces[1];
    MmSurfaceId other_id = -2;

    EXPECT_EQ(kOk, sprite_get_collision(sprite1, 1, &other_id));
    EXPECT_EQ(0xFFF0 | kSpriteEdgeLeft, other_id);

    EXPECT_EQ(kOk, sprite_get_collision(sprite1, 2, &other_id));
    EXPECT_EQ(-1, other_id);
}

TEST_F(SpriteTest, GetCollision_GivenSpriteOverlappingBottomAndRight) {
    GivenSurfaceOverlappingEdge(1, kSpriteEdgeBottom | kSpriteEdgeRight);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    MmSurface *sprite1 = &graphics_surfaces[1];
    MmSurfaceId other_id = -2;

    EXPECT_EQ(kOk, sprite_get_collision(sprite1, 1, &other_id));
    EXPECT_EQ(0xFFF0 | kSpriteEdgeBottom | kSpriteEdgeRight, other_id);

    EXPECT_EQ(kOk, sprite_get_collision(sprite1, 2, &other_id));
    EXPECT_EQ(-1, other_id);
}

TEST_F(SpriteTest, GetCollision_GivenSurfacesOverlapping) {
    GivenSurfacesOverlapping(1, 3);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    MmSurface *sprite1 = &graphics_surfaces[1];
    MmSurface *sprite3 = &graphics_surfaces[3];
    MmSurfaceId other_id = -2;

    EXPECT_EQ(kOk, sprite_get_collision(sprite1, 1, &other_id));
    EXPECT_EQ(3, other_id);

    EXPECT_EQ(kOk, sprite_get_collision(sprite1, 2, &other_id));
    EXPECT_EQ(-1, other_id);

    EXPECT_EQ(kOk, sprite_get_collision(sprite3, 1, &other_id));
    EXPECT_EQ(1, other_id);

    EXPECT_EQ(kOk, sprite_get_collision(sprite3, 2, &other_id));
    EXPECT_EQ(-1, other_id);
}

TEST_F(SpriteTest, GetCollision_GivenSpriteOverlappingAllOtherSpritesAndEdge) {
    GivenSpriteOverlappingAllOtherSpritesAndEdge();
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    MmSurface *sprite1 = &graphics_surfaces[1];
    MmSurfaceId other_id = -2;

    for (size_t ii = 1; ii < 10; ++ii) {
        EXPECT_EQ(kOk, sprite_get_collision(sprite1, ii, &other_id));
        EXPECT_EQ(ii + 1, other_id);
    }
    EXPECT_EQ(kOk, sprite_get_collision(sprite1, 10, &other_id));
    EXPECT_EQ(0xFFF0 | kSpriteEdgeLeft | kSpriteEdgeTop, other_id);
    EXPECT_EQ(kOk, sprite_get_collision(sprite1, 11, &other_id));
    EXPECT_EQ(-1, other_id);

    for (MmSurfaceId id = 2; id <= 10; ++id) {
        EXPECT_EQ(kOk, sprite_get_collision(&graphics_surfaces[id], 1, &other_id));
        EXPECT_EQ(1, other_id);
    }
}

TEST_F(SpriteTest, GetNumCollidedSprites_Returns_1_GivenSpriteNotOverlapping) {
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count = 0;

    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(0, count);
}

TEST_F(SpriteTest, GetNumCollidedSprites_Returns_1_GivenSpriteOverlappingLeft) {
    GivenSurfaceOverlappingEdge(1, kSpriteEdgeLeft);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count = 0;

    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(1, count);
}

TEST_F(SpriteTest, GetNumCollidedSprites_Returns_2_GivenSurfacesOverlapping) {
    GivenSurfacesOverlapping(1, 3);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count = 0;

    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(2, count);
}

TEST_F(SpriteTest, GetNumCollidedSprites_Returns_10_GivenSpriteOverlappingAllOtherSpritesAndEdge) {
    GivenSpriteOverlappingAllOtherSpritesAndEdge();
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count = 0;

    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(10, count);
}

TEST_F(SpriteTest, GetCollidedSprite_GivenSpriteNotOverlapping) {
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    MmSurfaceId id = -2;

    EXPECT_EQ(kOk, sprite_get_collided_sprite(1, &id));
    EXPECT_EQ(-1, id);

    EXPECT_EQ(kOk, sprite_get_collided_sprite(2, &id));
    EXPECT_EQ(-1, id);
}

TEST_F(SpriteTest, GetCollidedSprite_GivenSpriteOverlappingLeft) {
    GivenSurfaceOverlappingEdge(1, kSpriteEdgeLeft);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    MmSurfaceId id = -2;

    EXPECT_EQ(kOk, sprite_get_collided_sprite(1, &id));
    EXPECT_EQ(1, id);

    EXPECT_EQ(kOk, sprite_get_collided_sprite(2, &id));
    EXPECT_EQ(-1, id);
}

TEST_F(SpriteTest, GetCollidedSprite_GivenSurfacesOverlapping) {
    GivenSurfacesOverlapping(1, 3);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    MmSurfaceId id = -2;

    EXPECT_EQ(kOk, sprite_get_collided_sprite(1, &id));
    EXPECT_EQ(1, id);

    EXPECT_EQ(kOk, sprite_get_collided_sprite(2, &id));
    EXPECT_EQ(3, id);

    EXPECT_EQ(kOk, sprite_get_collided_sprite(3, &id));
    EXPECT_EQ(-1, id);
}

TEST_F(SpriteTest, GetCollidedSprite_GivenSpriteOverlappingAllOtherSpritesAndEdge) {
    GivenSpriteOverlappingAllOtherSpritesAndEdge();
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    for (size_t ii = 1; ii <= 10; ++ii) {
        MmSurfaceId id = -2;
        EXPECT_EQ(kOk, sprite_get_collided_sprite(ii, &id));
        EXPECT_EQ(ii, id);
    }
}

TEST_F(SpriteTest, SpritesOnSameNonZeroLayersDoCollide) {
    MmSurface *sprite1 = &graphics_surfaces[1];
    sprite1->layer = 1;
    MmSurface *sprite3 = &graphics_surfaces[3];
    sprite3->layer = 1;
    GivenSurfacesOverlapping(1, 3);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count;

    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(1, count);
    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite3, &count));
    EXPECT_EQ(1, count);
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(2, count);
}

TEST_F(SpriteTest, SpritesOnDifferentNonZeroLayersDoNotCollide) {
    MmSurface *sprite1 = &graphics_surfaces[1];
    sprite1->layer = 1;
    MmSurface *sprite3 = &graphics_surfaces[3];
    sprite3->layer = 2;
    GivenSurfacesOverlapping(1, 3);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count;

    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(0, count);
    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite3, &count));
    EXPECT_EQ(0, count);
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(0, count);
}

TEST_F(SpriteTest, SpritesOnLayerZeroAlwaysCollide) {
    MmSurface *sprite1 = &graphics_surfaces[1];
    sprite1->layer = 1;
    MmSurface *sprite3 = &graphics_surfaces[3];
    sprite3->layer = 0;
    GivenSurfacesOverlapping(1, 3);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count;

    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(1, count);
    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite3, &count));
    EXPECT_EQ(1, count);
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(2, count);
}

TEST_F(SpriteTest, InactiveSpritesDoNotCollideWithSprites) {
    MmSurface *sprite1 = &graphics_surfaces[1];
    MmSurface *sprite3 = &graphics_surfaces[3];
    EXPECT_EQ(kOk, sprite_show(sprite1, graphics_current, sprite1->x, sprite1->y, sprite1->layer,
                               0x0));
    EXPECT_EQ(kOk, sprite_hide(sprite3)); // hidden = inactive
    GivenSurfacesOverlapping(1, 3);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count;

    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(0, count);
    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite3, &count));
    EXPECT_EQ(0, count);
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(0, count);
}

TEST_F(SpriteTest, InactiveSpritesDoNotCollideWithEdges) {
    MmSurface *sprite1 = &graphics_surfaces[1];
    EXPECT_EQ(kOk, sprite_hide(sprite1)); // hidden = inactive
    GivenSurfaceOverlappingEdge(1, kSpriteEdgeLeft);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count;

    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(0, count);
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(0, count);
}

TEST_F(SpriteTest, SpritesDoNotCollideWithBuffers) {
    MmSurface *sprite1 = &graphics_surfaces[1];
    EXPECT_EQ(kOk, graphics_buffer_create(20, 10, 10));
    GivenSurfacesOverlapping(1, 20);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count;

    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(0, count);
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(0, count);
}

TEST_F(SpriteTest, SpritesDoNotCollideWithWindows) {
    EXPECT_EQ(kOk, graphics_window_create(20, 10, 10, 0, 0, 1, NULL, NULL, true));
    GivenSurfacesOverlapping(1, 20);

    uint32_t count;

    EXPECT_EQ(kOk, sprite_get_num_collisions(&graphics_surfaces[1], &count));
    EXPECT_EQ(0, count);
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(0, count);
}

TEST_F(SpriteTest, BuffersDoNotCollideWithEdges) {
    EXPECT_EQ(kOk, graphics_buffer_create(20, 10, 10));
    GivenSurfaceOverlappingEdge(20, kSpriteEdgeLeft);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count;

    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(0, count);
}

TEST_F(SpriteTest, WindowsDoNotCollideWithEdges) {
    EXPECT_EQ(kOk, graphics_window_create(20, 10, 10, 0, 0, 1, NULL, NULL, true));
    GivenSurfaceOverlappingEdge(20, kSpriteEdgeLeft);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count;

    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(0, count);
}

TEST_F(SpriteTest, UpdateAllCollisions_RegistersCollisions_GivenSpritesTouchingButNotOverlappingSprites) {
    // Arrange 9 sprites in 3x3 grid touching each other - sprites are width 10, height 10.
    GivenSpriteAt(1, 20, 20);
    GivenSpriteAt(2, 30, 20);
    GivenSpriteAt(3, 40, 20);
    GivenSpriteAt(4, 20, 30);
    GivenSpriteAt(5, 30, 30);
    GivenSpriteAt(6, 40, 30);
    GivenSpriteAt(7, 20, 40);
    GivenSpriteAt(8, 30, 40);
    GivenSpriteAt(9, 40, 40);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count;

    uint32_t expected[] = { 3, 5, 3, 5, 8, 5, 3, 5, 3, 0 };
    for (MmSurfaceId id = 1; id <= 10; ++id) {
        EXPECT_EQ(kOk, sprite_get_num_collisions(&graphics_surfaces[id], &count));
        EXPECT_EQ(expected[id - 1], count);
    }
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(9, count);
}

TEST_F(SpriteTest, UpdateAllCollisions_DoesNotRegisterCollisions_GivenSpritesTouchingButNotOverlappingEdges) {
    // Arrange 4 sprites at corners of the 640x480 surface.    
    GivenSpriteAt(1, 0, 0);
    GivenSpriteAt(2, 630, 0);
    GivenSpriteAt(3, 0, 470);
    GivenSpriteAt(4, 630, 470);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    uint32_t count;

    for (MmSurfaceId id = 1; id <= 1; ++id) {
        EXPECT_EQ(kOk, sprite_get_num_collisions(&graphics_surfaces[id], &count));
        EXPECT_EQ(0, count);
    }
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(0, count);

    // For good measure, slightly overlap.
    GivenSpriteAt(1, -1, -1);
    GivenSpriteAt(2, 631, -1);
    GivenSpriteAt(3, -1, 471);
    GivenSpriteAt(4, 631, 471);
    EXPECT_EQ(kOk, sprite_update_all_collisions());

    for (MmSurfaceId id = 1; id <= 1; ++id) {
        EXPECT_EQ(kOk, sprite_get_num_collisions(&graphics_surfaces[id], &count));
        EXPECT_EQ(1, count);
    }
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(4, count);
}

TEST_F(SpriteTest, UpdateCollisions_RegistersCollision_GivenSpriteOverlappingOtherSprite) {
    MmSurface *sprite1 = &graphics_surfaces[1];
    MmSurface *sprite3 = &graphics_surfaces[3];
    uint32_t count;
    MmSurfaceId sprite_id;

    // Assert that we've started with no collisions.
    EXPECT_EQ(kOk, sprite_update_all_collisions());
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(0, count);

    GivenSurfacesOverlapping(1, 3);
    EXPECT_EQ(kOk, sprite_update_collisions(sprite1));

    // Check sprite 1 collisions.
    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(1, count);
    EXPECT_EQ(kOk, sprite_get_collision(sprite1, 1, &sprite_id));
    EXPECT_EQ(3, sprite_id);

    // Check sprite 3 collisions.
    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite3, &count));
    EXPECT_EQ(1, count);
    EXPECT_EQ(kOk, sprite_get_collision(sprite3, 1, &sprite_id));
    EXPECT_EQ(1, sprite_id);

    // Check global collisions.
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(2, count);
    EXPECT_EQ(kOk, sprite_get_collided_sprite(1, &sprite_id));
    EXPECT_EQ(1, sprite_id);
    EXPECT_EQ(kOk, sprite_get_collided_sprite(2, &sprite_id));
    EXPECT_EQ(3, sprite_id);

    ///////////////////////////////////////////
    // For good measure un-collide the sprites.
    ///////////////////////////////////////////

    sprite3->x = sprite1->x + 20;
    EXPECT_EQ(kOk, sprite_update_collisions(sprite1));

    for (MmSurfaceId id = 1; id <= 10; ++id) {
        MmSurface *sprite = &graphics_surfaces[id];
        EXPECT_EQ(kOk, sprite_get_num_collisions(sprite, &count));
        EXPECT_EQ(0, count);
        EXPECT_EQ(kOk, sprite_get_collision(sprite, 1, &sprite_id));
        EXPECT_EQ(-1, sprite_id);
    }

    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(0, count);
    EXPECT_EQ(kOk, sprite_get_collided_sprite(1, &sprite_id));
    EXPECT_EQ(-1, sprite_id);
}

TEST_F(SpriteTest, UpdateCollisions_RegistersCollision_GivenSurfaceOverlappingEdge) {
    MmSurface *sprite1 = &graphics_surfaces[1];
    MmSurfaceId sprite_id;
    uint32_t count;

    // Assert that we've started with no collisions.
    EXPECT_EQ(kOk, sprite_update_all_collisions());
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(0, count);

    GivenSurfaceOverlappingEdge(1, kSpriteEdgeLeft);
    EXPECT_EQ(kOk, sprite_update_collisions(sprite1));

    // Check sprite 1 collisions.
    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(1, count);
    EXPECT_EQ(kOk, sprite_get_collision(sprite1, 1, &sprite_id));
    EXPECT_EQ(0xFFF0 | kSpriteEdgeLeft, sprite_id);

    // Check global collisions.
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(1, count);
    EXPECT_EQ(kOk, sprite_get_collided_sprite(1, &sprite_id));
    EXPECT_EQ(1, sprite_id);

    //////////////////////////////////////////
    // For good measure un-collide the sprite.
    //////////////////////////////////////////

    GivenSpriteAt(1, 20, 20);
    EXPECT_EQ(kOk, sprite_update_collisions(sprite1));

    // Check sprite 1 collisions.
    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(0, count);
    EXPECT_EQ(kOk, sprite_get_collision(sprite1, 1, &sprite_id));
    EXPECT_EQ(-1, sprite_id);

    // Check global collisions.
    EXPECT_EQ(kOk, sprite_get_num_collided_sprites(&count));
    EXPECT_EQ(0, count);
    EXPECT_EQ(kOk, sprite_get_collided_sprite(1, &sprite_id));
    EXPECT_EQ(-1, sprite_id);
}

TEST_F(SpriteTest, GetCollisionBitSet_GivenNoCollisions) {
    MmSurface *sprite1 = &graphics_surfaces[1];
    uint64_t bitset;

    GivenNoCollisions();

    EXPECT_EQ(kOk, sprite_get_collision_bitset(sprite1, 0, &bitset));
    EXPECT_EQ(0, bitset);
    EXPECT_EQ(kOk, sprite_get_collision_bitset(sprite1, 64, &bitset));
    EXPECT_EQ(0, bitset);
    EXPECT_EQ(kOk, sprite_get_collision_bitset(sprite1, 128, &bitset));
    EXPECT_EQ(0, bitset);
    EXPECT_EQ(kOk, sprite_get_collision_bitset(sprite1, 192, &bitset));
    EXPECT_EQ(0, bitset);
}

TEST_F(SpriteTest, GetCollisionBitSet_GivenCollisions) {
    MmSurface *sprite1 = &graphics_surfaces[1];
    uint64_t bitset;

    // Cheat on the collisions by updating sprite1->sprite_collisions directly.
    bitset_set(sprite1->sprite_collisions, 0);
    bitset_set(sprite1->sprite_collisions, 2);
    bitset_set(sprite1->sprite_collisions, 32);
    bitset_set(sprite1->sprite_collisions, 63);
    bitset_set(sprite1->sprite_collisions, 0 + 64);
    bitset_set(sprite1->sprite_collisions, 2 + 64 + 1);
    bitset_set(sprite1->sprite_collisions, 32 + 64 + 1);
    bitset_set(sprite1->sprite_collisions, 63 + 64 - 1);
    bitset_set(sprite1->sprite_collisions, 0 + 128);
    bitset_set(sprite1->sprite_collisions, 2 + 128 + 2);
    bitset_set(sprite1->sprite_collisions, 32 + 128 + 2);
    bitset_set(sprite1->sprite_collisions, 63 + 128 - 2);
    bitset_set(sprite1->sprite_collisions, 0 + 192);
    bitset_set(sprite1->sprite_collisions, 2 + 192 + 3);
    bitset_set(sprite1->sprite_collisions, 32+ 192 + 3);
    bitset_set(sprite1->sprite_collisions, 63 + 192 - 3);
    
    EXPECT_EQ(kOk, sprite_get_collision_bitset(sprite1, 0, &bitset));
    EXPECT_EQ(0b1000000000000000000000000000000100000000000000000000000000000101, bitset);
    EXPECT_EQ(kOk, sprite_get_collision_bitset(sprite1, 64, &bitset));
    EXPECT_EQ(0b0100000000000000000000000000001000000000000000000000000000001001, bitset);
    EXPECT_EQ(kOk, sprite_get_collision_bitset(sprite1, 128, &bitset));
    EXPECT_EQ(0b0010000000000000000000000000010000000000000000000000000000010001, bitset);
    EXPECT_EQ(kOk, sprite_get_collision_bitset(sprite1, 192, &bitset));
    EXPECT_EQ(0b0001000000000000000000000000100000000000000000000000000000100001, bitset);
}

TEST_F(SpriteTest, Hide_Fails_GivenSpriteInactive) {
    MmSurface *sprite1 = &graphics_surfaces[1];

    EXPECT_EQ(kOk, sprite_hide(sprite1)); // hidden = inactive

    EXPECT_EQ(kSpriteInactive, sprite_hide(sprite1));
}

TEST_F(SpriteTest, HideAll_Fails_GivenHideAllActive) {
    EXPECT_EQ(kOk, sprite_hide_all());

    EXPECT_EQ(kSpritesAreHidden, sprite_hide_all());
}

TEST_F(SpriteTest, Show_Fails_GivenHideAllActive) {
    EXPECT_EQ(kOk, sprite_hide_all());

    MmSurface *sprite1 = &graphics_surfaces[1];
    EXPECT_EQ(kSpritesAreHidden, sprite_show(sprite1, graphics_current, sprite1->x, sprite1->y,
                                             sprite1->layer, 0x0));
}

TEST_F(SpriteTest, Hide_ClearsAllSpritesCollisions) {
    GivenSpriteOverlappingAllOtherSpritesAndEdge();
    EXPECT_EQ(kOk, sprite_update_all_collisions());
    MmSurface *sprite1 = &graphics_surfaces[1];
    uint32_t count;
    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(10, count); // 9 other sprites + edge(s).

    EXPECT_EQ(kOk, sprite_hide(sprite1));

    // sprite1 should not believe it has any collisions.
    EXPECT_EQ(0x0, sprite1->edge_collisions);
    for (MmSurfaceId id = 0; id < GRAPHICS_MAX_ID; ++id) {
        EXPECT_EQ(false, bitset_get(sprite1->sprite_collisions, id));
    }

    // None of the other sprites should believe they have collided with sprite1.
    for (MmSurfaceId id = 2; id <= 10; ++id) {
        MmSurface *other = &graphics_surfaces[id];
        EXPECT_EQ(false, bitset_get(other->sprite_collisions, 1));
    }
}

TEST_F(SpriteTest, Destroy_ClearsAllSpritesCollisions) {
    GivenSpriteOverlappingAllOtherSpritesAndEdge();
    EXPECT_EQ(kOk, sprite_update_all_collisions());
    MmSurface *sprite1 = &graphics_surfaces[1];
    uint32_t count;
    EXPECT_EQ(kOk, sprite_get_num_collisions(sprite1, &count));
    EXPECT_EQ(10, count); // 9 other sprites + edge(s).

    EXPECT_EQ(kOk, sprite_destroy(sprite1));

    // sprite 1 should not believe it has any collisions.
    EXPECT_EQ(0x0, sprite1->edge_collisions);
    for (MmSurfaceId id = 0; id < GRAPHICS_MAX_ID; ++id) {
        EXPECT_EQ(false, bitset_get(sprite1->sprite_collisions, id));
    }

    // None of the other sprites should believe they have collided with sprite 1.
    for (MmSurfaceId id = 2; id <= 10; ++id) {
        MmSurface *other = &graphics_surfaces[id];
        EXPECT_EQ(false, bitset_get(other->sprite_collisions, 1));
    }
}

TEST_F(SpriteTest, Destroy_RemovesSpriteFromStack) {
    MmSurface *sprite1 = &graphics_surfaces[1];

    // Check that sprite is in the sprite stack.
    EXPECT_EQ(kGraphicsSprite, sprite1->type);
    EXPECT_TRUE(stack_contains(sprite_get_stack(1), sprite1->id));

    EXPECT_EQ(kOk, sprite_destroy(sprite1));
    EXPECT_FALSE(stack_contains(sprite_get_stack(1), sprite1->id));

    // Check that sprite is no longer in the sprite stack.
    EXPECT_EQ(kGraphicsNone, sprite1->type);
}

TEST_F(SpriteTest, IdToSurfaceId_GivenValidId_Succeeds) {
    mmb_options.simulate = kSimulateMmb4l;
    for (MmSurfaceId sprite_id = 0; sprite_id <= 255; ++sprite_id) {
        const MmSurfaceId expected_surface_id = sprite_id;
        EXPECT_EQ(expected_surface_id, sprite_id_to_surface_id(sprite_id));
    }

    const OptionsSimulate sim[] = {
        kSimulateMmb4w,
        kSimulateCmm2,
        kSimulatePicoMiteVga,
        kSimulateGameMite
    };

    for (size_t i = 0; i < sizeof(sim) / sizeof(OptionsSimulate); ++i) {
        mmb_options.simulate = sim[i];
        for (MmSurfaceId sprite_id = 0; sprite_id <= 64; ++sprite_id) {
            const MmSurfaceId expected_surface_id = (sprite_id == 0) ? 0 : sprite_id + 127;
            EXPECT_EQ(expected_surface_id, sprite_id_to_surface_id(sprite_id));
        }
    }
}

TEST_F(SpriteTest, IdToSurfaceId_GivenInvalidId_ReturnsMinusOne) {
    mmb_options.simulate = kSimulateMmb4l;
    EXPECT_EQ(-1, sprite_id_to_surface_id(-1));
    EXPECT_EQ(-1, sprite_id_to_surface_id(256));

    const OptionsSimulate sim[] = {
        kSimulateMmb4w,
        kSimulateCmm2,
        kSimulatePicoMiteVga,
        kSimulateGameMite
    };

    for (size_t i = 0; i < sizeof(sim) / sizeof(OptionsSimulate); ++i) {
        mmb_options.simulate = sim[i];
        EXPECT_EQ(-1, sprite_id_to_surface_id(-1));
        EXPECT_EQ(-1, sprite_id_to_surface_id(65));
    }
}

TEST_F(SpriteTest, IdFromSurfaceId_GivenValidId_Succeeds) {
    mmb_options.simulate = kSimulateMmb4l;

    for (MmSurfaceId surface_id = 0; surface_id <= 255; ++surface_id) {
        const MmSurfaceId expected_sprite_id = surface_id;
        EXPECT_EQ(expected_sprite_id, sprite_id_from_surface_id(surface_id));
    }

    const OptionsSimulate sim[] = {
        kSimulateMmb4w,
        kSimulateCmm2,
        kSimulatePicoMiteVga,
        kSimulateGameMite
    };

    for (size_t i = 0; i < sizeof(sim) / sizeof(OptionsSimulate); ++i) {
        mmb_options.simulate = sim[i];
        for (MmSurfaceId surface_id = 128; surface_id <= 191; ++surface_id) {
            const MmSurfaceId expected_sprite_id = (surface_id == 0) ? 0 : surface_id - 127;
            EXPECT_EQ(expected_sprite_id, sprite_id_from_surface_id(surface_id));
        }
    }
}

TEST_F(SpriteTest, IdFromSurfaceId_GivenInvalidId_ReturnsMinusOne) {
    mmb_options.simulate = kSimulateMmb4l;
    EXPECT_EQ(-1, sprite_id_from_surface_id(-1));
    EXPECT_EQ(-1, sprite_id_from_surface_id(256));

    const OptionsSimulate sim[] = {
        kSimulateMmb4w,
        kSimulateCmm2,
        kSimulatePicoMiteVga,
        kSimulateGameMite
    };

    for (size_t i = 0; i < sizeof(sim) / sizeof(OptionsSimulate); ++i) {
        mmb_options.simulate = sim[i];
        EXPECT_EQ(-1, sprite_id_from_surface_id(CMM2_SPRITE_BASE));
        EXPECT_EQ(-1, sprite_id_from_surface_id(CMM2_SPRITE_BASE + CMM2_SPRITE_COUNT + 1));
    }
}

TEST_F(SpriteTest, IdIsInRange) {
    mmb_options.simulate = kSimulateMmb4l;
    EXPECT_EQ(false, sprite_id_is_in_range(0));
    EXPECT_EQ(true, sprite_id_is_in_range(1));
    EXPECT_EQ(true, sprite_id_is_in_range(255));
    EXPECT_EQ(false, sprite_id_is_in_range(256));

    const OptionsSimulate sim[] = {
        kSimulateMmb4w,
        kSimulateCmm2,
        kSimulatePicoMiteVga,
        kSimulateGameMite
    };
    for (size_t i = 0; i < sizeof(sim) / sizeof(OptionsSimulate); ++i) {
        mmb_options.simulate = sim[i];
        EXPECT_EQ(false, sprite_id_is_in_range(0));
        EXPECT_EQ(true, sprite_id_is_in_range(1));
        EXPECT_EQ(true, sprite_id_is_in_range(64));
        EXPECT_EQ(false, sprite_id_is_in_range(65));
    }
}

TEST_F(SpriteTest, MaxId) {
    mmb_options.simulate = kSimulateMmb4l;
    EXPECT_EQ(255, sprite_max_id());

    const OptionsSimulate sim[] = {
        kSimulateMmb4w,
        kSimulateCmm2,
        kSimulatePicoMiteVga,
        kSimulateGameMite
    };
    for (size_t i = 0; i < sizeof(sim) / sizeof(OptionsSimulate); ++i) {
        mmb_options.simulate = sim[i];
        EXPECT_EQ(64, sprite_max_id());
    }
}

TEST_F(SpriteTest, Show_GivenBlitFlagsMinus1_KeepsExistingBlitFlags) {
    MmSurface *sprite1 = &graphics_surfaces[1];
    EXPECT_EQ(kOk, graphics_surface_destroy(sprite1));
    EXPECT_EQ(kOk, graphics_sprite_create(1, 20, 20));

    // By default sprites are transparent.
    EXPECT_EQ(kBlitWithTransparency, sprite1->blit_flags);

    // Explicitly set blit flags.
    EXPECT_EQ(kOk, sprite_show(sprite1, graphics_current, 50, 50, 1, kBlitHorizontalFlip));
    EXPECT_EQ(kBlitHorizontalFlip, sprite1->blit_flags);

    // Use a value of 1 to keep existing blit flags.
    EXPECT_EQ(kOk, sprite_show(sprite1, graphics_current, 50, 50, 1, -1));
    EXPECT_EQ(kBlitHorizontalFlip, sprite1->blit_flags);
}

TEST_F(SpriteTest, ShowSafe_GivenOntop_AddsSpriteToTopOfStack) {
    GivenAllSpritesHidden();

    MmSurface *sprite1 = &graphics_surfaces[1];
    MmSurface *sprite2 = &graphics_surfaces[2];
    MmSurface *sprite3 = &graphics_surfaces[3];
    Stack *stack1 = sprite_get_stack(1);
    MmSurfaceId out;

    EXPECT_EQ(kOk, sprite_show_safe(sprite1, graphics_current, 0, 0, 1, 0x0, true));
    EXPECT_EQ(1, stack_size(stack1));
    EXPECT_EQ(kOk, stack_get(stack1, 0, &out));
    EXPECT_EQ(1, out);

    EXPECT_EQ(kOk, sprite_show_safe(sprite2, graphics_current, 0, 0, 1, 0x0, true));
    EXPECT_EQ(2, stack_size(stack1));
    EXPECT_EQ(kOk, stack_get(stack1, 1, &out));
    EXPECT_EQ(2, out);

    EXPECT_EQ(kOk, sprite_show_safe(sprite3, graphics_current, 0, 0, 1, 0x0, true));
    EXPECT_EQ(3, stack_size(stack1));
    EXPECT_EQ(kOk, stack_get(stack1, 2, &out));
    EXPECT_EQ(3, out);
}

TEST_F(SpriteTest, ShowSafe_GivenNotOntop_InsertsSpriteCorrectlyIntoStack) {
    GivenAllSpritesHidden();

    MmSurface *sprite1 = &graphics_surfaces[1];
    MmSurface *sprite2 = &graphics_surfaces[2];
    MmSurface *sprite3 = &graphics_surfaces[3];
    EXPECT_EQ(kOk, sprite_show_safe(sprite1, graphics_current, 0, 0, 1, 0x0, true));
    EXPECT_EQ(kOk, sprite_show_safe(sprite2, graphics_current, 0, 0, 1, 0x0, true));
    EXPECT_EQ(kOk, sprite_show_safe(sprite3, graphics_current, 0, 0, 1, 0x0, true));

    // Show sprite 2 again with ontop == false.
    EXPECT_EQ(kOk, sprite_show_safe(sprite2, graphics_current, 0, 0, 1, 0x0, false));

    // Sprite 2 should still be in element 1 of the stack (0 based).
    Stack *stack1 = sprite_get_stack(1);
    MmSurfaceId out;
    EXPECT_EQ(3, stack_size(stack1));
    EXPECT_EQ(kOk, stack_get(stack1, 0, &out));
    EXPECT_EQ(1, out);
    EXPECT_EQ(kOk, stack_get(stack1, 1, &out));
    EXPECT_EQ(2, out);
    EXPECT_EQ(kOk, stack_get(stack1, 2, &out));
    EXPECT_EQ(3, out);
}
