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

static int64_t snes_a_buttons = 0x0;
static int64_t snes_b_buttons = 0x0;

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
            *out = snes_a_buttons;
            break;
        case 2:
            *out = snes_b_buttons;
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

class GpioTest : public ::testing::Test {
   protected:
    void SetUp() override {
        memset(&mmb_options, 0x0, sizeof(Options));
        gpio_init();
    }

    void TearDown() override {}
};

TEST_F(GpioTest, GpioTranslateFromGpPin) {
    uint8_t pin_num;

    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(0, &pin_num));
    EXPECT_EQ(1, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(1, &pin_num));
    EXPECT_EQ(2, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(2, &pin_num));
    EXPECT_EQ(4, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(3, &pin_num));
    EXPECT_EQ(5, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(4, &pin_num));
    EXPECT_EQ(6, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(5, &pin_num));
    EXPECT_EQ(7, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(6, &pin_num));
    EXPECT_EQ(9, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(7, &pin_num));
    EXPECT_EQ(10, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(8, &pin_num));
    EXPECT_EQ(11, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(9, &pin_num));
    EXPECT_EQ(12, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(10, &pin_num));
    EXPECT_EQ(14, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(11, &pin_num));
    EXPECT_EQ(15, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(12, &pin_num));
    EXPECT_EQ(16, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(13, &pin_num));
    EXPECT_EQ(17, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(14, &pin_num));
    EXPECT_EQ(19, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(15, &pin_num));
    EXPECT_EQ(20, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(16, &pin_num));
    EXPECT_EQ(21, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(17, &pin_num));
    EXPECT_EQ(22, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(18, &pin_num));
    EXPECT_EQ(24, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(19, &pin_num));
    EXPECT_EQ(25, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(20, &pin_num));
    EXPECT_EQ(26, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(21, &pin_num));
    EXPECT_EQ(27, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(22, &pin_num));
    EXPECT_EQ(29, pin_num);
    EXPECT_EQ(kGpioInvalidPin, gpio_translate_from_gp_pin(23, &pin_num));
    EXPECT_EQ(kGpioInvalidPin, gpio_translate_from_gp_pin(24, &pin_num));
    EXPECT_EQ(kGpioInvalidPin, gpio_translate_from_gp_pin(25, &pin_num));
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(26, &pin_num));
    EXPECT_EQ(31, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(27, &pin_num));
    EXPECT_EQ(32, pin_num);
    EXPECT_EQ(kOk, gpio_translate_from_gp_pin(28, &pin_num));
    EXPECT_EQ(34, pin_num);
    EXPECT_EQ(kGpioInvalidPin, gpio_translate_from_gp_pin(29, &pin_num));
}

TEST_F(GpioTest, GpioGetSetPinValue) {
    for (uint8_t pin_num = 0; pin_num <= GPIO_MAX_PIN_NUM; ++pin_num) {
        if (gpio_is_valid_pin_num(pin_num)) {
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
}

TEST_F(GpioTest, GpioPulsePin) {
    for (uint8_t pin_num = 0; pin_num <= GPIO_MAX_PIN_NUM; ++pin_num) {
        if (gpio_is_valid_pin_num(pin_num)) {
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
        } else {
            EXPECT_EQ(kGpioInvalidPin, gpio_pulse_pin(pin_num, 10));
        }
    }

    EXPECT_EQ(kOk, gpio_pulse_pin(2, 0));
    EXPECT_EQ(kGpioInvalidPulseWidth, gpio_pulse_pin(2, -1));
}

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
    : public GpioTest,
      public ::testing::WithParamInterface<std::tuple<std::string, GamepadButton, uint16_t>> {};

TEST_P(SnesControllerSimulationTest, ControllerA) {
    mmb_options.simulate = kSimulatePicoMiteVga;

    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_SNES_A_LATCH, kGpioPinDOut));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_SNES_A_CLOCK, kGpioPinDOut));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_SNES_A_DATA, kGpioPinDIn));

    snes_a_buttons = std::get<1>(GetParam());
    EXPECT_EQ(std::get<2>(GetParam()),
              read_snes(GPIO_SNES_A_LATCH, GPIO_SNES_A_CLOCK, GPIO_SNES_A_DATA));

    snes_a_buttons = 0x0;
    EXPECT_EQ(0b1111111111111111,
              read_snes(GPIO_SNES_A_LATCH, GPIO_SNES_A_CLOCK, GPIO_SNES_A_DATA));
}

TEST_P(SnesControllerSimulationTest, ControllerB) {
    mmb_options.simulate = kSimulatePicoMiteVga;

    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_SNES_B_LATCH, kGpioPinDOut));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_SNES_B_CLOCK, kGpioPinDOut));
    EXPECT_EQ(kOk, gpio_configure_pin(GPIO_SNES_B_DATA, kGpioPinDIn));

    snes_b_buttons = std::get<1>(GetParam());
    EXPECT_EQ(std::get<2>(GetParam()),
              read_snes(GPIO_SNES_B_LATCH, GPIO_SNES_B_CLOCK, GPIO_SNES_B_DATA));

    snes_b_buttons = 0x0;
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
    [](const testing::TestParamInfo<std::tuple<std::string, GamepadButton, uint16_t>> &info) {
        return std::get<0>(info.param);
    });
