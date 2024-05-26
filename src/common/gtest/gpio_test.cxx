/*
 * Copyright (c) 2021-2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include <SDL.h>
#include <stdint.h>

#include "../gpio.h"
#include "../mmresult.h"

static int64_t gamepad_1_buttons = 0x0;
static int64_t gamepad_2_buttons = 0x0;

// Defined in "audio.c"
const char *audio_last_error() { return NULL; }

// Defined in "events.c"
MmResult events_init() { return kOk; }
const char *events_last_error() { return NULL; }

// Defined in "gamepad.c"
const char *gamepad_last_error() { return NULL; }
MmResult gamepad_open(MmGamepadId id) { return kOk; }
MmResult gamepad_close(MmGamepadId id) { return kOk; }

MmResult gamepad_read_buttons(MmGamepadId id, int64_t *out) {
    switch (id) {
        case 1:
            *out = gamepad_1_buttons;
            break;
        case 2:
            *out = gamepad_2_buttons;
            break;
        default:
            break;
    }
    return kOk;
}

// Defined in "graphics.c"
const char *graphics_last_error() { return NULL; }

// Defined in "options.c"
Options mmb_options;
}

class GpioPinTest : public ::testing::TestWithParam<std::tuple<uint8_t, uint8_t, MmResult>> {
   protected:
    void SetUp() override {
        memset(&mmb_options, 0x0, sizeof(Options));
        gpio_init();
    }

    void TearDown() override {
        gpio_term();
    }
};

TEST_P(GpioPinTest, FromPinNum) {
    uint8_t pin_gp = 0;
    if (std::get<0>(GetParam()) != 0xFF) {
        EXPECT_EQ(std::get<2>(GetParam()),
                  gpio_translate_from_pin_num(std::get<0>(GetParam()), &pin_gp));
        EXPECT_EQ(std::get<1>(GetParam()), pin_gp);
    }
}

TEST_P(GpioPinTest, FromPinGp) {
    uint8_t pin_num = 0;
    if (std::get<1>(GetParam()) != 0xFF) {
        EXPECT_EQ(std::get<2>(GetParam()),
                  gpio_translate_from_pin_gp(std::get<1>(GetParam()), &pin_num));
        EXPECT_EQ(std::get<0>(GetParam()), pin_num);
    }
}

TEST_P(GpioPinTest, GetSetPinValue) {
    uint8_t pin_num = std::get<0>(GetParam());
    if (pin_num != 0xFF && std::get<1>(GetParam()) != 0xFF) {
        uint8_t value = 0xFF;
        EXPECT_EQ(kOk, gpio_get_pin_value(pin_num, &value));
        EXPECT_EQ(0, value);
        EXPECT_EQ(kOk, gpio_configure_pin(pin_num, kGpioPinDOut));
        EXPECT_EQ(kOk, gpio_set_pin_value(pin_num, 1));
        EXPECT_EQ(kOk, gpio_get_pin_value(pin_num, &value));
        EXPECT_EQ(1, value);
    } else {
        uint8_t value = 0xFF;
        EXPECT_EQ(kGpioInvalidPin, gpio_get_pin_value(pin_num, &value));
        EXPECT_EQ(kGpioInvalidPin, gpio_set_pin_value(pin_num, 1));
    }
}

TEST_P(GpioPinTest, PulsePin) {
    uint8_t pin_num = std::get<0>(GetParam());
    if (pin_num != 0xFF && std::get<1>(GetParam()) != 0xFF) {
        EXPECT_EQ(kOk, gpio_configure_pin(pin_num, kGpioPinDOut));

        // Not a lot we can actually test here since gpio_pulse_pin() should leave pin in
        // its original state.
        uint8_t value = 0xFF;
        EXPECT_EQ(kOk, gpio_set_pin_value(pin_num, 0));
        EXPECT_EQ(kOk, gpio_pulse_pin(pin_num, 5));
        EXPECT_EQ(kOk, gpio_get_pin_value(pin_num, &value));
        EXPECT_EQ(0, value);

        EXPECT_EQ(kOk, gpio_set_pin_value(pin_num, 1));
        EXPECT_EQ(kOk, gpio_pulse_pin(pin_num, 5));
        EXPECT_EQ(kOk, gpio_get_pin_value(pin_num, &value));
        EXPECT_EQ(1, value);

        // Zero length pulse.
        EXPECT_EQ(kOk, gpio_pulse_pin(pin_num, 0));

        // Invalid length pulse.
        EXPECT_EQ(kGpioInvalidPulseWidth, gpio_pulse_pin(pin_num, -1));
    } else {
        EXPECT_EQ(kGpioInvalidPin, gpio_pulse_pin(pin_num, 10));
    }
}

// Values are: (pin_num, pin_gp, translation result)
INSTANTIATE_TEST_SUITE_P(
    , GpioPinTest,
    ::testing::Values(
        std::make_tuple(1, 0, kOk), std::make_tuple(2, 1, kOk),
        std::make_tuple(3, 0xFF, kGpioInvalidPin), std::make_tuple(4, 2, kOk),
        std::make_tuple(5, 3, kOk), std::make_tuple(6, 4, kOk), std::make_tuple(7, 5, kOk),
        std::make_tuple(8, 0xFF, kGpioInvalidPin), std::make_tuple(9, 6, kOk),
        std::make_tuple(10, 7, kOk), std::make_tuple(11, 8, kOk), std::make_tuple(12, 9, kOk),
        std::make_tuple(13, 0xFF, kGpioInvalidPin), std::make_tuple(14, 10, kOk),
        std::make_tuple(15, 11, kOk), std::make_tuple(16, 12, kOk), std::make_tuple(17, 13, kOk),
        std::make_tuple(18, 0xFF, kGpioInvalidPin), std::make_tuple(19, 14, kOk),
        std::make_tuple(20, 15, kOk), std::make_tuple(21, 16, kOk), std::make_tuple(22, 17, kOk),
        std::make_tuple(23, 0xFF, kGpioInvalidPin), std::make_tuple(24, 18, kOk),
        std::make_tuple(25, 19, kOk), std::make_tuple(26, 20, kOk), std::make_tuple(27, 21, kOk),
        std::make_tuple(28, 0xFF, kGpioInvalidPin), std::make_tuple(29, 22, kOk),
        std::make_tuple(30, 0xFF, kGpioInvalidPin), std::make_tuple(31, 26, kOk),
        std::make_tuple(32, 27, kOk), std::make_tuple(33, 0xFF, kGpioInvalidPin),
        std::make_tuple(34, 28, kOk), std::make_tuple(35, 0xFF, kGpioInvalidPin),
        std::make_tuple(36, 0xFF, kGpioInvalidPin), std::make_tuple(37, 0xFF, kGpioInvalidPin),
        std::make_tuple(38, 0xFF, kGpioInvalidPin), std::make_tuple(39, 0xFF, kGpioInvalidPin),
        std::make_tuple(40, 0xFF, kGpioInvalidPin), std::make_tuple(41, 23, kOk),
        std::make_tuple(42, 24, kOk), std::make_tuple(43, 25, kOk),
        std::make_tuple(0xFF, 29, kGpioInvalidPin)),
    [](const testing::TestParamInfo<GpioPinTest::ParamType> &info) {
        char buf[32];
        if (std::get<0>(info.param) == 0xFF) {
            sprintf(buf, "Gp_%d", std::get<1>(info.param));
        } else if (std::get<1>(info.param) == 0xFF) {
            sprintf(buf, "Pin_%d", std::get<0>(info.param));
        } else if (std::get<1>(info.param) != 0xFF) {
            sprintf(buf, "Pin_%d_Gp_%d", std::get<0>(info.param), std::get<1>(info.param));
        }
        return std::string(buf);
    });

static uint16_t read_snes(uint8_t latch, uint8_t clock, uint8_t data) {
    const float PULSE_DURATION = 0.012;  // 12uS
    EXPECT_EQ(kOk, gpio_pulse_pin(latch, PULSE_DURATION));
    uint16_t bits = 0x0;
    for (size_t i = 0; i < 16; ++i) {
        uint8_t value = 0;
        EXPECT_EQ(kOk, gpio_get_pin_value(data, &value));
        bits |= (value << i);
        EXPECT_EQ(kOk, gpio_pulse_pin(clock, PULSE_DURATION));
    }

    // Read another 8 bits for good measure, should all be 1.
    for (size_t i = 0; i < 8; ++i) {
        uint8_t value = 0;
        EXPECT_EQ(kOk, gpio_get_pin_value(data, &value));
        EXPECT_EQ(1, value);
        EXPECT_EQ(kOk, gpio_pulse_pin(clock, PULSE_DURATION));
    }

    return bits;
}

class SnesControllerSimulationTest
    : public ::testing::TestWithParam<std::tuple<std::string, GamepadButton, uint16_t>> {
   protected:
    void SetUp() override {
        memset(&mmb_options, 0x0, sizeof(Options));
        gpio_init();
    }

    void TearDown() override {
        gpio_term();
    }
};

TEST_P(SnesControllerSimulationTest, ControllerA) {
    mmb_options.simulate = kSimulatePicoMiteVga;

    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_SNES_A_LATCH, kGpioPinDOut));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_SNES_A_CLOCK, kGpioPinDOut));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_SNES_A_DATA, kGpioPinDIn));

    gamepad_1_buttons = std::get<1>(GetParam());
    EXPECT_EQ(std::get<2>(GetParam()),
              read_snes(GPIO_SNES_A_LATCH, GPIO_SNES_A_CLOCK, GPIO_SNES_A_DATA));

    gamepad_1_buttons = 0x0;
    EXPECT_EQ(0b1111111111111111,
              read_snes(GPIO_SNES_A_LATCH, GPIO_SNES_A_CLOCK, GPIO_SNES_A_DATA));
}

TEST_P(SnesControllerSimulationTest, ControllerB) {
    mmb_options.simulate = kSimulatePicoMiteVga;

    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_SNES_B_LATCH, kGpioPinDOut));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_SNES_B_CLOCK, kGpioPinDOut));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_SNES_B_DATA, kGpioPinDIn));

    gamepad_2_buttons = std::get<1>(GetParam());
    EXPECT_EQ(std::get<2>(GetParam()),
              read_snes(GPIO_SNES_B_LATCH, GPIO_SNES_B_CLOCK, GPIO_SNES_B_DATA));

    gamepad_2_buttons = 0x0;
    EXPECT_EQ(0b1111111111111111,
              read_snes(GPIO_SNES_B_LATCH, GPIO_SNES_B_CLOCK, GPIO_SNES_B_DATA));
}

INSTANTIATE_TEST_SUITE_P(
    , SnesControllerSimulationTest,
    ::testing::Values(std::make_tuple("No_buttons", (GamepadButton)0x0, 0b1111111111111111),
                      std::make_tuple("Button_B", kButtonB, 0b1111111111111110),
                      std::make_tuple("Button_Y", kButtonY, 0b1111111111111101),
                      std::make_tuple("Button_Select", kButtonSelect, 0b1111111111111011),
                      std::make_tuple("Button_Start", kButtonStart, 0b1111111111110111),
                      std::make_tuple("Button_Up", kButtonUp, 0b1111111111101111),
                      std::make_tuple("Button_Down", kButtonDown, 0b1111111111011111),
                      std::make_tuple("Button_Left", kButtonLeft, 0b1111111110111111),
                      std::make_tuple("Button_Right", kButtonRight, 0b1111111101111111),
                      std::make_tuple("Button_A", kButtonA, 0b1111111011111111),
                      std::make_tuple("Button_X", kButtonX, 0b1111110111111111),
                      std::make_tuple("Button_L", kButtonL, 0b1111101111111111),
                      std::make_tuple("Button_R", kButtonR, 0b1111011111111111),
                      std::make_tuple("Multiple_buttons",
                                      (GamepadButton)(kButtonA | kButtonUp | kButtonLeft),
                                      0b1111111010101111)),
    [](const testing::TestParamInfo<SnesControllerSimulationTest::ParamType> &info) {
        return std::get<0>(info.param);
    });

static uint16_t read_gamemite() {
    uint8_t down = 0x0;
    EXPECT_EQ(kOk, gpio_get_pin_value(GPIO_GP8, &down));
    uint8_t left = 0x0;
    EXPECT_EQ(kOk, gpio_get_pin_value(GPIO_GP9, &left));
    uint8_t up = 0x0;
    EXPECT_EQ(kOk, gpio_get_pin_value(GPIO_GP10, &up));
    uint8_t right = 0x0;
    EXPECT_EQ(kOk, gpio_get_pin_value(GPIO_GP11, &right));
    uint8_t select = 0x0;
    EXPECT_EQ(kOk, gpio_get_pin_value(GPIO_GP12, &select));
    uint8_t start = 0x0;
    EXPECT_EQ(kOk, gpio_get_pin_value(GPIO_GP13, &start));
    uint8_t b = 0x0;
    EXPECT_EQ(kOk, gpio_get_pin_value(GPIO_GP14, &b));
    uint8_t a = 0x0;
    EXPECT_EQ(kOk, gpio_get_pin_value(GPIO_GP15, &a));

    return down | (left << 1) | (up << 2) | (right << 3) | (select << 4) | (start << 5) | (b << 6) |
           (a << 7);
}

class GameMiteControllerSimulationTest
    : public ::testing::TestWithParam<std::tuple<std::string, GamepadButton, uint16_t>> {
   protected:
    void SetUp() override {
        memset(&mmb_options, 0x0, sizeof(Options));
        gpio_init();
    }

    void TearDown() override {
        gpio_term();
    }
};

TEST_P(GameMiteControllerSimulationTest, ControllerA) {
    mmb_options.simulate = kSimulateGameMite;

    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_GP8, kGpioPinDIn));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_GP9, kGpioPinDIn));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_GP10, kGpioPinDIn));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_GP11, kGpioPinDIn));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_GP12, kGpioPinDIn));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_GP13, kGpioPinDIn));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_GP14, kGpioPinDIn));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_GP15, kGpioPinDIn));

    gamepad_1_buttons = std::get<1>(GetParam());
    EXPECT_EQ(std::get<2>(GetParam()), read_gamemite());

    gamepad_1_buttons = 0x0;
    EXPECT_EQ(0b11111111, read_gamemite());
}

INSTANTIATE_TEST_SUITE_P(
    , GameMiteControllerSimulationTest,
    ::testing::Values(std::make_tuple("No_buttons", (GamepadButton)0x0, 0b11111111),
                      std::make_tuple("Button_B", kButtonB, 0b10111111),
                      std::make_tuple("Button_Y", kButtonY, 0b10111111),
                      std::make_tuple("Button_Select", kButtonSelect, 0b11101111),
                      std::make_tuple("Button_Start", kButtonStart, 0b11011111),
                      std::make_tuple("Button_Up", kButtonUp, 0b11111011),
                      std::make_tuple("Button_Down", kButtonDown, 0b11111110),
                      std::make_tuple("Button_Left", kButtonLeft, 0b11111101),
                      std::make_tuple("Button_Right", kButtonRight, 0b11110111),
                      std::make_tuple("Button_A", kButtonA, 0b01111111),
                      std::make_tuple("Button_X", kButtonX, 0b01111111),
                      std::make_tuple("Button_L", kButtonL, 0b11111111),
                      std::make_tuple("Button_R", kButtonR, 0b11111111),
                      std::make_tuple("Multiple_buttons",
                                      (GamepadButton)(kButtonA | kButtonUp | kButtonLeft),
                                      0b01111001)),
    [](const testing::TestParamInfo<GameMiteControllerSimulationTest::ParamType> &info) {
        return std::get<0>(info.param);
    });
