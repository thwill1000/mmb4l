/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gmock/gmock.h>  // Needed for EXPECT_THAT.
#include <gtest/gtest.h>

extern "C" {

#include <SDL.h>

#include "stubs/sdl2_stubs.h"
#include "../error.h"
#include "../gamepad.h"
#include "../gamepad_private.h"
#include "../graphics.h"
#include "../interrupt.h"

// Defined in "common/audio.c"
const char *audio_last_error() { return ""; }

// Defined in "common/graphics.c"
MmSurface graphics_surfaces[GRAPHICS_MAX_SURFACES] = { 0 };

MmSurfaceId graphics_find_window(uint32_t window_id) { return 0; }
const char *graphics_last_error() { return ""; }
MmResult graphics_surface_destroy(MmSurface *surface) { return kOk; }

// Defined in "common/keyboard.c"
MmResult keyboard_key_down(const SDL_Keysym *keysym) { return kError; }
MmResult keyboard_key_up(const SDL_Keysym *keysym) { return kError; }

// Defined in "core/MMBasic.c"
const char *CurrentLinePtr = NULL;

} // extern "C"

SDL_Joystick joystick[4];
SDL_GameController game_controller[4];

class GamepadTest : public ::testing::Test {
   protected:
    void SetUp() override {
        gamepad_init();

        for (uint8_t ii = 0; ii < 4; ++ii) {
            joystick[ii].id = 10 + 2 * ii;
        }
        mock_SDL_JoystickOpen = [](int device_index) -> SDL_Joystick* {
            return &joystick[device_index];
        };
        mock_SDL_GameControllerOpen = [](int joystick_index) -> SDL_GameController* {
            return &game_controller[joystick_index];
        };
        mock_SDL_JoystickInstanceID = [](SDL_Joystick *joystick) -> SDL_JoystickID {
            return joystick->id;
        };
    }

    void TearDown() override { gamepad_term(); }
};

TEST_F(GamepadTest, Open_Succeeds_GivenGamepadClosed) {
    MmResult result = gamepad_open(1, NULL, 0);
    EXPECT_EQ(kOk, result) << mmresult_to_string(result);

    GamepadDevice *gamepad = &gamepad_devices[1];
    EXPECT_EQ(10, gamepad->sdlId);
    EXPECT_EQ(&joystick[0], gamepad->joystick);
    EXPECT_EQ(&game_controller[0], gamepad->controller);
    int64_t out = 0x0;
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(0, out);
    EXPECT_EQ(kOk, gamepad_read_left_x(1, &out));
    EXPECT_EQ(0, out);
    EXPECT_EQ(kOk, gamepad_read_left_y(1, &out));
    EXPECT_EQ(0, out);
    EXPECT_EQ(kOk, gamepad_read_right_x(1, &out));
    EXPECT_EQ(0, out);
    EXPECT_EQ(kOk, gamepad_read_right_y(1, &out));
    EXPECT_EQ(0, out);
    EXPECT_EQ(kOk, gamepad_read_left_analog_button(1, &out));
    EXPECT_EQ(0, out);
    EXPECT_EQ(kOk, gamepad_read_right_analog_button(1, &out));
    EXPECT_EQ(0, out);
}

TEST_F(GamepadTest, Open_ReturnsError_GivenInvalidGamepadId) {
    EXPECT_EQ(kGamepadInvalidId, gamepad_open(0, NULL, 0));
    EXPECT_EQ(kGamepadInvalidId, gamepad_open(5, NULL, 0));
}

TEST_F(GamepadTest, Close_Succeeds_GivenSomeButtonsDown) {
    EXPECT_EQ(kOk, gamepad_open(1, NULL, 0));;
    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_A));
    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_START));
    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_DPAD_RIGHT));

    MmResult result = gamepad_close(1);
    EXPECT_EQ(kOk, result) << mmresult_to_string(result);

    GamepadDevice *gamepad = &gamepad_devices[1];
    EXPECT_EQ(-1, gamepad->sdlId);
    EXPECT_EQ(NULL, gamepad->joystick);
    EXPECT_EQ(NULL, gamepad->controller);
    EXPECT_EQ(0, gamepad->buttons);
    EXPECT_EQ(0, gamepad->left_x);
    EXPECT_EQ(0, gamepad->left_y);
    EXPECT_EQ(0, gamepad->right_x);
    EXPECT_EQ(0, gamepad->right_y);
    EXPECT_EQ(0, gamepad->left_analog_button);
    EXPECT_EQ(0, gamepad->right_analog_button);
    EXPECT_EQ(0, gamepad->left_analog_x);
    EXPECT_EQ(0, gamepad->left_analog_y);
    EXPECT_EQ(0, gamepad->right_analog_x);
    EXPECT_EQ(0, gamepad->right_analog_y);
}

TEST_F(GamepadTest, Close_ReturnsError_GivenInvalidGamepadId) {
    EXPECT_EQ(kGamepadInvalidId, gamepad_close(0));
    EXPECT_EQ(kGamepadInvalidId, gamepad_close(5));
}

TEST_F(GamepadTest, CloseAll_Succeeds_GivenAllGamepadsOpen) {
    for (uint8_t ii = 1; ii < 4; ++ii) {
        MmResult result = gamepad_open(ii, NULL, 0);
        EXPECT_EQ(kOk, result) << mmresult_to_string(result);
        EXPECT_EQ(kOk, gamepad_on_button_down(joystick[ii - 1].id, SDL_CONTROLLER_BUTTON_A));
        EXPECT_EQ(kOk, gamepad_on_button_down(joystick[ii - 1].id, SDL_CONTROLLER_BUTTON_START));
        EXPECT_EQ(kOk, gamepad_on_button_down(joystick[ii - 1].id, SDL_CONTROLLER_BUTTON_DPAD_RIGHT));
    }

    MmResult result = gamepad_close_all();
    EXPECT_EQ(kOk, result) << mmresult_to_string(result);

    for (uint8_t ii = 1; ii < 4; ++ii) {
        GamepadDevice *gamepad = &gamepad_devices[ii];
        EXPECT_EQ(-1, gamepad->sdlId);
        EXPECT_EQ(NULL, gamepad->joystick);
        EXPECT_EQ(NULL, gamepad->controller);
        EXPECT_EQ(0, gamepad->buttons);
        EXPECT_EQ(0, gamepad->left_x);
        EXPECT_EQ(0, gamepad->left_y);
        EXPECT_EQ(0, gamepad->right_x);
        EXPECT_EQ(0, gamepad->right_y);
        EXPECT_EQ(0, gamepad->left_analog_button);
        EXPECT_EQ(0, gamepad->right_analog_button);
        EXPECT_EQ(0, gamepad->left_analog_x);
        EXPECT_EQ(0, gamepad->left_analog_y);
        EXPECT_EQ(0, gamepad->right_analog_x);
        EXPECT_EQ(0, gamepad->right_analog_y);
    }
}

TEST_F(GamepadTest, Term_ClosesAll_GivenAllGamepadsOpen) {
    for (uint8_t ii = 1; ii < 4; ++ii) {
        MmResult result = gamepad_open(ii, NULL, 0);
        EXPECT_EQ(kOk, result) << mmresult_to_string(result);
        EXPECT_EQ(kOk, gamepad_on_button_down(joystick[ii - 1].id, SDL_CONTROLLER_BUTTON_A));
        EXPECT_EQ(kOk, gamepad_on_button_down(joystick[ii - 1].id, SDL_CONTROLLER_BUTTON_START));
        EXPECT_EQ(kOk, gamepad_on_button_down(joystick[ii - 1].id, SDL_CONTROLLER_BUTTON_DPAD_RIGHT));
    }

    MmResult result = gamepad_term();
    EXPECT_EQ(kOk, result) << mmresult_to_string(result);

    for (uint8_t ii = 1; ii < 4; ++ii) {
        GamepadDevice *gamepad = &gamepad_devices[ii];
        EXPECT_EQ(-1, gamepad->sdlId);
        EXPECT_EQ(NULL, gamepad->joystick);
        EXPECT_EQ(NULL, gamepad->controller);
        EXPECT_EQ(0, gamepad->buttons);
        EXPECT_EQ(0, gamepad->left_x);
        EXPECT_EQ(0, gamepad->left_y);
        EXPECT_EQ(0, gamepad->right_x);
        EXPECT_EQ(0, gamepad->right_y);
        EXPECT_EQ(0, gamepad->left_analog_button);
        EXPECT_EQ(0, gamepad->right_analog_button);
        EXPECT_EQ(0, gamepad->left_analog_x);
        EXPECT_EQ(0, gamepad->left_analog_y);
        EXPECT_EQ(0, gamepad->right_analog_x);
        EXPECT_EQ(0, gamepad->right_analog_y);
    }
}

TEST_F(GamepadTest, ReadButtons_Succeeds_GivenGamepadOpen) {
    int64_t out;
    EXPECT_EQ(kOk, gamepad_open(1, NULL, 0));;

    // Press the buttons down one at a time ...

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_A));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_B));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_X));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB | kButtonX, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_Y));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB | kButtonX | kButtonY, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_BACK));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB | kButtonX | kButtonY | kButtonSelect, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_GUIDE));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB | kButtonX | kButtonY | kButtonSelect | kButtonHome, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_START));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB | kButtonX | kButtonY | kButtonSelect | kButtonHome |
              kButtonStart, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_LEFTSTICK));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB | kButtonX | kButtonY | kButtonSelect | kButtonHome |
              kButtonStart, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_RIGHTSTICK));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB | kButtonX | kButtonY | kButtonSelect | kButtonHome |
              kButtonStart, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_LEFTSHOULDER));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB | kButtonX | kButtonY | kButtonSelect | kButtonHome |
              kButtonStart | kButtonL, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB | kButtonX | kButtonY | kButtonSelect | kButtonHome |
              kButtonStart | kButtonL | kButtonR, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_DPAD_UP));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB | kButtonX | kButtonY | kButtonSelect | kButtonHome |
              kButtonStart | kButtonL | kButtonR | kButtonUp, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_DPAD_DOWN));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB | kButtonX | kButtonY | kButtonSelect | kButtonHome |
              kButtonStart | kButtonL | kButtonR | kButtonUp | kButtonDown, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_DPAD_LEFT));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB | kButtonX | kButtonY | kButtonSelect | kButtonHome |
              kButtonStart | kButtonL | kButtonR | kButtonUp | kButtonDown | kButtonLeft, out);

    EXPECT_EQ(kOk, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_DPAD_RIGHT));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonA | kButtonB | kButtonX | kButtonY | kButtonSelect | kButtonHome |
              kButtonStart | kButtonL | kButtonR | kButtonUp | kButtonDown | kButtonLeft |
              kButtonRight, out);

    // And then release them one at a time in the same order ...

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_A));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonB | kButtonX | kButtonY | kButtonSelect | kButtonHome |
              kButtonStart | kButtonL | kButtonR | kButtonUp | kButtonDown | kButtonLeft |
              kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_B));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonX | kButtonY | kButtonSelect | kButtonHome |
              kButtonStart | kButtonL | kButtonR | kButtonUp | kButtonDown | kButtonLeft |
              kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_X));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonY | kButtonSelect | kButtonHome |
              kButtonStart | kButtonL | kButtonR | kButtonUp | kButtonDown | kButtonLeft |
              kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_Y));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonSelect | kButtonHome |
              kButtonStart | kButtonL | kButtonR | kButtonUp | kButtonDown | kButtonLeft |
              kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_BACK));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonHome |
              kButtonStart | kButtonL | kButtonR | kButtonUp | kButtonDown | kButtonLeft |
              kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_GUIDE));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonStart | kButtonL | kButtonR | kButtonUp | kButtonDown | kButtonLeft |
              kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_START));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonL | kButtonR | kButtonUp | kButtonDown | kButtonLeft | kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_LEFTSTICK));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonL | kButtonR | kButtonUp | kButtonDown | kButtonLeft | kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_RIGHTSTICK));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonL | kButtonR | kButtonUp | kButtonDown | kButtonLeft | kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_LEFTSHOULDER));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonR | kButtonUp | kButtonDown | kButtonLeft | kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonUp | kButtonDown | kButtonLeft | kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_DPAD_UP));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonDown | kButtonLeft | kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_DPAD_DOWN));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonLeft | kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_DPAD_LEFT));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonRight, out);

    EXPECT_EQ(kOk, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_DPAD_RIGHT));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(0x0, out);
}

TEST_F(GamepadTest, ReadButtons_ReturnsError_GivenGamepadNotOpen) {
    int64_t out;

    EXPECT_EQ(kGamepadNotOpen, gamepad_read_buttons(1, &out));
}

TEST_F(GamepadTest, ReadButtons_ReturnsError_GivenInvalidGamepadId) {
    int64_t out;

    EXPECT_EQ(kGamepadInvalidId, gamepad_read_buttons(0, &out));
    EXPECT_EQ(kGamepadInvalidId, gamepad_read_buttons(5, &out));
}

TEST_F(GamepadTest, OnAnalog_ReturnsInternalFault_GivenNoMatchingGamepad) {
    EXPECT_EQ(kInternalFault, gamepad_on_analog(joystick[0].id, SDL_CONTROLLER_AXIS_LEFTX, 255));
}

TEST_F(GamepadTest, OnButtonDown_ReturnsInternalFault_GivenNoMatchingGamepad) {
    EXPECT_EQ(kInternalFault, gamepad_on_button_down(joystick[0].id, SDL_CONTROLLER_BUTTON_A));
}

TEST_F(GamepadTest, OnButtonUp_ReturnsInternalFault_GivenNoMatchingGamepad) {
    EXPECT_EQ(kInternalFault, gamepad_on_button_up(joystick[0].id, SDL_CONTROLLER_BUTTON_A));
}

TEST_F(GamepadTest, ReadLeftX) {
    int64_t out;

    EXPECT_EQ(kGamepadInvalidId, gamepad_read_left_x(0, &out));
    EXPECT_EQ(kGamepadInvalidId, gamepad_read_left_x(5, &out));
    EXPECT_EQ(kGamepadNotOpen, gamepad_read_left_x(1, &out));
    EXPECT_EQ(kOk, gamepad_open(1, NULL, 0));;
    EXPECT_EQ(kOk, gamepad_read_left_x(1, &out));
    EXPECT_EQ(0x0, out);
    EXPECT_EQ(kOk, gamepad_on_analog(joystick[0].id, SDL_CONTROLLER_AXIS_LEFTX, 1024));
    EXPECT_EQ(kOk, gamepad_read_left_x(1, &out));
    EXPECT_EQ(1024, out);
}

TEST_F(GamepadTest, ReadLeftY) {
    int64_t out;

    EXPECT_EQ(kGamepadInvalidId, gamepad_read_left_y(0, &out));
    EXPECT_EQ(kGamepadInvalidId, gamepad_read_left_y(5, &out));
    EXPECT_EQ(kGamepadNotOpen, gamepad_read_left_y(1, &out));
    EXPECT_EQ(kOk, gamepad_open(1, NULL, 0));;
    EXPECT_EQ(kOk, gamepad_read_left_y(1, &out));
    EXPECT_EQ(0x0, out);
    EXPECT_EQ(kOk, gamepad_on_analog(joystick[0].id, SDL_CONTROLLER_AXIS_LEFTY, 1024));
    EXPECT_EQ(kOk, gamepad_read_left_y(1, &out));
    EXPECT_EQ(1024, out);
}

TEST_F(GamepadTest, ReadRightX) {
    int64_t out;

    EXPECT_EQ(kGamepadInvalidId, gamepad_read_right_x(0, &out));
    EXPECT_EQ(kGamepadInvalidId, gamepad_read_right_x(5, &out));
    EXPECT_EQ(kGamepadNotOpen, gamepad_read_right_x(1, &out));
    EXPECT_EQ(kOk, gamepad_open(1, NULL, 0));;
    EXPECT_EQ(kOk, gamepad_read_right_x(1, &out));
    EXPECT_EQ(0x0, out);
    EXPECT_EQ(kOk, gamepad_on_analog(joystick[0].id, SDL_CONTROLLER_AXIS_RIGHTX, 1024));
    EXPECT_EQ(kOk, gamepad_read_right_x(1, &out));
    EXPECT_EQ(1024, out);
}

TEST_F(GamepadTest, ReadRightY) {
    int64_t out;

    EXPECT_EQ(kGamepadInvalidId, gamepad_read_right_y(0, &out));
    EXPECT_EQ(kGamepadInvalidId, gamepad_read_right_y(5, &out));
    EXPECT_EQ(kGamepadNotOpen, gamepad_read_right_y(1, &out));
    EXPECT_EQ(kOk, gamepad_open(1, NULL, 0));;
    EXPECT_EQ(kOk, gamepad_read_right_y(1, &out));
    EXPECT_EQ(0x0, out);
    EXPECT_EQ(kOk, gamepad_on_analog(joystick[0].id, SDL_CONTROLLER_AXIS_RIGHTY, 1024));
    EXPECT_EQ(kOk, gamepad_read_right_y(1, &out));
    EXPECT_EQ(1024, out);
}

TEST_F(GamepadTest, ReadRightLeftAnalogButton) {
    int64_t out;

    EXPECT_EQ(kGamepadInvalidId, gamepad_read_left_analog_button(0, &out));
    EXPECT_EQ(kGamepadInvalidId, gamepad_read_left_analog_button(5, &out));
    EXPECT_EQ(kGamepadNotOpen, gamepad_read_left_analog_button(1, &out));
    EXPECT_EQ(kOk, gamepad_open(1, NULL, 0));;
    EXPECT_EQ(kOk, gamepad_read_left_analog_button(1, &out));
    EXPECT_EQ(0x0, out);
    EXPECT_EQ(kOk, gamepad_on_analog(joystick[0].id, SDL_CONTROLLER_AXIS_TRIGGERLEFT, 1024));
    EXPECT_EQ(kOk, gamepad_read_left_analog_button(1, &out));
    EXPECT_EQ(1024, out);
}

TEST_F(GamepadTest, ReadRightRightAnalogButton) {
    int64_t out;

    EXPECT_EQ(kGamepadInvalidId, gamepad_read_right_analog_button(0, &out));
    EXPECT_EQ(kGamepadInvalidId, gamepad_read_right_analog_button(5, &out));
    EXPECT_EQ(kGamepadNotOpen, gamepad_read_right_analog_button(1, &out));
    EXPECT_EQ(kOk, gamepad_open(1, NULL, 0));;
    EXPECT_EQ(kOk, gamepad_read_right_analog_button(1, &out));
    EXPECT_EQ(0x0, out);
    EXPECT_EQ(kOk, gamepad_on_analog(joystick[0].id, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 1024));
    EXPECT_EQ(kOk, gamepad_read_right_analog_button(1, &out));
    EXPECT_EQ(1024, out);
}

TEST_F(GamepadTest, ReadButtons_ReturnsZL_GivenTriggerLeftDown) {
    int64_t out;

    EXPECT_EQ(kOk, gamepad_open(1, NULL, 0));;
    EXPECT_EQ(kOk, gamepad_on_analog(joystick[0].id, SDL_CONTROLLER_AXIS_TRIGGERLEFT,
                                     (SDL_JOYSTICK_AXIS_MAX / 2) + 1));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonZL, out);
}

TEST_F(GamepadTest, ReadButtons_DoesNotReturnZL_GivenTriggerLeftUp) {
    int64_t out;

    EXPECT_EQ(kOk, gamepad_open(1, NULL, 0));;
    EXPECT_EQ(kOk, gamepad_on_analog(joystick[0].id, SDL_CONTROLLER_AXIS_TRIGGERLEFT,
                                     (SDL_JOYSTICK_AXIS_MAX / 2)));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(0x0, out);
}

TEST_F(GamepadTest, ReadButtons_ReturnsZR_GivenTriggerRightDown) {
    int64_t out;

    EXPECT_EQ(kOk, gamepad_open(1, NULL, 0));;
    EXPECT_EQ(kOk, gamepad_on_analog(joystick[0].id, SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
                                     (SDL_JOYSTICK_AXIS_MAX / 2) + 1));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(kButtonZR, out);
}

TEST_F(GamepadTest, ReadButtons_DoesNotReturnZR_GivenTriggerRightUp) {
    int64_t out;

    EXPECT_EQ(kOk, gamepad_open(1, NULL, 0));;
    EXPECT_EQ(kOk, gamepad_on_analog(joystick[0].id, SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
                                     (SDL_JOYSTICK_AXIS_MAX / 2)));
    EXPECT_EQ(kOk, gamepad_read_buttons(1, &out));
    EXPECT_EQ(0x0, out);
}

// TODO: gamepad_term()
