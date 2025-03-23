/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_device.c

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#define DEFAULT_RUMBLE_DURATION  10000

/** CONTROLLER CLASSIC CLOSE [i2c] */
static MmResult cmd_device_classic_close(const char *p) {
    getargs(&p, 1, DELIM_COMMA);
    if (argc > 1) return kArgumentCount;
    int wii_i2c = (argc > 0) ? getint(argv[0], 1, 3) : 3;
    MmGamepadId gamepad_id = gamepad_transform_wii_i2c(wii_i2c);
    if (gamepad_id == -1) return kInternalFault;
    return gamepad_close(gamepad_id);
}

/** CONTROLLER CLASSIC OPEN [i2c] [, interrupt] [, bitmask] */
static MmResult cmd_device_classic_open(const char *p) {
    getargs(&p, 5, DELIM_COMMA);
    if (argc != 0 && argc != 1 && argc != 3 && argc != 5) return kArgumentCount;
    int wii_i2c = (argc > 0) ? getint(argv[0], 1, 3) : 3;
    MmGamepadId gamepad_id = gamepad_transform_wii_i2c(wii_i2c);
    if (gamepad_id == -1) return kInternalFault;
    const char *interrupt = (argc > 1) ? GetIntAddress(argv[2]) : NULL;
    uint16_t bitmask = (argc > 3) ? getint(argv[4], 0, UINT16_MAX) : GAMEPAD_BITMASK_ALL;
    ON_FAILURE_RETURN(gamepad_open(gamepad_id));
    if (interrupt) ON_FAILURE_RETURN(gamepad_interrupt_enable(gamepad_id, interrupt, bitmask));
    return kOk;
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
    getargs(&p, 1, DELIM_COMMA);
    if (argc != 1) return kArgumentCount;
    MmGamepadId gamepad_id = getint(argv[0], 1, 4);
    return gamepad_close(gamepad_id);
}

/** DEVICE GAMEPAD INTERRUPT DISABLE id */
static MmResult cmd_device_gamepad_interrupt_disable(const char *p) {
    getargs(&p, 1, DELIM_COMMA);
    if (argc != 1) return kArgumentCount;
    MmGamepadId gamepad_id = getint(argv[0], 1, 4);

    if (mmb_options.simulate == kSimulatePicoMiteVgaUsb) {
        gamepad_id -= 2;
        ON_FAILURE_RETURN(gamepad_open(gamepad_id));
    }

    return gamepad_interrupt_disable(gamepad_id);
}

/** DEVICE GAMEPAD INTERRUPT ENABLE id, interrupt [, bitmask] */
static MmResult cmd_device_gamepad_interrupt_enable(const char *p) {
    getargs(&p, 5, DELIM_COMMA);
    if (!(argc & 1) || argc < 3) return kArgumentCount;
    MmGamepadId gamepad_id = getint(argv[0], 1, 4);
    const char *interrupt = has_arg(2) ? GetIntAddress(argv[2]) : NULL;
    const uint16_t bitmask = has_arg(4) ? getint(argv[4], 0, UINT16_MAX) : GAMEPAD_BITMASK_ALL;

    if (mmb_options.simulate == kSimulatePicoMiteVgaUsb) {
        gamepad_id -= 2;
        ON_FAILURE_RETURN(gamepad_open(gamepad_id));
    }

    return gamepad_interrupt_enable(gamepad_id, interrupt, bitmask);
}

/** DEVICE GAMEPAD LED id, red, green, blue */
static MmResult cmd_device_gamepad_led(const char *p) {
    getargs(&p, 7, DELIM_COMMA);
    if (argc != 7) return kArgumentCount;
    const MmGamepadId gamepad_id = getint(argv[0], 1, 4);
    const uint8_t red = getint(argv[2], 0, 0xFF);
    const uint8_t green = getint(argv[4], 0, 0xFF);
    const uint8_t blue = getint(argv[6], 0, 0xFF);
    return gamepad_set_led(gamepad_id, red, green, blue);
}

/** DEVICE GAMEPAD OPEN id [, interrupt] [, bitmask] */
static MmResult cmd_device_gamepad_open(const char *p) {
    getargs(&p, 5, DELIM_COMMA);
    if (!(argc & 1)) return kArgumentCount;
    MmGamepadId gamepad_id = getint(argv[0], 1, 4);
    const char *interrupt = (argc > 1) ? GetIntAddress(argv[2]) : NULL;
    uint16_t bitmask = (argc > 3) ? getint(argv[4], 0, UINT16_MAX) : GAMEPAD_BITMASK_ALL;
    ON_FAILURE_RETURN(gamepad_open(gamepad_id));
    if (interrupt) ON_FAILURE_RETURN(gamepad_interrupt_enable(gamepad_id, interrupt, bitmask));
    return kOk;
}

/**
 * DEVICE GAMEPAD RUMBLE id [, low_freq] [, high_freq] [, duration_ms]
 * DEVICE GAMEPAD RUMBLE id, OFF
 */
static MmResult cmd_device_gamepad_rumble(const char *p) {
    getargs(&p, 7, DELIM_COMMA);
    if (!(argc & 1)) return kArgumentCount;
    const MmGamepadId gamepad_id = getint(argv[0], 1, 4);
    uint16_t low_freq = 0;
    uint16_t high_freq = 0;
    uint32_t duration = 0;
    const char *p2;
    if ((p2 = checkstring(argv[2], "OFF"))) {
        if (argc != 3) return kArgumentCount;
    } else {
        low_freq = has_arg(2) ? getint(argv[2], 0, UINT16_MAX) : UINT16_MAX;
        high_freq = has_arg(4) ? getint(argv[4], 0, UINT16_MAX) : UINT16_MAX;
        duration = has_arg(6) ? getint(argv[6], 0, UINT32_MAX) : DEFAULT_RUMBLE_DURATION;
    }
    return gamepad_rumble(gamepad_id, low_freq, high_freq, duration);
}

/**
 * DEVICE GAMEPAD RUMBLE TRIGGERS id [, left] [, right] [, duration_ms]
 * DEVICE GAMEPAD RUMBLE TRIGGERS id, OFF
 */
static MmResult cmd_device_gamepad_rumble_triggers(const char *p) {
    getargs(&p, 7, DELIM_COMMA);
    if (!(argc & 1)) return kArgumentCount;
    const MmGamepadId gamepad_id = getint(argv[0], 1, 4);
    uint16_t left = 0;
    uint16_t right = 0;
    uint32_t duration = 0;
    const char *p2;
    if ((p2 = checkstring(argv[2], "OFF"))) {
        if (argc != 3) return kArgumentCount;
    } else {
        left = has_arg(2) ? getint(argv[2], 0, UINT16_MAX) : UINT16_MAX;
        right = has_arg(4) ? getint(argv[4], 0, UINT16_MAX) : UINT16_MAX;
        duration = has_arg(6) ? getint(argv[6], 0, UINT32_MAX) : DEFAULT_RUMBLE_DURATION;
    }
    return gamepad_rumble_triggers(gamepad_id, left, right, duration);
}

static MmResult cmd_device_gamepad(const char *p) {
    MmResult result = kOk;
    const char *p2;
    if ((p2 = checkstring(p, "CLOSE"))) {
        result = cmd_device_gamepad_close(p2);
    } else if ((p2 = checkstring(p, "INTERRUPT DISABLE"))) {
        result = cmd_device_gamepad_interrupt_disable(p2);
    } else if ((p2 = checkstring(p, "INTERRUPT ENABLE"))) {
        result = cmd_device_gamepad_interrupt_enable(p2);
    } else if ((p2 = checkstring(p, "LED"))) {
        result = cmd_device_gamepad_led(p2);
    } else if ((p2 = checkstring(p, "OPEN"))) {
        result = cmd_device_gamepad_open(p2);
    } else if ((p2 = checkstring(p, "RUMBLE TRIGGERS"))) {
        result = cmd_device_gamepad_rumble_triggers(p2);
    } else if ((p2 = checkstring(p, "RUMBLE"))) {
        result = cmd_device_gamepad_rumble(p2);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("DEVICE GAMEPAD");
        result = kUnimplemented;
    }
    return result;
}

/** DEVICE GAMEPAD COLOUR id, colour */
static MmResult cmd_device_gamepad_colour(const char *p) {
    getargs(&p, 3, DELIM_COMMA);
    if (argc != 3) return kArgumentCount;
    MmGamepadId gamepad_id = getint(argv[0], 1, 4);
    const MMINTEGER colour = getint(argv[2], 0, 0xFFFFFF);

    if (mmb_options.simulate == kSimulatePicoMiteVgaUsb) {
        gamepad_id -= 2;
        ON_FAILURE_RETURN(gamepad_open(gamepad_id));
    }

    ON_FAILURE_RETURN(gamepad_set_led(gamepad_id,
                                      colour >> 16, (colour >> 8) & 0xFF, colour & 0xFF));
    return kOk;
}

/** DEVICE GAMEPAD HAPTIC id, left, right */
static MmResult cmd_device_gamepad_haptic(const char *p) {
    getargs(&p, 5, DELIM_COMMA);
    if (argc != 5) return kArgumentCount;
    MmGamepadId gamepad_id = getint(argv[0], 1, 4);
    const MMINTEGER left = getint(argv[2], 0, 255) << 8;
    const MMINTEGER right = getint(argv[4], 0, 255) << 8;

    if (mmb_options.simulate == kSimulatePicoMiteVgaUsb) {
        gamepad_id -= 2;
        ON_FAILURE_RETURN(gamepad_open(gamepad_id));
    }

    // ON_FAILURE_RETURN(gamepad_rumble_triggers(gamepad_id, left, right, DEFAULT_RUMBLE_DURATION));
    ON_FAILURE_RETURN(gamepad_rumble(gamepad_id, left, right, DEFAULT_RUMBLE_DURATION));
    return kOk;
}

MmResult cmd_device_gamepad_pmvga_usb(const char *p) {
    MmResult result = kOk;
    const char *p2;
    if ((p2 = checkstring(p, "COLOUR"))) {
        result = cmd_device_gamepad_colour(p2);
    } else if ((p2 = checkstring(p, "INTERRUPT ENABLE"))) {
        result = cmd_device_gamepad_interrupt_enable(p2);
    } else if ((p2 = checkstring(p, "INTERRUPT DISABLE"))) {
        result = cmd_device_gamepad_interrupt_disable(p2);
    } else if ((p2 = checkstring(p, "HAPTIC"))) {
        result = cmd_device_gamepad_haptic(p2);
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
        if (mmb_options.simulate == kSimulatePicoMiteVgaUsb) {
            result = cmd_device_gamepad_pmvga_usb(p);
        } else {
            result = cmd_device_gamepad(p);
        }
    } else if ((p = checkstring(cmdline, "MOUSE"))) {
        result = cmd_device_mouse(p);
    } else if ((p = checkstring(cmdline, "NUNCHUK"))) {
        result = cmd_device_classic(p);
    } else if ((p = checkstring(cmdline, "NUNCHUCK"))) {
        result = cmd_device_classic(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("DEVICE");
    }
    ON_FAILURE_ERROR(result);
}
