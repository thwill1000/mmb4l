/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_pinc.c

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

#include "../common/gpio.h"
#include "../common/mmb4l.h"
#include "../common/utility.h"

/** PIN(pin) */
void fun_pin(void) {
    if (mmb_options.simulate != kSimulatePicoMiteVga) {
        error_throw(kUnsupportedOnCurrentDevice);
        return;
    }

    getargs(&ep, 1, ",");
    if (argc != 1) {
        ERROR_ARGUMENT_COUNT;
        return;
    }

    uint8_t pin_num = 0;
    bool is_gp = false;
    const char *tp = argv[0];
    MmResult result = parse_pin_num(&tp, &pin_num, &is_gp);
    if (FAILED(result)) {
        error_throw(result);
        return;
    }

    if (!parse_is_end(tp)) {
        error_throw(kUnexpectedText);
        return;
    }

    uint8_t value = 0;
    result = gpio_get_pin_value(pin_num, &value);
    if (FAILED(result)) {
        error_throw(result);
        return;
    }

    g_rtn_type = T_INT;
    g_integer_rtn = value;
}
