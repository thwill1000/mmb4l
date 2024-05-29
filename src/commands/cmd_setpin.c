/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_setpin.c

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
#include "../common/mmresult.h"
#include "../common/options.h"
#include "../common/parse.h"
#include "../common/utility.h"

/** SETPIN pin, { DIN | DOUT | OFF } [, PULLUP] */
void cmd_setpin(void) {
    if (mmb_options.simulate != kSimulatePicoMiteVga && mmb_options.simulate != kSimulateGameMite) {
        error_throw(kUnsupportedOnCurrentDevice);
        return;
    }

    getargs(&cmdline, 5, ",");
    if (argc != 3 && argc != 5) {
        ERROR_ARGUMENT_COUNT;
        return;
    }

    uint8_t pin_num = 0;
    bool is_gp = false;
    const char *p = argv[0];
    MmResult result = parse_pin_num(&p, &pin_num, &is_gp);
    if (FAILED(result)) {
        error_throw(result);
        return;
    }

    if ((p = parse_check_string(argv[2], "DIN"))) {
        result = gpio_configure_pin(pin_num, kGpioPinDIn);
    } else if ((p = parse_check_string(argv[2], "DOUT"))) {
        result = gpio_configure_pin(pin_num, kGpioPinDOut);
    } else if ((p = parse_check_string(argv[2], "OFF"))) {
        result = gpio_configure_pin(pin_num, kGpioPinOff);
    } else {
        result = kSyntax;
    }

    if (argc == 5) {
        if ((p = parse_check_string(argv[4], "PULLUP"))) {
            // Ignored for now.
        } else {
            result = kSyntax;
        }
    }

    if (FAILED(result)) {
        error_throw(result);
        return;
    }
}
