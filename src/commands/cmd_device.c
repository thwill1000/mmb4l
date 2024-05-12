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

#include "../common/gamepad.h"
#include "../common/mmb4l.h"
#include "../common/utility.h"

static void cmd_device_classic_close(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    MMINTEGER wii_i2c = getint(argv[0], 1, 3);
    MmGamepadId gamepad_id = gamepad_transform_wii_i2c(wii_i2c);
    if (gamepad_id == -1) ERROR_INTERNAL_FAULT;
    MmResult result = gamepad_close(gamepad_id);
    if (FAILED(result)) error_throw(result);
}

static void cmd_device_classic_open(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    MMINTEGER wii_i2c = getint(argv[0], 1, 3);
    MmGamepadId gamepad_id = gamepad_transform_wii_i2c(wii_i2c);
    if (gamepad_id == -1) ERROR_INTERNAL_FAULT;
    MmResult result = gamepad_open(gamepad_id);
    if (FAILED(result)) error_throw(result);
}

/**
 * Maps CMM2 (Wii) "CLASSIC" controller commands to MMB4L "GAMEPAD" controllers.
 */
static void cmd_device_classic(const char *p) {
    if (mmb_options.device != kDeviceCmm2) {
        ERROR_UNIMPLEMENTED("CONTROLLER CLASSIC command except for 'Colour Maximite 2'");
    }
    const char *p2;
    if ((p2 = checkstring(p, "CLOSE"))) {
        cmd_device_classic_close(p2);
    } else if ((p2 = checkstring(p, "OPEN"))) {
        cmd_device_classic_open(p2);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("CONTROLLER CLASSIC");
    }
}

static void cmd_device_gamepad_close(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    MMINTEGER gamepad_id = getint(argv[0], 1, 4);
    MmResult result = gamepad_close(gamepad_id);
    if (FAILED(result)) error_throw(result);
}

static void cmd_device_gamepad_open(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    MMINTEGER gamepad_id = getint(argv[0], 1, 4);
    MmResult result = gamepad_open(gamepad_id);
    if (FAILED(result)) error_throw(result);
}

static void cmd_device_gamepad(const char *p) {
    const char *p2;
    if ((p2 = checkstring(p, "CLOSE"))) {
        cmd_device_gamepad_close(p2);
    } else if ((p2 = checkstring(p, "OPEN"))) {
        cmd_device_gamepad_open(p2);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("DEVICE GAMEPAD");
    }
}

static void cmd_device_mouse(const char *p) {
    ERROR_UNIMPLEMENTED("DEVICE MOUSE");
}

void cmd_device(void) {
    const char *p;
    if ((p = checkstring(cmdline, "CLASSIC"))) {
        cmd_device_classic(p);
    } else if ((p = checkstring(cmdline, "GAMEPAD"))) {
        cmd_device_gamepad(p);
    } else if ((p = checkstring(cmdline, "MOUSE"))) {
        cmd_device_mouse(p);
    } else if ((p = checkstring(cmdline, "NUNCHUK"))) {
        cmd_device_classic(p);
    } else if ((p = checkstring(cmdline, "NUNCHUCK"))) {
        cmd_device_classic(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("DEVICE");
    }
}