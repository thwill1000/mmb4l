/*-*****************************************************************************

MMBasic for Linux (MMB4L)

gamepad.c

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

#include "cstring.h"
#include "events.h"
#include "gamepad.h"
#include "gamepad_private.h"
#include "interrupt.h"
#include "utility.h"
#include "../Configuration.h"

#define MAX_GAMEPADS  4

static const char* NO_ERROR = "";

static bool gamepad_initialised = false;
GamepadDevice gamepad_devices[MAX_GAMEPADS + 1]; // 0'th element is unused.

MmResult gamepad_init() {
    if (gamepad_initialised) return kOk;
    MmResult result = events_init();
    if (FAILED(result)) return result;

    memset(gamepad_devices, 0, sizeof(gamepad_devices));

    // Configure SDL so that controllers send events.
    SDL_GameControllerEventState(SDL_ENABLE);

    gamepad_initialised = true;
    return kOk;
}

MmResult gamepad_term() {
    if (!gamepad_initialised) return kOk;
    MmResult result = gamepad_close_all();
    gamepad_initialised = false;
    return result;
}

const char *gamepad_last_error() {
    const char* emsg = SDL_GetError();
    return emsg && *emsg ? emsg : NO_ERROR;
}

MmResult gamepad_info(MmGamepadId id, char *buf) {
    if (!gamepad_initialised) gamepad_init();
    buf[0] = '\0';
    if (SDL_NumJoysticks() < id) return kGamepadNotFound;
    SDL_GameController *controller = SDL_GameControllerOpen(id - 1);
    if (!controller) return kGamepadApiError;
    char *mapping = SDL_GameControllerMapping(controller);
    if (!mapping) return kGamepadApiError;
    cstring_cpy(buf, mapping, MAXSTRLEN);
    return kOk;
}

MmResult gamepad_open(MmGamepadId id, const char *interrupt, uint16_t bitmask) {
    if (!gamepad_initialised) gamepad_init();
    if (id < 1 || id > 4) return kGamepadInvalidId;
    GamepadDevice *gamepad = &gamepad_devices[id];
    if (gamepad->controller) return kOk;

    gamepad->joystick = SDL_JoystickOpen(id - 1);
    if (!gamepad->joystick) return kGamepadApiError;
    gamepad->controller = SDL_GameControllerOpen(id - 1);
    if (!gamepad->controller) return kGamepadApiError;
    gamepad->sdlId = SDL_JoystickInstanceID(gamepad->joystick);

    gamepad->interrupt_bitmask = bitmask ? bitmask : GAMEPAD_BITMASK_ALL;
    if (interrupt) {
        interrupt_enable(kInterruptGamepad1 + id - 1, interrupt);
    }

    return kOk;
}

MmResult gamepad_close(MmGamepadId id) {
    if (!gamepad_initialised) gamepad_init();
    if (id < 1 || id > 4) return kGamepadInvalidId;
    GamepadDevice *gamepad = &gamepad_devices[id];
    if (!gamepad->controller) return kOk;

    if (gamepad->controller) {
        SDL_GameControllerClose(gamepad->controller);
        SDL_JoystickClose(gamepad->joystick);
        memset(gamepad, 0x0, sizeof(GamepadDevice));
        gamepad->sdlId = -1;
    }

    interrupt_disable(kInterruptGamepad1 + id - 1);

    return kOk;
}

MmResult gamepad_close_all() {
    MmResult result = kOk;
    for (MmGamepadId id = 1; id <= MAX_GAMEPADS; ++id) {
        MmResult r = gamepad_close(id);
        if (SUCCEEDED(result)) result = r;
    }
    return result;
}

MmResult gamepad_on_analog(int32_t sdlId, uint8_t sdlAxis, int16_t value) {
    GamepadDevice *gamepad = NULL;
    for (MmGamepadId i = 1; i <= MAX_GAMEPADS; ++i) {
        if (sdlId == gamepad_devices[i].sdlId) {
            gamepad = &gamepad_devices[i];
            break;
        }
    }

    if (!gamepad) return kInternalFault;

    switch (sdlAxis) {
        case SDL_CONTROLLER_AXIS_LEFTX:
            gamepad->left_analog_x = value;
            break;
        case SDL_CONTROLLER_AXIS_LEFTY:
            gamepad->left_analog_y = value;
            break;
        case SDL_CONTROLLER_AXIS_RIGHTX:
            gamepad->right_analog_x = value;
            break;
        case SDL_CONTROLLER_AXIS_RIGHTY:
            gamepad->right_analog_y = value;
            break;
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            gamepad->left_analog_button = value;
            if (value > SDL_JOYSTICK_AXIS_MAX / 2) {
                gamepad->buttons |= kButtonZL;
            } else {
                gamepad->buttons &= ~kButtonZL;
            }
            break;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            gamepad->right_analog_button = value;
            if (value > SDL_JOYSTICK_AXIS_MAX / 2) {
                gamepad->buttons |= kButtonZR;
            } else {
                gamepad->buttons &= ~kButtonZR;
            }
            break;
        default:
            return kInternalFault;
    }

    return kOk;
}

static inline GamepadButton gamepad_map_button(uint8_t sdlButton) {
    switch (sdlButton) {
        case SDL_CONTROLLER_BUTTON_A:             return kButtonA;
        case SDL_CONTROLLER_BUTTON_B:             return kButtonB;
        case SDL_CONTROLLER_BUTTON_X:             return kButtonX;
        case SDL_CONTROLLER_BUTTON_Y:             return kButtonY;
        case SDL_CONTROLLER_BUTTON_BACK:          return kButtonSelect;
        case SDL_CONTROLLER_BUTTON_GUIDE:         return kButtonHome;
        case SDL_CONTROLLER_BUTTON_START:         return kButtonStart;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:     return 0x0; // UNSUPPORTED
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:    return 0x0; // UNSUPPORTED
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  return kButtonL;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return kButtonR;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:       return kButtonUp;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:     return kButtonDown;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:     return kButtonLeft;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:    return kButtonRight;
        default: {
            printf("Unknown button: %d\n", sdlButton);
            return 0x0;
        }
    }
}

MmResult gamepad_on_button_down(int32_t sdlId, uint8_t sdlButton) {
    for (MmGamepadId id = 1; id <= MAX_GAMEPADS; ++id) {
        if (sdlId == gamepad_devices[id].sdlId) {
            const GamepadButton btn = gamepad_map_button(sdlButton);
            gamepad_devices[id].buttons |= btn;
            if (btn & gamepad_devices[id].interrupt_bitmask) {
                interrupt_fire(kInterruptGamepad1 + id - 1);
            }
            return kOk;
        }
    }
    return kInternalFault;
}

MmResult gamepad_on_button_up(int32_t sdlId, uint8_t sdlButton) {
    for (MmGamepadId id = 1; id <= MAX_GAMEPADS; ++id) {
        if (sdlId == gamepad_devices[id].sdlId) {
            gamepad_devices[id].buttons &= ~gamepad_map_button(sdlButton);
            return kOk;
        }
    }
    return kInternalFault;
}

#define CHECK_GAMEPAD(id) \
    if (id < 1 || id > 4) return kGamepadInvalidId; \
    GamepadDevice *gamepad = &gamepad_devices[id]; \
    if (!gamepad->controller) return kGamepadNotOpen;

MmResult gamepad_read_buttons(MmGamepadId id, int64_t *out) {
    CHECK_GAMEPAD(id);
    *out = gamepad->buttons;
    // printf("0x%lx\n", *out);
    return kOk;
}

MmResult gamepad_read_left_x(MmGamepadId id, int64_t *out) {
    CHECK_GAMEPAD(id);
    *out = gamepad->left_analog_x;
    return kOk;
}

MmResult gamepad_read_left_y(MmGamepadId id, int64_t *out) {
    CHECK_GAMEPAD(id);
    *out = gamepad->left_analog_y;
    return kOk;
}

MmResult gamepad_read_right_x(MmGamepadId id, int64_t *out) {
    CHECK_GAMEPAD(id);
    *out = gamepad->right_analog_x;
    return kOk;
}

MmResult gamepad_read_right_y(MmGamepadId id, int64_t *out) {
    CHECK_GAMEPAD(id);
    *out = gamepad->right_analog_y;
    return kOk;
}

MmResult gamepad_read_left_analog_button(MmGamepadId id, int64_t *out) {
    CHECK_GAMEPAD(id);
    *out = gamepad->left_analog_button;
    return kOk;
}

MmResult gamepad_read_right_analog_button(MmGamepadId id, int64_t *out) {
    CHECK_GAMEPAD(id);
    *out = gamepad->right_analog_button;
    return kOk;
}

MmGamepadId gamepad_transform_wii_i2c(int32_t i2c) {
    switch (i2c) {
        case 1: // Right hand port on CMM2 G2
            return 2;
        case 2: // Middle port on CMM2 Deluxe
            return 3;
        case 3: // Only port on CMM2 G1, Left hand port on CMM2 G2
            return 1;
        default:
            return -1;
    }
}

MmResult gamepad_vibrate(MmGamepadId id, uint16_t low_freq, uint16_t high_freq, uint32_t duration_ms) {
    CHECK_GAMEPAD(id);
    if (SDL_GameControllerHasRumble(gamepad->controller)) {
        if (FAILED(SDL_GameControllerRumble(gamepad->controller, low_freq, high_freq, duration_ms))) {
            return kGamepadApiError;
        }
    }
    return kOk;
}
