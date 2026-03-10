/*
 * Copyright (c) 2026-2026 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include <SDL.h>

#include "../../common/features.h"
#include "../../common/memory.h"
#include "../../common/gtest/stubs/error_stubs.h"
#include "../../core/Commands.h"
#include "../../core/MMBasic.h"
#include "../../core/tokentbl.h"
#include "../../core/vartbl.h"
#include "../../core/gtest/command_stubs.h"
#define DO_NOT_STUB_FUN_MMINFO
#define DO_NOT_STUB_FUN_HRES
#define DO_NOT_STUB_FUN_VRES
#include "../../core/gtest/function_stubs.h"
#include "../../core/gtest/operation_stubs.h"

// Defined in "main.c"
char *CFunctionFlash;
char *CFunctionLibrary;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Features mmb_features;
Options mmb_options;
ErrorState mmb_normal_error_state;

// Defined in "commands/cmd_read.c"
void cmd_read_clear_cache()  { }

// Defined in "commands/cmd_run.c"
char cmd_run_args[STRINGSIZE];

// Defined in "common/gpio.c"
MmResult gpio_term() { return kOk; }
MmResult gpio_translate_from_pin_gp(uint8_t pin_gp, uint8_t *pin_num) { return kOk; }

// Defined in "common/keyboard.c"
uint64_t keyboard_get_last_ps2_scancode() { return 0; }
MmResult keyboard_key_down(const SDL_Keysym *keysym) { return kError; }
MmResult keyboard_key_up(const SDL_Keysym *keysym) { return kError; }

// Defined in "common/mmgetline.c"
void MMgetline(int fnbr, char *p) {}

// Defined in "common/program.c"
char CurrentFile[STRINGSIZE];

// Defined in "common/prompt.c"
MmResult prompt_getc(int *ch) {
    *ch = -1;
    return kOk;
}

// Defined in "common/system.c"
int system_getpid() { return 0; }

// Defined in "core/Commands.c"
char DimUsed;
int doindex;
struct s_dostack dostack[MAXDOLOOPS];
const char *errorstack[MAXGOSUB];
int forindex;
struct s_forstack forstack[MAXFORLOOPS + 1];
int gosubindex;
const char *gosubstack[MAXGOSUB];

// Defined in "common/gtest/stubs/display_stubs.c"
extern MmResult (*mock_display_get_size)(bool pixel, int *width, int *height);

} // extern "C"

class BaseTest : public ::testing::Test {

protected:

    void SetUp() override {
        vartbl_init_called = false;
        ASSERT_EQ(kOk, memory_init());
        ASSERT_EQ(kOk, InitBasic());
        error_msg[0] = '\0';
    }

    void TearDown() override {
        ASSERT_EQ(kOk, graphics_term());
        ASSERT_EQ(kOk, memory_term());
    }
};

class PlatformParameterisedTest : public BaseTest, public testing::WithParamInterface<OptionsSimulate> {
   protected:
    void SetUp() override {
        BaseTest::SetUp();
        sim = GetParam();
        OPTIONS_SET_SIMULATE(sim);
        SwitchPlatform(mmb_options.simulate);
        if (sim == kSimulateMmb4l) {
            ASSERT_EQ(kOk, graphics_buffer_create(1, 320, 240));
        }
        mock_display_get_size= NULL;
    }

    void TearDown() override {
        mock_display_get_size = NULL;
        BaseTest::TearDown();
    }

    MmSurfaceId DefaultSurface() {
        return mmb_features.graphics_type == kGraphicsTypeCmm2 ? 0 : GRAPHICS_SURFACE_N;
    }

    OptionsSimulate sim;
};

class FunMmInfoHResTest : public PlatformParameterisedTest { };

TEST_P(FunMmInfoHResTest, WithSurface_ReturnsWidthInPixels_OrErrorForSimulatedPlatform) {
    ASSERT_EQ(kOk, graphics_surface_write(DefaultSurface()));

    char args[STRINGSIZE] = "HRES";
    ep = args;
    iret = 9999;
    fun_mminfo();

    if (sim == kSimulateMmb4l) {
        EXPECT_EQ(320, iret);
        EXPECT_STREQ("", error_msg);
    } else {
        EXPECT_EQ(9999, iret);
        EXPECT_STREQ("Unsupported on current device/platform", error_msg);
    }
}

TEST_P(FunMmInfoHResTest, WithNoSurface_ReturnsTerminalWidthInPixels_OrErrorForSimulatedPlatform) {
    ASSERT_EQ(kOk, graphics_surface_write(GRAPHICS_NONE));
    mock_display_get_size = [](bool pixel, int *width, int *height) -> MmResult {
       *width = 640;
       *height = 480;
       return kOk;
    };

    char args[STRINGSIZE] = "HRES";
    ep = args;
    iret = 9999;
    fun_mminfo();

    if (sim == kSimulateMmb4l) {
        EXPECT_EQ(640, iret);
        EXPECT_STREQ("", error_msg);
    } else {
        EXPECT_EQ(9999, iret);
        EXPECT_STREQ("Unsupported on current device/platform", error_msg);
    }
}

TEST_P(FunMmInfoHResTest, WithCharParameter_WithSurface_ReturnsWidthInCharacters_OrErrorForSimulatedPlatform) {
    ASSERT_EQ(kOk, graphics_surface_write(DefaultSurface()));

    char args[STRINGSIZE] = "HRES C";
    ep = args;
    iret = 9999;
    fun_mminfo();

    if (sim == kSimulateMmb4l) {
        EXPECT_EQ(320 / 8, iret);  // font width 8
        EXPECT_STREQ("", error_msg);
    } else {
        EXPECT_EQ(9999, iret);
        EXPECT_STREQ("Unsupported on current device/platform", error_msg);
    }

    strcpy(args, "HRES CHAR");
    ep = args;
    iret = 9999;
    fun_mminfo();

    if (sim == kSimulateMmb4l) {
        EXPECT_EQ(320 / 8, iret);  // font width 8
        EXPECT_STREQ("", error_msg);
    } else {
        EXPECT_EQ(9999, iret);
        EXPECT_STREQ("Unsupported on current device/platform", error_msg);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ,
    FunMmInfoHResTest,
    testing::Range(
        static_cast<OptionsSimulate>(kSimulateUnspecified + 1),
        kSimulateCount
    ),
    [](const testing::TestParamInfo<OptionsSimulate>& info) {
        std::string s = options_simulate_to_string(info.param);
        std::replace_if(s.begin(), s.end(), [](char c) { return !std::isalnum(c); }, '_');
        return s;
    }
);

class FunMmInfoVResTest : public PlatformParameterisedTest { };

TEST_P(FunMmInfoVResTest, WithSurface_ReturnsHeightInPixels_OrErrorForSimulatedPlatform) {
    ASSERT_EQ(kOk, graphics_surface_write(DefaultSurface()));

    char args[STRINGSIZE] = "VRES";
    ep = args;
    iret = 9999;
    fun_mminfo();

    if (sim == kSimulateMmb4l) {
        EXPECT_EQ(240, iret);
        EXPECT_STREQ("", error_msg);
    } else {
        EXPECT_EQ(9999, iret);
        EXPECT_STREQ("Unsupported on current device/platform", error_msg);
    }
}

TEST_P(FunMmInfoVResTest, WithNoSurface_ReturnsTerminalHeightInPixels_OrErrorForSimulatedPlatform) {
    ASSERT_EQ(kOk, graphics_surface_write(GRAPHICS_NONE));
    mock_display_get_size = [](bool pixel, int *width, int *height) -> MmResult {
       *width = 640;
       *height = 480;
       return kOk;
    };

    char args[STRINGSIZE] = "VRES";
    ep = args;
    iret = 9999;
    fun_mminfo();

    if (sim == kSimulateMmb4l) {
        EXPECT_EQ(480, iret);
        EXPECT_STREQ("", error_msg);
    } else {
        EXPECT_EQ(9999, iret);
        EXPECT_STREQ("Unsupported on current device/platform", error_msg);
    }
}

TEST_P(FunMmInfoVResTest, WithCharParameter_WithSurface_ReturnsHeightInCharacters_OrErrorForSimulatedPlatform) {
    ASSERT_EQ(kOk, graphics_surface_write(DefaultSurface()));

    char args[STRINGSIZE] = "VRES C";
    ep = args;
    iret = 9999;
    fun_mminfo();

    if (sim == kSimulateMmb4l) {
        EXPECT_EQ(240 / 12, iret);  // font height 12
        EXPECT_STREQ("", error_msg);
    } else {
        EXPECT_EQ(9999, iret);
        EXPECT_STREQ("Unsupported on current device/platform", error_msg);
    }

    strcpy(args, "VRES CHAR");
    ep = args;
    iret = 9999;
    fun_mminfo();

    if (sim == kSimulateMmb4l) {
        EXPECT_EQ(240 / 12, iret);  // font height 12
        EXPECT_STREQ("", error_msg);
    } else {
        EXPECT_EQ(9999, iret);
        EXPECT_STREQ("Unsupported on current device/platform", error_msg);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ,
    FunMmInfoVResTest,
    testing::Range(
        static_cast<OptionsSimulate>(kSimulateUnspecified + 1),
        kSimulateCount
    ),
    [](const testing::TestParamInfo<OptionsSimulate>& info) {
        std::string s = options_simulate_to_string(info.param);
        std::replace_if(s.begin(), s.end(), [](char c) { return !std::isalnum(c); }, '_');
        return s;
    }
);

class FunHResTest : public PlatformParameterisedTest {
   protected:
    int ExpectedHRes() {
        static const int expected[kSimulateCount] = { 0, 320, 800, 800, 320, 320, 640, 640, 640 };
        return expected[sim];
    }
};

TEST_P(FunHResTest, WithDefaultSurface_ReturnsWidthInPixels) {
    ASSERT_EQ(kOk, graphics_surface_write(DefaultSurface()));

    iret = 9999;
    fun_hres();
    EXPECT_EQ(ExpectedHRes(), iret);
    EXPECT_STREQ("", error_msg);
}

TEST_P(FunHResTest, WithNonDefaultSurface_ReturnsWidthInPixels) {
    ASSERT_EQ(kOk, graphics_buffer_create(64, 30, 40));
    ASSERT_EQ(kOk, graphics_surface_write(64));

    // When we ARE NOT simulating a different MMBasic platform we expect the
    // width of the write surface. When we ARE simulating we expect the
    // width of the default surface.
    const int expected_hres = (sim == kSimulateMmb4l)
            ? 30
            : ExpectedHRes();

    iret = 9999;
    fun_hres();
    EXPECT_EQ(expected_hres, iret);
    EXPECT_STREQ("", error_msg);
}

TEST_P(FunHResTest, WithNoSurface_ReturnsWidthInPixels) {
    mock_display_get_size = [](bool pixel, int *width, int *height) -> MmResult {
       *width = 111;
       *height = 222;
       return kOk;
    };

    // When there is no current surface and we ARE NOT simulating a different
    // MMBasic platform we expect the width of the terminal/display.
    const int expected_hres = (sim == kSimulateMmb4l)
            ? 111
            : ExpectedHRes();

    iret = 9999;
    fun_hres();
    EXPECT_EQ(expected_hres, iret);
    EXPECT_STREQ("", error_msg);
}

INSTANTIATE_TEST_SUITE_P(
    ,
    FunHResTest,
    testing::Range(
        static_cast<OptionsSimulate>(kSimulateUnspecified + 1),
        kSimulateCount
    ),
    [](const testing::TestParamInfo<OptionsSimulate>& info) {
        std::string s = options_simulate_to_string(info.param);
        std::replace_if(s.begin(), s.end(), [](char c) { return !std::isalnum(c); }, '_');
        return s;
    }
);

class FunVResTest : public PlatformParameterisedTest {
   protected:
    int ExpectedVRes() {
        static const int expected[kSimulateCount] = { 0, 240, 600, 600, 240, 320, 480, 480, 480 };
        return expected[sim];
    }
};

TEST_P(FunVResTest, WithDefaultSurface_ReturnsHeightInPixels) {
    ASSERT_EQ(kOk, graphics_surface_write(DefaultSurface()));

    iret = 9999;
    fun_vres();
    EXPECT_EQ(ExpectedVRes(), iret);
    EXPECT_STREQ("", error_msg);
}

TEST_P(FunVResTest, WithNonDefaultSurface_ReturnsHeightInPixels) {
    ASSERT_EQ(kOk, graphics_buffer_create(64, 30, 40));
    ASSERT_EQ(kOk, graphics_surface_write(64));

    // When we ARE NOT simulating a different MMBasic platform we expect the
    // height of the write surface. When we ARE simulating we expect the
    // height of the default surface.
    const int expected_vres = (sim == kSimulateMmb4l)
            ? 40
            : ExpectedVRes();

    iret = 9999;
    fun_vres();
    EXPECT_EQ(expected_vres, iret);
    EXPECT_STREQ("", error_msg);
}

TEST_P(FunVResTest, WithNoSurface_ReturnsHeightInPixels) {
    mock_display_get_size = [](bool pixel, int *width, int *height) -> MmResult {
       *width = 111;
       *height = 222;
       return kOk;
    };

    // When there is no current surface and we ARE NOT simulating a different
    // MMBasic platform we expect the height of the terminal/display.
    const int expected_vres = (sim == kSimulateMmb4l)
            ? 222
            : ExpectedVRes();

    iret = 9999;
    fun_vres();
    EXPECT_EQ(expected_vres, iret);
    EXPECT_STREQ("", error_msg);
}

INSTANTIATE_TEST_SUITE_P(
    ,
    FunVResTest,
    testing::Range(
        static_cast<OptionsSimulate>(kSimulateUnspecified + 1),
        kSimulateCount
    ),
    [](const testing::TestParamInfo<OptionsSimulate>& info) {
        std::string s = options_simulate_to_string(info.param);
        std::replace_if(s.begin(), s.end(), [](char c) { return !std::isalnum(c); }, '_');
        return s;
    }
);