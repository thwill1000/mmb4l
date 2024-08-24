/*-*****************************************************************************

MMBasic for Linux (MMB4L)

gpio.c

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

#include "gpio.h"

#include "mmb4l.h"
#include "mmtime.h"
#include "utility.h"

typedef struct {
    uint8_t pin_num;
    uint8_t pin_gp;
    bool valid;
    GpioPinConfig config;
    uint8_t value;
} GpioPin;

static GpioPin gpio_pins[GPIO_MAX_PIN_NUM + 1] = {
    {.pin_num = 0, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 1, .pin_gp = 0, .valid = true},
    {.pin_num = 2, .pin_gp = 1, .valid = true},
    {.pin_num = 3, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 4, .pin_gp = 2, .valid = true},
    {.pin_num = 5, .pin_gp = 3, .valid = true},
    {.pin_num = 6, .pin_gp = 4, .valid = true},
    {.pin_num = 7, .pin_gp = 5, .valid = true},
    {.pin_num = 8, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 9, .pin_gp = 6, .valid = true},
    {.pin_num = 10, .pin_gp = 7, .valid = true},
    {.pin_num = 11, .pin_gp = 8, .valid = true},
    {.pin_num = 12, .pin_gp = 9, .valid = true},
    {.pin_num = 13, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 14, .pin_gp = 10, .valid = true},
    {.pin_num = 15, .pin_gp = 11, .valid = true},
    {.pin_num = 16, .pin_gp = 12, .valid = true},
    {.pin_num = 17, .pin_gp = 13, .valid = true},
    {.pin_num = 18, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 19, .pin_gp = 14, .valid = true},
    {.pin_num = 20, .pin_gp = 15, .valid = true},
    {.pin_num = 21, .pin_gp = 16, .valid = true},
    {.pin_num = 22, .pin_gp = 17, .valid = true},
    {.pin_num = 23, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 24, .pin_gp = 18, .valid = true},
    {.pin_num = 25, .pin_gp = 19, .valid = true},
    {.pin_num = 26, .pin_gp = 20, .valid = true},
    {.pin_num = 27, .pin_gp = 21, .valid = true},
    {.pin_num = 28, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 29, .pin_gp = 22, .valid = true},
    {.pin_num = 30, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 31, .pin_gp = 26, .valid = true},
    {.pin_num = 32, .pin_gp = 27, .valid = true},
    {.pin_num = 33, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 34, .pin_gp = 28, .valid = true},
    {.pin_num = 35, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 36, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 37, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 38, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 39, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 40, .pin_gp = 0xFF, .valid = false},
    {.pin_num = 41, .pin_gp = 23, .valid = true},
    {.pin_num = 42, .pin_gp = 24, .valid = true},
    {.pin_num = 43, .pin_gp = 25, .valid = true}
};

/**
 * Map of GP pin (0..28) to hardware pin number (1..40).
 * Hardware pins 41 .. 44 are the unavailable pins on the Raspberry Pi Pico.
 */
static const uint8_t GP_PIN_MAP[] = {1,  2,  4,  5,  6,  7,  9,  10, 11, 12, 14, 15, 16, 17, 19,
                                     20, 21, 22, 24, 25, 26, 27, 29, 41, 42, 43, 31, 32, 34, 44};

/** Simulated bits returned by SNES controller A. */
static uint16_t gpio_snes_a;

/** Simulated bits returned by SNES controller B. */
static uint16_t gpio_snes_b;

static bool gpio_initialised = false;

static void gpio_reset() {
    for (size_t pin_num = 0; pin_num <= GPIO_MAX_PIN_NUM; ++pin_num) {
        gpio_pins[pin_num].config = kGpioPinOff;
        gpio_pins[pin_num].value = 0;
    }
    gpio_snes_a = 0b1111111111111111;
    gpio_snes_b = 0b1111111111111111;
}

void gpio_init() {
    if (!gpio_initialised) {
        gpio_reset();
        gpio_initialised = true;
    }
}

void gpio_term() {
    if (gpio_initialised) {
        gpio_reset();
        gpio_initialised = false;
    }
}

/**
 * Is a GP pin number valid?
 *
 * @param  pin_gp  The potential GP pin number.
 */
static bool gpio_is_valid_pin_gp(uint8_t pin_gp) {
    return pin_gp <= 28;
}

static MmResult gpio_configure_pin_gamemite(uint8_t pin_num, GpioPinConfig config) {
    MmResult result = kOk;
    switch (pin_num) {
        case GPIO_GP8:
            if (config == kGpioPinDIn && gpio_pins[pin_num].config == kGpioPinOff) {
                result = gamepad_open(1, NULL, 0);
            } else if (config == kGpioPinOff && gpio_pins[pin_num].config == kGpioPinDIn) {
                result = gamepad_close(1);
            }
            break;
        default:
            break;
    }
    return result;
}

/**
 * This calls gamepad_{open|close} as appropriate when a pin corresponding to the SNES gamepad
 * A or B latch is configured.
 *
 * @param  id       Gamepad ID, 1 = SNES A, 2 = SNES B.
 * @param  pin_num  The corresponding latch pin.
 * @param  config   The new pin configuration. Note this function does not modify the current
 *                  pin configuration.
 */
static MmResult gpio_configure_snes_latch(MmGamepadId id, uint8_t pin_num, GpioPinConfig config) {
    MmResult result = kOk;
    if (config == kGpioPinDOut && gpio_pins[pin_num].config == kGpioPinOff) {
        result = gamepad_open(id, NULL, 0);
    } else if (config == kGpioPinOff && gpio_pins[pin_num].config == kGpioPinDOut) {
        result = gamepad_close(id);
    }
    return result;
}

static MmResult gpio_configure_pin_picomite_vga(uint8_t pin_num, GpioPinConfig config) {
    switch (pin_num) {
        case GPIO_SNES_A_LATCH:
            return gpio_configure_snes_latch(1, GPIO_SNES_A_LATCH, config);

        case GPIO_SNES_B_LATCH:
            return gpio_configure_snes_latch(2, GPIO_SNES_B_LATCH, config);

        case GPIO_SNES_A_CLOCK:
        case GPIO_SNES_A_DATA:
        case GPIO_SNES_B_CLOCK:
        case GPIO_SNES_B_DATA:
            return kOk;

        default:
            // Only the pins used by the PicoGAME VGA SNES controllers are supported.
            return kUnsupportedParameterOnCurrentDevice;
    }
}

MmResult gpio_configure_pin(uint8_t pin_num, GpioPinConfig config) {
    if (!gpio_initialised) gpio_init();
    if (!gpio_is_valid_pin_num(pin_num)) return kGpioInvalidPin;

    MmResult result = kOk;

    switch (mmb_options.simulate) {
        case kSimulateGameMite:
            result = gpio_configure_pin_gamemite(pin_num, config);
            break;
        case kSimulatePicoMiteVga:
            result = gpio_configure_pin_picomite_vga(pin_num, config);
            break;
        default:
            break;
    }

    if (SUCCEEDED(result)) gpio_pins[pin_num].config = config;

    return result;
}

MmResult gpio_translate_from_pin_gp(uint8_t pin_gp, uint8_t *pin_num) {
    if (!gpio_is_valid_pin_gp(pin_gp)) {
        *pin_num = 0xFF;
        return kGpioInvalidPin;
    } else {
        *pin_num = GP_PIN_MAP[pin_gp];
        return kOk;
    }
}

MmResult gpio_translate_from_pin_num(uint8_t pin_num, uint8_t *gp_pin) {
    if (!gpio_is_valid_pin_num(pin_num) || gpio_pins[pin_num].pin_gp == 0xFF) {
        *gp_pin = 0xFF;
        return kGpioInvalidPin;
    } else {
        *gp_pin = gpio_pins[pin_num].pin_gp;
        return kOk;
    }
}

bool gpio_is_valid_pin_num(uint8_t pin_num) {
    return pin_num <= GPIO_MAX_PIN_NUM && gpio_pins[pin_num].valid;
}

MmResult gpio_get_pin_value(uint8_t pin_num, uint8_t *value) {
    if (!gpio_initialised) gpio_init();
    if (!gpio_is_valid_pin_num(pin_num)) return kGpioInvalidPin;

    MmResult result = kOk;
    if (mmb_options.simulate == kSimulateGameMite) {
        GamepadButton button = 0x0;
        switch (pin_num) {
            case GPIO_GP8:
                button = kButtonDown;
                break;
            case GPIO_GP9:
                button = kButtonLeft;
                break;
            case GPIO_GP10:
                button = kButtonUp;
                break;
            case GPIO_GP11:
                button = kButtonRight;
                break;
            case GPIO_GP12:
                button = kButtonSelect;
                break;
            case GPIO_GP13:
                button = kButtonStart;
                break;
            case GPIO_GP14:
                button = kButtonB | kButtonY;
                break;
            case GPIO_GP15:
                button = kButtonA | kButtonX;
                break;
            default:
                break;
        }
        if (button) {
            int64_t out = 0x0;
            result = gamepad_read_buttons(1, &out);
            if (SUCCEEDED(result)) {
                *value = (out & button) ? 0 : 1;
            }
        } else {
            *value = gpio_pins[pin_num].value;
        }
    } else {
        *value = gpio_pins[pin_num].value;
    }

    return result;
}

/**
 * Reads gamepad buttons and fills 16-bit "shift register" with the corresponding SNES bit pattern.
 */
static MmResult gpio_read_snes_bits(MmGamepadId gamepad_id, uint16_t *snes) {
    int64_t buttons = 0x0;
    MmResult result = gamepad_read_buttons(gamepad_id, &buttons);
    if (FAILED(result)) return result;
    *snes = 0b1111111111111111 & ~(((buttons & kButtonB) > 0) ? 1 : 0) &
            ~(((buttons & kButtonY) > 0) << 1) & ~(((buttons & kButtonSelect) > 0) << 2) &
            ~(((buttons & kButtonStart) > 0) << 3) & ~(((buttons & kButtonUp) > 0) << 4) &
            ~(((buttons & kButtonDown) > 0) << 5) & ~(((buttons & kButtonLeft) > 0) << 6) &
            ~(((buttons & kButtonRight) > 0) << 7) & ~(((buttons & kButtonA) > 0) << 8) &
            ~(((buttons & kButtonX) > 0) << 9) & ~(((buttons & kButtonL) > 0) << 10) &
            ~(((buttons & kButtonR) > 0) << 11);
    return kOk;
}

/** Called BEFORE a pin transitions from high to low. */
static MmResult gpio_on_pin_high_to_low(uint8_t pin_num) { return kOk; }

/** Called BEFORE a pin transitions from low to high. */
static MmResult gpio_on_pin_low_to_high(uint8_t pin_num) {
    if (mmb_options.simulate == kSimulatePicoMiteVga) {
        switch (pin_num) {
            case GPIO_SNES_A_LATCH: {
                gpio_read_snes_bits(1, &gpio_snes_a);
                gpio_pins[GPIO_SNES_A_DATA].value = gpio_snes_a & 0x1;
                break;
            }

            case GPIO_SNES_A_CLOCK: {
                gpio_snes_a >>= 1;                  // Shift right.
                gpio_snes_a |= 0b1000000000000000;  // Fill with 1.
                gpio_pins[GPIO_SNES_A_DATA].value = gpio_snes_a & 0x1;
                break;
            }

            case GPIO_SNES_B_LATCH: {
                gpio_read_snes_bits(2, &gpio_snes_b);
                gpio_pins[GPIO_SNES_B_DATA].value = gpio_snes_b & 0x1;
                break;
            }

            case GPIO_SNES_B_CLOCK: {
                gpio_snes_b >>= 1;                  // SHift right.
                gpio_snes_b |= 0b1000000000000000;  // Fill with 1.
                gpio_pins[GPIO_SNES_B_DATA].value = gpio_snes_b & 0x1;
                break;
            }

            default:
                break;
        }
    }
    return kOk;
}

MmResult gpio_set_pin_value(uint8_t pin_num, uint8_t value) {
    if (!gpio_initialised) gpio_init();
    if (!gpio_is_valid_pin_num(pin_num)) return kGpioInvalidPin;
    if (gpio_pins[pin_num].config != kGpioPinDOut) return kGpioPinIsNotAnOutput;
    if (value != 0 && value != 1) return kInvalidValue;
    MmResult result = kOk;
    switch (gpio_pins[pin_num].value) {
        case 0:
            if (value == 1) result = gpio_on_pin_low_to_high(pin_num);
            break;
        case 1:
            if (value == 0) result = gpio_on_pin_high_to_low(pin_num);
            break;
        default:
            result = kInternalFault;
            break;
    }
    if (SUCCEEDED(result)) gpio_pins[pin_num].value = value;
    return result;
}

// NOTE: The PicoMite version of MMBasic performs a synchronous pulse if width < 3ms and an
//       asynchronous pulse if width >= 3ms. In addition it supports multiple asynchronous pulses
//       "in flight" at the same time.
MmResult gpio_pulse_pin(uint8_t pin_num, MMFLOAT width) {
    if (!gpio_initialised) gpio_init();
    if (width < 0.0) return kGpioInvalidPulseWidth;
    if (width == 0.0) return kOk;  // Ignore zero width pulse.

    // Read current value.
    uint8_t original_value = 0;
    MmResult result = gpio_get_pin_value(pin_num, &original_value);
    if (FAILED(result)) return result;

    // Invert value.
    result = gpio_set_pin_value(pin_num, (original_value == 0) ? 1 : 0);
    if (FAILED(result)) return result;

    // Wait.
    mmtime_sleep_ns((int64_t)(width * 1000000.0));

    // Revert value.
    return gpio_set_pin_value(pin_num, original_value);
}
