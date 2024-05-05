/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include "../../gamepad.h"

#include <stddef.h>

MmResult gamepad_term() { return kOk; }
const char *gamepad_last_error() { return "Stubbed error message"; }
MmResult gamepad_on_analog(int32_t sdlId, uint8_t sdlAxis, int16_t value) { return kOk; }
MmResult gamepad_on_button_down(int32_t sdlId, uint8_t sdlButton) { return kOk; }
MmResult gamepad_on_button_up(int32_t sdlId, uint8_t sdlButton) { return kOk; }
