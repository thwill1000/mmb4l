/*
 * Copyright (c) 2024-2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gmock/gmock.h>  // Needed for EXPECT_THAT.
#include <gtest/gtest.h>

extern "C" {

#include <SDL.h>

#include "../error.h"
#include "../features.h"
#include "../graphics.h"
#include "../mmb4l.h"
#include "../../third_party/spbmp.h"

// Defined in "main.c"
char *CFunctionFlash;
char *CFunctionLibrary;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Features mmb_features;
Options mmb_options;
ErrorState mmb_normal_error_state;

void CheckAbort(void) {}
void MMgetline(int fnbr, char *p) {}

// Defined in "common/keyboard.c"
MmResult keyboard_key_down(const SDL_Keysym *keysym) { return kError; }
MmResult keyboard_key_up(const SDL_Keysym *keysym) { return kError; }

// Defined in "common/program.c"
char CurrentFile[STRINGSIZE];

// Defined in "common/prompt.c"
MmResult prompt_getc(int *ch) {
    *ch = -1;
    return kOk;
}

// Defined in "common/spbmp.c"
void spbmp_init(
    SpBmpFileReadCb file_read_cb,
    SpBmpFileWriteCb file_write_cb,
    SpBmpGetPixelCb get_pixel_cb,
    SpBmpSetPixelCb set_pixel_cb,
    SpBmpAbortCheckCb abort_check_cb) {}
SpBmpResult spbmp_load(void *file, int x, int y, void *userdata) { return kSpBmpError; }
SpBmpResult spbmp_save(void *file, SpBmpFormat format, void *userdata, int x, int y, int width,
                       int height) {
    return kSpBmpError;
}

// Defined in "common/sprite.c"
MmResult sprite_hide(MmSurface *sprite) { return kOk; }
MmResult sprite_init() { return kOk; }
MmResult sprite_term() { return kOk; }

// Defined in "core/MMBasic.c"
const char *CurrentLinePtr = NULL;
int LocalIndex = 0;

MMINTEGER getinteger(const char *p) { return 0; }
MMINTEGER getint(const char *p, MMINTEGER min, MMINTEGER max) { return 0; }
void makeargs(const char **tp, int maxargs, char *argbuf, char *argv[], int *argc,
              const DelimType *delim) {}

}  // extern "C"

class GraphicsTest : public ::testing::Test {
   protected:
    void SetUp() override {
        graphics_init();
        OPTIONS_SET_SIMULATE(kSimulateMmb4l);
    }

    void TearDown() override {
        EXPECT_EQ(kOk, graphics_term());
    }
};

TEST_F(GraphicsTest, SpriteCreate_WithIdZero_Fails) {
    EXPECT_EQ(kGraphicsInvalidSpriteIdZero, graphics_sprite_create(0, 100, 100));
}

TEST_F(GraphicsTest, GetDefaultWindowTitle_GivenNoCurrentFile) {
    char title[STRINGSIZE];
    CurrentFile[0] = '\0';

    OPTIONS_SET_SIMULATE(kSimulateMmb4l);
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("MMBasic - Window 0", title);
    EXPECT_EQ(kOk, graphics_get_default_window_title(1, title, STRINGSIZE));
    EXPECT_STREQ("MMBasic - Window 1", title);

    OPTIONS_SET_SIMULATE(kSimulateCmm2);
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("Colour Maximite 2 - Mode 2", title);

    OPTIONS_SET_SIMULATE(kSimulateMmb4w);
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("MMBasic for Windows - Mode 2", title);

    OPTIONS_SET_SIMULATE(kSimulateGamemite);
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("Game*Mite", title);

    OPTIONS_SET_SIMULATE(kSimulatePicomiteVga);
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("PicoMiteVGA - Mode 2", title);
}

TEST_F(GraphicsTest, GetDefaultWindowTitle_GivenCurrentFile) {
    char title[STRINGSIZE];
    snprintf(CurrentFile, STRINGSIZE, "foo/bar");

    OPTIONS_SET_SIMULATE(kSimulateMmb4l);
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("MMBasic - Window 0: foo/bar", title);
    EXPECT_EQ(kOk, graphics_get_default_window_title(1, title, STRINGSIZE));
    EXPECT_STREQ("MMBasic - Window 1: foo/bar", title);

    OPTIONS_SET_SIMULATE(kSimulateCmm2);
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("Colour Maximite 2 - Mode 2: foo/bar", title);

    OPTIONS_SET_SIMULATE(kSimulateMmb4w);
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("MMBasic for Windows - Mode 2: foo/bar", title);

    OPTIONS_SET_SIMULATE(kSimulateGamemite);
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("Game*Mite: foo/bar", title);

    OPTIONS_SET_SIMULATE(kSimulatePicomiteVga);
    graphics_mode = 2;
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, STRINGSIZE));
    EXPECT_STREQ("PicoMiteVGA - Mode 2: foo/bar", title);

    // Title will exactly fill the buffer.
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, 30));
    EXPECT_STREQ("PicoMiteVGA - Mode 2: foo/bar", title);

    // Title is longer than the buffer.
    EXPECT_EQ(kOk, graphics_get_default_window_title(0, title, 29));
    EXPECT_STREQ("PicoMiteVGA - Mode 2: foo/ba", title);
}

#define EXPECT_SURFACE_TYPE(id, type_, expected) \
    { \
        char out[STRINGSIZE]; \
        graphics_surfaces[id].type = type_; \
        EXPECT_EQ(kOk, graphics_type_as_string(&graphics_surfaces[id], out, STRINGSIZE)); \
        EXPECT_STREQ(expected, out); \
    }

TEST_F(GraphicsTest, TypeAsString_GivenNotSimulatingOther) {
    OPTIONS_SET_SIMULATE(kSimulateMmb4l);

    // Initialise all the surfaces as (tiny) buffers.
    EXPECT_EQ(kOk, graphics_surface_destroy_all());
    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        EXPECT_EQ(kOk, graphics_buffer_create(id, 8, 8));
    }

    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        EXPECT_SURFACE_TYPE(id, kGraphicsNone, "None");
        EXPECT_SURFACE_TYPE(id, kGraphicsWindow, "Window");
        EXPECT_SURFACE_TYPE(id, kGraphicsBuffer, "Buffer");
        EXPECT_SURFACE_TYPE(id, kGraphicsSprite, "Sprite (Active)");
        EXPECT_SURFACE_TYPE(id, kGraphicsInactiveSprite, "Sprite (Inactive)");
    }
}

TEST_F(GraphicsTest, TypeAsString_GivenSimulatingCmm2) {
    OPTIONS_SET_SIMULATE(kSimulateCmm2);

    // Initialise all the surfaces as (tiny) buffers.
    EXPECT_EQ(kOk, graphics_surface_destroy_all());
    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        EXPECT_EQ(kOk, graphics_buffer_create(id, 8, 8));
    }

    // Window with 0 <= id <= 63 is a "Page <id>"
    EXPECT_SURFACE_TYPE(0, kGraphicsWindow, "Page 0");
    EXPECT_SURFACE_TYPE(63, kGraphicsWindow, "Page 63");

    // Window with id >= 64 is a "Window"
    EXPECT_SURFACE_TYPE(64, kGraphicsWindow, "Window");

    // Buffer with 0 <= id <= 63 is a "Page <id>"
    EXPECT_SURFACE_TYPE(0, kGraphicsBuffer, "Page 0");
    EXPECT_SURFACE_TYPE(63, kGraphicsBuffer, "Page 63");

    // Buffer with 64 <= id <= 127 is a "Buffer <id - 63>"
    EXPECT_SURFACE_TYPE(64, kGraphicsBuffer, "Buffer 1");
    EXPECT_SURFACE_TYPE(127, kGraphicsBuffer, "Buffer 64");

    // Buffer with id >= 128 is a "Buffer"
    EXPECT_SURFACE_TYPE(128, kGraphicsBuffer, "Buffer");

    // Inactive Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Inactive)"
    EXPECT_SURFACE_TYPE(128, kGraphicsInactiveSprite, "Sprite #1 (Inactive)");
    EXPECT_SURFACE_TYPE(191, kGraphicsInactiveSprite, "Sprite #64 (Inactive)");

    // Inactive Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsInactiveSprite, "Sprite (Inactive)");

    // Active Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Active)"
    EXPECT_SURFACE_TYPE(128, kGraphicsSprite, "Sprite #1 (Active)");
    EXPECT_SURFACE_TYPE(191, kGraphicsSprite, "Sprite #64 (Active)");

    // Active Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsSprite, "Sprite (Active)");
}

TEST_F(GraphicsTest, TypeAsString_GivenSimulatingMmb4w) {
    OPTIONS_SET_SIMULATE(kSimulateMmb4w);

    // Initialise all the surfaces as (tiny) buffers.
    EXPECT_EQ(kOk, graphics_surface_destroy_all());
    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        EXPECT_EQ(kOk, graphics_buffer_create(id, 8, 8));
    }

    // Window with 0 <= id <= 63 is a "Page <id>"
    EXPECT_SURFACE_TYPE(0, kGraphicsWindow, "Page 0");
    EXPECT_SURFACE_TYPE(63, kGraphicsWindow, "Page 63");

    // Window with id >= 64 is a "Window"
    EXPECT_SURFACE_TYPE(64, kGraphicsWindow, "Window");

    // Buffer with 0 <= id <= 63 is a "Page <id>"
    EXPECT_SURFACE_TYPE(0, kGraphicsBuffer, "Page 0");
    EXPECT_SURFACE_TYPE(63, kGraphicsBuffer, "Page 63");

    // Buffer with 64 <= id <= 127 is a "Buffer <id - 63>"
    EXPECT_SURFACE_TYPE(64, kGraphicsBuffer, "Buffer 1");
    EXPECT_SURFACE_TYPE(127, kGraphicsBuffer, "Buffer 64");

    // Buffer with id >= 128 is a "Buffer"
    EXPECT_SURFACE_TYPE(128, kGraphicsBuffer, "Buffer");

    // Inactive Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Inactive)"
    EXPECT_SURFACE_TYPE(128, kGraphicsInactiveSprite, "Sprite #1 (Inactive)");
    EXPECT_SURFACE_TYPE(191, kGraphicsInactiveSprite, "Sprite #64 (Inactive)");

    // Inactive Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsInactiveSprite, "Sprite (Inactive)");

    // Active Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Active)"
    EXPECT_SURFACE_TYPE(128, kGraphicsSprite, "Sprite #1 (Active)");
    EXPECT_SURFACE_TYPE(191, kGraphicsSprite, "Sprite #64 (Active)");

    // Active Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsSprite, "Sprite (Active)");
}

TEST_F(GraphicsTest, TypeAsString_GivenSimulatingGamemite) {
    OPTIONS_SET_SIMULATE(kSimulateGamemite);

    // Initialise all the surfaces as (tiny) buffers.
    EXPECT_EQ(kOk, graphics_surface_destroy_all());
    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        EXPECT_EQ(kOk, graphics_buffer_create(id, 8, 8));
    }

    // Window with id == 0 is the "Display"
    EXPECT_SURFACE_TYPE(0, kGraphicsWindow, "Display");

    // Buffer with id == 1 is "Buffer N"
    EXPECT_SURFACE_TYPE(1, kGraphicsBuffer, "Buffer N");

    // Buffer with id == 2 is "Buffer F"
    EXPECT_SURFACE_TYPE(2, kGraphicsBuffer, "Buffer F");

    // Buffer with id == 3 is "Buffer L"
    EXPECT_SURFACE_TYPE(3, kGraphicsBuffer, "Buffer L");

    // Window with id >= 1 is a "Window"
    EXPECT_SURFACE_TYPE(1, kGraphicsWindow, "Window");

    // Buffer with 4 <= id <= 63 is a "Buffer"
    EXPECT_SURFACE_TYPE(4, kGraphicsBuffer, "Buffer");
    EXPECT_SURFACE_TYPE(63, kGraphicsBuffer, "Buffer");

    // Buffer with 64 <= id <= 127 is a "Buffer <id - 63>"
    EXPECT_SURFACE_TYPE(64, kGraphicsBuffer, "Buffer 1");
    EXPECT_SURFACE_TYPE(127, kGraphicsBuffer, "Buffer 64");

    // Buffer with id >= 128 is a "Buffer"
    EXPECT_SURFACE_TYPE(128, kGraphicsBuffer, "Buffer");

    // Inactive Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Inactive)"
    EXPECT_SURFACE_TYPE(128, kGraphicsInactiveSprite, "Sprite #1 (Inactive)");
    EXPECT_SURFACE_TYPE(191, kGraphicsInactiveSprite, "Sprite #64 (Inactive)");

    // Inactive Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsInactiveSprite, "Sprite (Inactive)");

    // Active Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Active)"
    EXPECT_SURFACE_TYPE(128, kGraphicsSprite, "Sprite #1 (Active)");
    EXPECT_SURFACE_TYPE(191, kGraphicsSprite, "Sprite #64 (Active)");

    // Active Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsSprite, "Sprite (Active)");
}

TEST_F(GraphicsTest, TypeAsString_GivenSimulatingPicomiteVga) {
    OPTIONS_SET_SIMULATE(kSimulatePicomiteVga);

    // Initialise all the surfaces as (tiny) buffers.
    EXPECT_EQ(kOk, graphics_surface_destroy_all());
    for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
        EXPECT_EQ(kOk, graphics_buffer_create(id, 8, 8));
    }

    // Window with id == 0 is the "Display"
    EXPECT_SURFACE_TYPE(0, kGraphicsWindow, "Display");

    // Buffer with id == 1 is "Buffer N"
    EXPECT_SURFACE_TYPE(1, kGraphicsBuffer, "Buffer N");

    // Buffer with id == 2 is "Buffer F"
    EXPECT_SURFACE_TYPE(2, kGraphicsBuffer, "Buffer F");

    // Buffer with id == 3 is "Buffer L"
    EXPECT_SURFACE_TYPE(3, kGraphicsBuffer, "Buffer L");

    // Window with id >= 1 is a "Window"
    EXPECT_SURFACE_TYPE(1, kGraphicsWindow, "Window");

    // Buffer with 4 <= id <= 63 is a "Buffer"
    EXPECT_SURFACE_TYPE(4, kGraphicsBuffer, "Buffer");
    EXPECT_SURFACE_TYPE(63, kGraphicsBuffer, "Buffer");

    // Buffer with 64 <= id <= 127 is a "Buffer <id - 63>"
    EXPECT_SURFACE_TYPE(64, kGraphicsBuffer, "Buffer 1");
    EXPECT_SURFACE_TYPE(127, kGraphicsBuffer, "Buffer 64");

    // Buffer with id >= 128 is a "Buffer"
    EXPECT_SURFACE_TYPE(128, kGraphicsBuffer, "Buffer");

    // Inactive Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Inactive)"
    EXPECT_SURFACE_TYPE(128, kGraphicsInactiveSprite, "Sprite #1 (Inactive)");
    EXPECT_SURFACE_TYPE(191, kGraphicsInactiveSprite, "Sprite #64 (Inactive)");

    // Inactive Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsInactiveSprite, "Sprite (Inactive)");

    // Active Sprite with 128 <= id <= 191 is a "Sprite <id - 127> (Active)"
    EXPECT_SURFACE_TYPE(128, kGraphicsSprite, "Sprite #1 (Active)");
    EXPECT_SURFACE_TYPE(191, kGraphicsSprite, "Sprite #64 (Active)");

    // Active Sprite with id >= 192 is a "Sprite id (Inactive)"
    EXPECT_SURFACE_TYPE(192, kGraphicsSprite, "Sprite (Active)");
}
