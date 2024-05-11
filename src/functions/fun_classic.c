/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_classic.c

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

#include "../common/gamepad.h"
#include "../common/mmb4l.h"
#include "../common/utility.h"

/** CLASSIC(funct [, i2c]) */
void fun_classic(void) {
    if (mmb_options.simulate != kSimulateCmm2) {
        ERROR_UNIMPLEMENTED("CLASSIC function except for 'Colour Maximite 2'");
    }

    getargs(&ep, 3, ",");
    if (argc != 1 && argc != 3) ERROR_ARGUMENT_COUNT;
    const int wii_i2c = (argc == 3) ? getint(argv[2], 1, 3) : 3;
    const MmGamepadId gamepad_id = gamepad_transform_wii_i2c(wii_i2c);
    if (gamepad_id == -1) ERROR_INTERNAL_FAULT;

    const char *p2;
    MmResult result = kOk;
    targ = T_INT;
    if ((p2 = checkstring(argv[0], "B"))) {
        result = gamepad_read_buttons(gamepad_id, &iret);
    } else if ((p2 = checkstring(argv[0], "LX"))) {
        result = gamepad_read_left_x(gamepad_id, &iret);
    } else if ((p2 = checkstring(argv[0], "LY"))) {
        result = gamepad_read_left_y(gamepad_id, &iret);
    } else if ((p2 = checkstring(argv[0], "RX"))) {
        result = gamepad_read_right_x(gamepad_id, &iret);
    } else if ((p2 = checkstring(argv[0], "RY"))) {
        result = gamepad_read_right_y(gamepad_id, &iret);
    } else if ((p2 = checkstring(argv[0], "L"))) {
        result = gamepad_read_left_analog_button(gamepad_id, &iret);
    } else if ((p2 = checkstring(argv[0], "R"))) {
        result = gamepad_read_right_analog_button(gamepad_id, &iret);
    } else if ((p2 = checkstring(argv[0], "T"))) {
        iret = 0xA4200101; // I2C id for Wii Classic controller.
        result = kOk;
    } else {
        error_throw_ex(kSyntax, "Unknown gamepad function");
    }
    ERROR_ON_FAILURE(result);
}
