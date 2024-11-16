/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_rgb.c

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

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/graphics.h"
#include "../common/utility.h"

static MmResult fun_rgb_parse_colour(const char *p, MmGraphicsColour *colour) {
    MmResult result = kOk;
    if (checkstring(p, "WHITE"))
        *colour = RGB_WHITE;
    else if (checkstring(p, "BLACK"))
        *colour = RGB_BLACK;
    else if (checkstring(p, "NOTBLACK"))
        *colour = RGB_NOTBLACK;
    else if (checkstring(p, "BEIGE"))
        *colour = RGB_BEIGE;
    else if (checkstring(p, "BLUE"))
        *colour = RGB_BLUE;
    else if (checkstring(p, "BROWN"))
        *colour = (mmb_options.simulate == kSimulateGameMite || mmb_options.simulate == kSimulatePicoMiteVga)
                ? RGB_BROWN_4BIT : RGB_BROWN;
    else if (checkstring(p, "GREEN"))
        *colour = RGB_GREEN;
    else if (checkstring(p, "CERULEAN"))
        *colour = RGB_CERULEAN;
    else if (checkstring(p, "COBALT"))
        *colour = RGB_COBALT;
    else if (checkstring(p, "CYAN"))
        *colour = RGB_CYAN;
    else if (checkstring(p, "FUCHSIA"))
        *colour = RGB_FUCHSIA;
    else if (checkstring(p, "GOLD"))
        *colour = RGB_GOLD;
    else if (checkstring(p, "GRAY"))
        *colour = RGB_GRAY;
    else if (checkstring(p, "GREY"))
        *colour = RGB_GRAY;
    else if (checkstring(p, "LIGHTGRAY"))
        *colour = RGB_LITEGRAY;
    else if (checkstring(p, "LIGHTGREY"))
        *colour = RGB_LITEGRAY;
    else if (checkstring(p, "LILAC"))
        *colour = RGB_LILAC;
    else if (checkstring(p, "MAGENTA"))
        *colour = (mmb_options.simulate == kSimulateGameMite || mmb_options.simulate == kSimulatePicoMiteVga)
                ? RGB_MAGENTA_4BIT : RGB_MAGENTA;
    else if (checkstring(p, "MIDGREEN"))
        *colour = RGB_MIDGREEN;
    else if (checkstring(p, "MYRTLE"))
        *colour = RGB_MYRTLE;
    else if (checkstring(p, "ORANGE"))
        *colour = RGB_ORANGE;
    else if (checkstring(p, "PINK"))
        *colour = RGB_PINK;
    else if (checkstring(p, "RED"))
        *colour = RGB_RED;
    else if (checkstring(p, "RUST"))
        *colour = RGB_RUST;
    else if (checkstring(p, "SALMON"))
        *colour = RGB_SALMON;
    else if (checkstring(p, "YELLOW"))
        *colour = RGB_YELLOW;
    else {
        result = kGraphicsInvalidColour;
    }
    return result;
}

/**
 * RGB(red, green, blue [, trans])
 * RGB(shortcut [, trans])
 */
void fun_rgb(void) {
    getargs(&ep, 7, ",");
    MmResult result = kOk;
    MmGraphicsColour colour = -1;
    switch (argc) {
        case 1:
            result = fun_rgb_parse_colour(argv[0], &colour);
            break;
        case 3:
            result = fun_rgb_parse_colour(argv[0], &colour);
            colour &= 0x00FFFFFF; // Clear the default transparency ...
            colour |= getint(argv[2], 0, 255) << 24; // ... and replace it.
            break;
        case 5:
            colour = RGB(getint(argv[0], 0, 255), getint(argv[2], 0, 255),
                         getint(argv[4], 0, 255), 255);
            break;
        case 7:
            colour = RGB(getint(argv[0], 0, 255), getint(argv[2], 0, 255),
                         getint(argv[4], 0, 255), getint(argv[6], 0, 255));
            break;
        default:
            result = kArgumentCount;
            break;
    }
    ON_FAILURE_ERROR(result);
    iret = colour;
    targ = T_INT;
}
