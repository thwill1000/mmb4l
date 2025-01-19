/*-*****************************************************************************

MMBasic for Linux (MMB4L)

gamepad.h

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

#if !defined(MMBASIC_GAMEPAD_H)
#define MMBASIC_GAMEPAD_H

#include "mmresult.h"

/** Bitmask to pass to gamepad_open() to interrupt on all digital button presses. */
#define GAMEPAD_BITMASK_ALL  0b111111111111111

typedef enum {
    kButtonR      = 0x01,
    kButtonStart  = 0x02,
    kButtonHome   = 0x04,
    kButtonSelect = 0x08,
    kButtonL      = 0x10,
    kButtonDown   = 0x20,
    kButtonRight  = 0x40,
    kButtonUp     = 0x80,
    kButtonLeft   = 0x100,
    kButtonZR     = 0x200,
    kButtonX      = 0x400,
    kButtonA      = 0x800,
    kButtonY      = 0x1000,
    kButtonB      = 0x2000,
    kButtonZL     = 0x4000,
} GamepadButton;

typedef int32_t MmGamepadId;

MmResult gamepad_init();
MmResult gamepad_term();
const char *gamepad_last_error();
MmResult gamepad_info(MmGamepadId id, char *buf);
MmResult gamepad_open(MmGamepadId id, const char *interrupt, uint16_t bitmask);
MmResult gamepad_close(MmGamepadId id);
MmResult gamepad_close_all();
MmResult gamepad_on_analog(int32_t sdlId, uint8_t sdlAxis, int16_t value);
MmResult gamepad_on_button_down(int32_t sdlId, uint8_t sdlButton);
MmResult gamepad_on_button_up(int32_t sdlId, uint8_t sdlButton);
MmResult gamepad_read_buttons(MmGamepadId id, int64_t *out);
MmResult gamepad_read_left_x(MmGamepadId id, int64_t *out);
MmResult gamepad_read_left_y(MmGamepadId id, int64_t *out);
MmResult gamepad_read_right_x(MmGamepadId id, int64_t *out);
MmResult gamepad_read_right_y(MmGamepadId id, int64_t *out);
MmResult gamepad_read_left_analog_button(MmGamepadId id, int64_t *out);
MmResult gamepad_read_right_analog_button(MmGamepadId id, int64_t *out);
MmResult gamepad_vibrate(MmGamepadId id, uint16_t low_freq, uint16_t high_freq, uint32_t duration_ms);

/**
 * Transforms CMM2 Wii Classic Controller i2c channel to a corresponding MmGamepadId.
 */
MmGamepadId gamepad_transform_wii_i2c(int32_t i2c);

#endif // #if !defined(MMBASIC_GAMEPAD_H)
