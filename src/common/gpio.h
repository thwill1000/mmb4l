/*-*****************************************************************************

MMBasic for Linux (MMB4L)

gpio.h

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

#if !defined(MMB4L_GPIO_H)
#define MMB4L_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#include "gamepad.h"
#include "mmb4l.h"
#include "mmresult.h"

#define GPIO_MAX_PIN_NUM 43

#define GPIO_GP1    2
#define GPIO_GP2    4
#define GPIO_GP3    5
#define GPIO_GP4    6
#define GPIO_GP5    7
#define GPIO_GP8   11
#define GPIO_GP9   12
#define GPIO_GP10  14
#define GPIO_GP11  15
#define GPIO_GP12  16
#define GPIO_GP13  17
#define GPIO_GP14  19
#define GPIO_GP15  20
#define GPIO_GP22  29

#define GPIO_SNES_A_DATA GPIO_GP1
#define GPIO_SNES_A_LATCH GPIO_GP2
#define GPIO_SNES_A_CLOCK GPIO_GP3

#define GPIO_SNES_B_DATA GPIO_GP4
#define GPIO_SNES_B_LATCH GPIO_GP5
#define GPIO_SNES_B_CLOCK GPIO_GP22

typedef enum {
    kGpioPinOff = 0x0,
    kGpioPinDIn,
    kGpioPinDOut,
} GpioPinConfig;

/**
 * Initialises the fake GPIO and resets all the pins.
 */
void gpio_init();

/**
 * Terminates the 'gpio' module.
 */
void gpio_term();

/**
 * Is a hardware pin number (not GPnn) valid?
 *
 * @param  pin_num  The potential hardware pin number.
 */
bool gpio_is_valid_pin_num(uint8_t pin_num);

/**
 * Configures pin as input/output/off.
 *
 * @param  pin_num  The hardware pin number.
 * @param  config   The new pin configuration.
 */
MmResult gpio_configure_pin(uint8_t pin_num, GpioPinConfig config);

/**
 * Translates from GPnn to hardware pin number.
 *
 * @param[in]   pin_gp   The GP pin number.
 * @param[out]  pin_num  The hardware pin number.
 */
MmResult gpio_translate_from_pin_gp(uint8_t pin_gp, uint8_t *pin_num);

/**
 * Translates from hardware pin number to GPnn.
 *
 * @param[in]   pin_num  The hardware pin number.
 * @param[out]  pin_gp   The GP pin number.
 */
MmResult gpio_translate_from_pin_num(uint8_t pin_num, uint8_t *pin_gp);

/**
 * Gets the value of a given pin.
 *
 * @param[in]   pin_num  The hardware pin number.
 * @param[out]  value    On return will be set to the value, 0 or 1.
 */
MmResult gpio_get_pin_value(uint8_t pin_num, uint8_t *value);

/**
 * Sets the value of a given pin.
 *
 * @param[in]  pin_num  The hardware pin number.
 * @param[in]  value    The value, 0 or 1.
 */
MmResult gpio_set_pin_value(uint8_t pin_num, uint8_t value);

/**
 * Pulses a given pin.
 * If the pin is currently high then pulses low, or vice-versa.
 *
 * @param[in]  pin_num  The hardware pin number.
 * @param[in]  pulse    The duration of the pulse in milliseconds.
 */
MmResult gpio_pulse_pin(uint8_t pin_num, MMFLOAT width);

#endif  // #if !defined(MMB4L_GPIO_H)
