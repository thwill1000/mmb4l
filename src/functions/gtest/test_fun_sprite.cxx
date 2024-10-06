/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include <SDL.h>

#include "../../Hardware_Includes.h"
#include "../../common/bitset.h"
#include "../../common/mmresult.h"
#include "../../common/utility.h"
#include "../../common/gtest/test_helper.h"
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
Options mmb_options;
ErrorState mmb_normal_error_state;
int WatchdogSet;
int IgnorePIN;

void CheckAbort(void) { }
int MMgetchar(void) { return 0; }
void MMgetline(int fnbr, char *p) {}

// Defined in "commands/cmd_read.c"
void cmd_read_clear_cache()  { }

// Defined in "common/console.c"
int console_kbhit(void) { return 0; }
char console_putc(char c) { return c; }
void console_puts(const char *s) {}
void console_set_title(const char *title) {}
size_t console_write(const char *buf, size_t sz) { return 0; }

// Defined in "common/gpio.c"
void gpio_term() { }
MmResult gpio_translate_from_pin_gp(uint8_t pin_gp, uint8_t *pin_num) { return kOk; }

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
    sprintf(args, "C, %c2", tokentbl_get("-")); // "C, -2"
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
    mmb_options.simulate = kSimulateCmm2;

    char args[STRINGSIZE];
    sprintf(args, "C, %c1", tokentbl_get("-")); // "C, -1"
    ep = args;
    iret = 9999;

    fun_sprite();

    EXPECT_EQ(0, iret);
    EXPECT_STREQ("\% is invalid (valid is \% to \%)", error_msg);
}

TEST_F(FunSpriteTest, SpriteCollision_GivenSpriteIdEquals65_AndSimulatingClassicMmBasic_Fails) {
    mmb_options.simulate = kSimulateCmm2;

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
    mmb_options.simulate = kSimulateCmm2;

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
    mmb_options.simulate = kSimulateCmm2;

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
