/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_device.c

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

/** CONTROLLER CLASSIC CLOSE [i2c] */
static MmResult cmd_device_classic_close(const char *p) {
    getargs(&p, 1, ",");
    if (argc > 1) return kArgumentCount;
    int wii_i2c = (argc > 0) ? getint(argv[0], 1, 3) : 3;
    MmGamepadId gamepad_id = gamepad_transform_wii_i2c(wii_i2c);
    if (gamepad_id == -1) return kInternalFault;
    return gamepad_close(gamepad_id);
}

/** CONTROLLER CLASSIC OPEN [i2c] [, interrupt] [, bitmask] */
static MmResult cmd_device_classic_open(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 0 && argc != 1 && argc != 3 && argc != 5) return kArgumentCount;
    int wii_i2c = (argc > 0) ? getint(argv[0], 1, 3) : 3;
    MmGamepadId gamepad_id = gamepad_transform_wii_i2c(wii_i2c);
    if (gamepad_id == -1) return kInternalFault;
    const char *interrupt = (argc > 1) ? GetIntAddress(argv[2]) : NULL;
    uint16_t bitmask = (argc > 3) ? getint(argv[4], 0, UINT16_MAX) : GAMEPAD_BITMASK_ALL;
    return gamepad_open(gamepad_id, interrupt, bitmask);
}

/**
 * Maps CMM2 (Wii) "CLASSIC" controller commands to MMB4L "GAMEPAD" controllers.
 */
static MmResult cmd_device_classic(const char *p) {
    if (mmb_options.simulate != kSimulateCmm2) return kUnsupportedOnCurrentDevice;
    const char *p2;
    if ((p2 = checkstring(p, "CLOSE"))) {
        return cmd_device_classic_close(p2);
    } else if ((p2 = checkstring(p, "OPEN"))) {
        return cmd_device_classic_open(p2);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("CONTROLLER CLASSIC");
        return kUnimplemented;
    }
}

/** DEVICE GAMEPAD CLOSE id */
static MmResult cmd_device_gamepad_close(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) return kArgumentCount;
    MmGamepadId gamepad_id = getint(argv[0], 1, 4);
    return gamepad_close(gamepad_id);
}

/** DEVICE GAMEPAD OPEN id [, interrupt] [, bitmask] */
static MmResult cmd_device_gamepad_open(const char *p) {
    getargs(&p, 5, ",");
    if (!(argc & 1) || argc > 5) return kArgumentCount;
    MmGamepadId gamepad_id = getint(argv[0], 1, 4);
    const char *interrupt = (argc > 1) ? GetIntAddress(argv[2]) : NULL;
    uint16_t bitmask = (argc > 3) ? getint(argv[4], 0, UINT16_MAX) : GAMEPAD_BITMASK_ALL;
    return gamepad_open(gamepad_id, interrupt, bitmask);
}

/**
 * DEVICE GAMEPAD VIBRATE id [, low_freq] [, high_freq] [, duration_ms]
 * DEVICE GAMEPAD VIBRATE id OFF
 */
static MmResult cmd_device_gamepad_vibrate(const char *p) {
    getargs(&p, 7, ",");
    if (!(argc & 1) || argc > 7) return kArgumentCount;
    MmGamepadId gamepad_id = getint(argv[0], 1, 4);
    if (argc == 3) {
        const char *p2;
        if ((p2 = checkstring(argv[2], "OFF"))) {
            return gamepad_vibrate(gamepad_id, 0, 0, 0);
        }
    }
    uint16_t low_freq = (argc > 1) ? getint(argv[2], 0, UINT16_MAX) : 0xFFFF;
    uint16_t high_freq = (argc > 3) ? getint(argv[4], 0, UINT16_MAX) : 0xFFFF;
    uint32_t duration = (argc > 5) ? getint(argv[6], 0, UINT32_MAX) : 10000;
    return gamepad_vibrate(gamepad_id, low_freq, high_freq, duration);
}

static MmResult cmd_device_gamepad(const char *p) {
    MmResult result = kOk;
    const char *p2;
    if ((p2 = checkstring(p, "CLOSE"))) {
        result = cmd_device_gamepad_close(p2);
    } else if ((p2 = checkstring(p, "OPEN"))) {
        result = cmd_device_gamepad_open(p2);
    } else if ((p2 = checkstring(p, "VIBRATE"))) {
        result = cmd_device_gamepad_vibrate(p2);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("DEVICE GAMEPAD");
        result = kUnimplemented;
    }
    return result;
}

static MmResult cmd_device_mouse(const char *p) {
    ERROR_UNIMPLEMENTED("DEVICE MOUSE");
    return kUnimplemented;
}

void cmd_device(void) {
    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(cmdline, "CLASSIC"))) {
        result = cmd_device_classic(p);
    } else if ((p = checkstring(cmdline, "GAMEPAD"))) {
        result = cmd_device_gamepad(p);
    } else if ((p = checkstring(cmdline, "MOUSE"))) {
        result = cmd_device_mouse(p);
    } else if ((p = checkstring(cmdline, "NUNCHUK"))) {
        result = cmd_device_classic(p);
    } else if ((p = checkstring(cmdline, "NUNCHUCK"))) {
        result = cmd_device_classic(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("DEVICE");
    }
    ERROR_ON_FAILURE(result);
}
