/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_device.c

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

#include <stdint.h>

#include "../common/gamepad.h"
#include "../common/mmb4l.h"
#include "../common/utility.h"

/** Adjusts range of signed 16-bit value (-32768 .. 32767) to an unsigned 8-bit value. */
static int transform_analog_for_mmb4w(int in) {
    int out = 128;
    if (in < INT16_MIN) {
        out = 0;
    } else if (in > INT16_MAX) {
        out = 255;
    } else if (in < -3072 || in > 3072) { // Allow some play in joystick center.
        out = max(0, min(256, 128 + (in / 256)));
    }
    return out;
}

MmResult fun_device_gamepad(const char *p, bool mmb4w_compatibility) {
    getargs(&p, 3, ",");
    if (argc != 1 && argc != 3) ERROR_ARGUMENT_COUNT;
    MmGamepadId gamepad_id = (argc == 3) ? getint(argv[0], 1, 4) : 1;
    const char *flag = argv[argc - 1];
    const char *p2;
    MmResult result = kOk;
    targ = T_INT;
    if ((p2 = checkstring(flag, "B"))) {
        result = gamepad_read_buttons(gamepad_id, &iret);
        return result;
    } else if ((p2 = checkstring(flag, "LX"))) {
        result = gamepad_read_left_x(gamepad_id, &iret);
    } else if ((p2 = checkstring(flag, "LY"))) {
        result = gamepad_read_left_y(gamepad_id, &iret);
    } else if ((p2 = checkstring(flag, "RX"))) {
        result = gamepad_read_right_x(gamepad_id, &iret);
    } else if ((p2 = checkstring(flag, "RY"))) {
        result = gamepad_read_right_y(gamepad_id, &iret);
    } else if ((p2 = checkstring(flag, "L"))) {
        result = gamepad_read_left_analog_button(gamepad_id, &iret);
    } else if ((p2 = checkstring(flag, "R"))) {
        result = gamepad_read_right_analog_button(gamepad_id, &iret);
    } else {
        result = kGamepadUnknownFunction;
    }
    if (SUCCEEDED(result) && mmb4w_compatibility) iret = transform_analog_for_mmb4w(iret);
    return result;
}

void fun_device(void) {
    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(ep, "GAMEPAD"))) {
        result = fun_device_gamepad(p, false);
    } else {
        ERROR_UNKNOWN_SUBFUNCTION("DEVICE");
    }
    ON_FAILURE_ERROR(result);
}
