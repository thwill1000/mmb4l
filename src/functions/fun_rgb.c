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

#define ERROR_INVALID_COLOUR(s)     error_throw_ex(kError, "Invalid colour: $", s)
#define ERROR_INVALID_TRANSPARENCY  error_throw_ex(kError, "Transparency not valid for this mode")

static int VideoColour = 8;

static int rgb(int r, int g, int b, int t) {
    return RGB(r, g, b, t);
}

static MMINTEGER fun_rgb_parse_colour(const char *p) {
    if (checkstring(p, "WHITE"))
        return RGB_WHITE;
    else if (checkstring(p, "BLACK"))
        return RGB_BLACK;
    else if (checkstring(p, "NOTBLACK"))
        return RGB_NOTBLACK;
    else if (checkstring(p, "BEIGE"))
        return RGB_BEIGE;
    else if (checkstring(p, "BLUE"))
        return RGB_BLUE;
    else if (checkstring(p, "BROWN"))
        return RGB_BROWN;
    else if (checkstring(p, "GREEN"))
        return RGB_GREEN;
    else if (checkstring(p, "CERULEAN"))
        return RGB_CERULEAN;
    else if (checkstring(p, "COBALT"))
        return RGB_COBALT;
    else if (checkstring(p, "CYAN"))
        return RGB_CYAN;
    else if (checkstring(p, "FUCHSIA"))
        return RGB_FUCHSIA;
    else if (checkstring(p, "GOLD"))
        return RGB_GOLD;
    else if (checkstring(p, "GRAY"))
        return RGB_GRAY;
    else if (checkstring(p, "GREY"))
        return RGB_GRAY;
    else if (checkstring(p, "LIGHTGRAY"))
        return RGB_LITEGRAY;
    else if (checkstring(p, "LIGHTGREY"))
        return RGB_LITEGRAY;
    else if (checkstring(p, "LILAC"))
        return RGB_LILAC;
    else if (checkstring(p, "MAGENTA"))
        return RGB_MAGENTA;
    else if (checkstring(p, "MIDGREEN"))
        return RGB_MIDGREEN;
    else if (checkstring(p, "MYRTLE"))
        return RGB_MYRTLE;
    else if (checkstring(p, "ORANGE"))
        return RGB_ORANGE;
    else if (checkstring(p, "PINK"))
        return RGB_PINK;
    else if (checkstring(p, "RED"))
        return RGB_RED;
    else if (checkstring(p, "RUST"))
        return RGB_RUST;
    else if (checkstring(p, "SALMON"))
        return RGB_SALMON;
    else if (checkstring(p, "YELLOW"))
        return RGB_YELLOW;
    else {
        ERROR_INVALID_COLOUR(p);
        return RGB_BLACK;
    }
}

void fun_rgb(void) {
    getargs(&ep, 7, ",");
    if (argc == 5) {
        iret = rgb(getint(argv[0], 0, 255), getint(argv[2], 0, 255),
                   getint(argv[4], 0, 255), 15);
    } else if (argc == 1) {
        iret = fun_rgb_parse_colour(argv[0]);
        if (VideoColour != 32) iret &= 0xFFFFFFF;
    } else if (argc == 3) {
        if (VideoColour == 8 || VideoColour == 16)
            ERROR_INVALID_TRANSPARENCY;
        iret = fun_rgb_parse_colour(argv[0]);
        iret &= 0xFFFFFF;
        if (VideoColour == 12) iret |= (getint(argv[2], 0, 15) << 24);
        if (VideoColour == 32) iret |= (getint(argv[2], 0, 255) << 24);
    } else if (argc == 7) {
        if (VideoColour == 8 || VideoColour == 16)
            ERROR_INVALID_TRANSPARENCY;
        iret = rgb(getint(argv[0], 0, 255), getint(argv[2], 0, 255),
                   getint(argv[4], 0, 255),
                   (VideoColour == 12 ? getint(argv[6], 0, 15)
                                      : getint(argv[6], 0, 255)));
    } else {
        ERROR_SYNTAX;
    }
    targ = T_INT;
}
