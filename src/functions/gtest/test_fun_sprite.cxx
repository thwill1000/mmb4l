/*
 * Copyright (c) 2024-2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

#if !defined(ENABLE_GTEST_EXTRAS)
#define ENABLE_GTEST_EXTRAS
#endif

extern "C" {

#include <SDL.h>

#include "../../common/bitset.h"
#include "../../common/features.h"
#include "../../common/options.h"
#include "../../common/mmresult.h"
#include "../../common/sprite.h"
#include "../../common/utility.h"
#include "../../common/gtest/test_helper.h"
#include "../../common/gtest/stubs/error_stubs.h"
#include "../../core/Commands.h"
#include "../../core/MMBasic.h"
#include "../../core/tokentbl.h"
#include "../../core/vartbl.h"
#include "../../core/gtest/command_stubs.h"
#define DO_NOT_STUB_FUN_SPRITE
#include "../../core/gtest/function_stubs.h"
#include "../../core/gtest/operation_stubs.h"

// Defined in "main.c"
char *CFunctionFlash;
char *CFunctionLibrary;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Features mmb_features;
Options mmb_options;
ErrorState mmb_normal_error_state;
int WatchdogSet;
int IgnorePIN;

void CheckAbort(void) { }
int MMgetchar(void) { return 0; }
void MMgetline(int fnbr, char *p) {}

// Defined in "commands/cmd_read.c"
void cmd_read_clear_cache()  { }

// Defined in "common/gpio.c"
void gpio_term() { }
MmResult gpio_translate_from_pin_gp(uint8_t pin_gp, uint8_t *pin_num) { return kOk; }

// Defined in "common/keyboard.c"
MmResult keyboard_key_down(const SDL_Keysym *keysym) { return kError; }
MmResult keyboard_key_up(const SDL_Keysym *keysym) { return kError; }

// Defined in "common/program.c"
char CurrentFile[STRINGSIZE];

// Defined in "core/Commands.c"
char DimUsed;
int doindex;
struct s_dostack dostack[MAXDOLOOPS];
const char *errorstack[MAXGOSUB];
int forindex;
struct s_forstack forstack[MAXFORLOOPS + 1];
int gosubindex;
const char *gosubstack[MAXGOSUB];
int TraceBuffIndex;
const char *TraceBuff[TRACE_BUFF_SIZE];
int TraceOn;

}

class FunSpriteTest : public ::testing::Test {

protected:

    void SetUp() override {
        vartbl_init_called = false;
        InitBasic();
        ClearRuntime();
        error_msg[0] = '\0';
        ClearProgMemory();
        graphics_init();
    }

    void TearDown() override {
        // Some of these tests set the type to kGraphicsSprite naively rather than properly
        // showing the sprite, we need to reverse this or destroying the sprites will hit an
        // assertion.
        for (MmSurfaceId id = 0; id <= GRAPHICS_MAX_ID; ++id) {
            MmSurface *surface = &graphics_surfaces[id];
            if (surface->type == kGraphicsSprite) surface->type = kGraphicsInactiveSprite;
        }
        graphics_term();
    }

    void ClearProgMemory() {
        clear_prog_memory();
    }

    void TokeniseAndAppend(const char* untokenised) {
        tokenise_and_append(untokenised);
        EXPECT_STREQ("", error_msg);
    }

};

TEST_F(FunSpriteTest, SpriteCollision_GivenSpriteIdEqualsMinus2_Fails) {
    char args[STRINGSIZE];
    sprintf(args, "C, %s2", tokentbl_encoded("-")); // "C, -2"
    ep = args;
    iret = 9999;

    fun_sprite();

    EXPECT_EQ(0, iret);
    EXPECT_STREQ("\% is invalid (valid is \% to \%)", error_msg);
}

TEST_F(FunSpriteTest, SpriteCollision_GivenSpriteIdEquals256_Fails) {
    const char *args = "C, 256";
    ep = args;
    iret = 9999;

    fun_sprite();

    EXPECT_EQ(0, iret);
    EXPECT_STREQ("\% is invalid (valid is \% to \%)", error_msg);
}

TEST_F(FunSpriteTest, SpriteCollision_GivenSpriteIdEqualsMinus1_AndSimulatingClassicMmBasic_Fails) {
    OPTIONS_SET_SIMULATE(kSimulateCmm2);

    char args[STRINGSIZE];
    sprintf(args, "C, %s1", tokentbl_encoded("-")); // "C, -1"
    ep = args;
    iret = 9999;

    fun_sprite();

    EXPECT_EQ(0, iret);
    EXPECT_STREQ("\% is invalid (valid is \% to \%)", error_msg);
}

TEST_F(FunSpriteTest, SpriteCollision_GivenSpriteIdEquals65_AndSimulatingClassicMmBasic_Fails) {
    OPTIONS_SET_SIMULATE(kSimulateCmm2);

    const char *args = "C, 65";
    ep = args;
    iret = 9999;

    fun_sprite();

    EXPECT_EQ(0, iret);
    EXPECT_STREQ("\% is invalid (valid is \% to \%)", error_msg);
}

TEST_F(FunSpriteTest, SpriteCollision_GivenNotASprite_ReturnsZero) {
    graphics_surfaces[0].type = kGraphicsBuffer;
    const char *args = "C, 0";
    ep = args;
    iret = 9999;

    fun_sprite();

    EXPECT_EQ(0, iret);
    EXPECT_STREQ("", error_msg);
}

TEST_F(FunSpriteTest, SpriteCollision_GivenNotASprite_AndSimulatingClassicMmBasic_ReturnsZero) {
    OPTIONS_SET_SIMULATE(kSimulateCmm2);

    graphics_surfaces[CMM2_SPRITE_BASE + 1].type = kGraphicsBuffer;
    const char *args = "C, 1";
    ep = args;
    iret = 9999;

    fun_sprite();

    EXPECT_EQ(0, iret);
    EXPECT_STREQ("", error_msg);
}

TEST_F(FunSpriteTest, SpriteCollision_GivenSprite1CollidedWithSprite2_ReturnsExpectedCollisions) {
    (void) graphics_sprite_create(1, 10, 10);
    MmSurface *sprite1 = &graphics_surfaces[1];
    sprite1->type = kGraphicsSprite;

    (void) graphics_sprite_create(2, 10, 10);
    MmSurface *sprite2 = &graphics_surfaces[2];
    sprite2->type = kGraphicsSprite;

    bitset_set(sprite1->sprite_collisions, 2);

    { // 1st collision is with sprite 2.
        const char *args = "C, 1, 1";
        ep = args;
        iret = 9999;

        fun_sprite();

        EXPECT_EQ(2, iret);
        EXPECT_STREQ("", error_msg);
    }

    { // There is no 2nd collision.
        const char *args = "C, 1, 2";
        ep = args;
        iret = 9999;

        fun_sprite();

        EXPECT_EQ(0, iret);
        EXPECT_STREQ("", error_msg);
    }
}

TEST_F(FunSpriteTest, SpriteCollision_GivenSprite1CollidedWithSprite2_AndSimulatingClassicMmBasic_ReturnsExpectedCollisions) {
    OPTIONS_SET_SIMULATE(kSimulateCmm2);

    (void) graphics_sprite_create(CMM2_SPRITE_BASE + 1, 10, 10);
    MmSurface *sprite1 = &graphics_surfaces[CMM2_SPRITE_BASE + 1];
    sprite1->type = kGraphicsSprite;

    (void) graphics_sprite_create(CMM2_SPRITE_BASE + 2, 10, 10);
    MmSurface *sprite2 = &graphics_surfaces[CMM2_SPRITE_BASE + 2];
    sprite2->type = kGraphicsSprite;

    bitset_set(sprite1->sprite_collisions, CMM2_SPRITE_BASE + 2);

    { // 1st collision is with sprite 2.
        const char *args = "C, 1, 1";
        ep = args;
        iret = 9999;

        fun_sprite();

        EXPECT_EQ(2, iret);
        EXPECT_STREQ("", error_msg);
    }

    { // There is no 2nd collision.
        const char *args = "C, 1, 2";
        ep = args;
        iret = 9999;

        fun_sprite();

        EXPECT_EQ(0, iret);
        EXPECT_STREQ("", error_msg);
    }
}

TEST_F(FunSpriteTest, SpriteCollision_GivenCollisionWithEdge_ReturnsExpectedCollisions) {
    (void) graphics_sprite_create(1, 10, 10);
    MmSurface *sprite1 = &graphics_surfaces[1];
    sprite1->type = kGraphicsSprite;

    sprite1->edge_collisions = kSpriteEdgeLeft | kSpriteEdgeBottom;

    { // 1st collision is with sprite edge.
        const char *args = "C, 1, 1";
        ep = args;
        iret = 9999;

        fun_sprite();

        EXPECT_EQ(0xFFF1 | 0xFFF8, iret);
        EXPECT_STREQ("", error_msg);
    }

    { // There is no 2nd collision.
        const char *args = "C, 1, 2";
        ep = args;
        iret = 9999;

        fun_sprite();

        EXPECT_EQ(0, iret);
        EXPECT_STREQ("", error_msg);
    }
}

TEST_F(FunSpriteTest, SpriteCollision_GivenCollisionWithEdge_AndSimulatingClassicMmBasic_ReturnsExpectedCollisions) {
    OPTIONS_SET_SIMULATE(kSimulateCmm2);

    (void) graphics_sprite_create(CMM2_SPRITE_BASE + 1, 10, 10);
    MmSurface *sprite1 = &graphics_surfaces[CMM2_SPRITE_BASE + 1];
    sprite1->type = kGraphicsSprite;

    sprite1->edge_collisions = kSpriteEdgeLeft | kSpriteEdgeBottom;

    { // 1st collision is with sprite edge.
        const char *args = "C, 1, 1";
        ep = args;
        iret = 9999;

        fun_sprite();

        EXPECT_EQ(0xF1 | 0xF8, iret);
        EXPECT_STREQ("", error_msg);
    }

    { // There is no 2nd collision.
        const char *args = "C, 1, 2";
        ep = args;
        iret = 9999;

        fun_sprite();

        EXPECT_EQ(0, iret);
        EXPECT_STREQ("", error_msg);
    }
}
