/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_gui.c

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

#include "../common/error.h"
#include "../common/graphics.h"
#include "../common/mmb4l.h"

/** GUI BITMAP x, y, bits [, width] [, height] [, scale] [, fcolour] [, bcolour] */
MmResult cmd_gui_bitmap(const char *p) {
    getargs(&p, 15, ",");
    if (!(argc & 1) || argc < 5) return kArgumentCount;

    int x = getinteger(argv[0]);
    int y = getinteger(argv[2]);

    // Get the type of argument 3 (the bitmap) and its value (INTEGER or STRING).
    int num_bytes = 8;
    char *s;
    MMFLOAT f;
    MMINTEGER i64;
    int t = T_NOTYPE;
    evaluate(argv[4], &f, &i64, &s, &t, true);
    if (t & T_NBR) {
        return kInvalidArgumentType;
    } else if (t & T_INT) {
        s = (char *) &i64;
    } else if (t & T_STR) {
        num_bytes = *s++;
    }

    int width = (argc > 5 && *argv[6]) ? getint(argv[6], 1, graphics_current->width) : 8;
    int height = (argc > 7 && *argv[8]) ? getint(argv[8], 1, graphics_current->height) : 8;
    int scale = (argc > 9 && *argv[10]) ? getint(argv[10], 1, 15) : 1;
    int fcolour = (argc > 11 && *argv[12]) ? getint(argv[12], RGB_BLACK, RGB_WHITE) : graphics_fcolour;
    int bcolour = (argc == 15) ? getint(argv[14], -1, RGB_WHITE) : graphics_bcolour;
    if (height * width > num_bytes * 8) return kNotEnoughData;

    return graphics_draw_bitmap(graphics_current, x, y, width, height, scale, fcolour, bcolour,
                                (unsigned char *) s);
}

#define ELSE_IF_UNIMPLEMENTED(s) \
    else if ((p = checkstring(cmdline, s))) { \
        ERROR_UNIMPLEMENTED("GUI " s); \
    }

void cmd_gui(void) {
    MmResult result = kOk;
    const char *p;

    if ((p = checkstring(cmdline, "BITMAP"))) {
        result = cmd_gui_bitmap(p);
    }
    ELSE_IF_UNIMPLEMENTED("AREA")
    ELSE_IF_UNIMPLEMENTED("BARGAUGE")
    ELSE_IF_UNIMPLEMENTED("BCOLOUR")
    ELSE_IF_UNIMPLEMENTED("BEEP")
    ELSE_IF_UNIMPLEMENTED("BITMAP")
    ELSE_IF_UNIMPLEMENTED("BUTTON")
    ELSE_IF_UNIMPLEMENTED("CALIBRATE")
    ELSE_IF_UNIMPLEMENTED("CAPTION")
    ELSE_IF_UNIMPLEMENTED("CHECKBOX")
    ELSE_IF_UNIMPLEMENTED("DELETE")
    ELSE_IF_UNIMPLEMENTED("DISABLE")
    ELSE_IF_UNIMPLEMENTED("DISPLAYBOX")
    ELSE_IF_UNIMPLEMENTED("ENABLE")
    ELSE_IF_UNIMPLEMENTED("FCOLOUR")
    ELSE_IF_UNIMPLEMENTED("FORMATBOX")
    ELSE_IF_UNIMPLEMENTED("FRAME")
    ELSE_IF_UNIMPLEMENTED("GAUGE")
    ELSE_IF_UNIMPLEMENTED("HIDE")
    ELSE_IF_UNIMPLEMENTED("INTERRUPT")
    ELSE_IF_UNIMPLEMENTED("LED")
    ELSE_IF_UNIMPLEMENTED("NUMBERBOX")
    ELSE_IF_UNIMPLEMENTED("PAGE")
    ELSE_IF_UNIMPLEMENTED("RADIO")
    ELSE_IF_UNIMPLEMENTED("REDRAW")
    ELSE_IF_UNIMPLEMENTED("RESET")
    ELSE_IF_UNIMPLEMENTED("RESTORE")
    ELSE_IF_UNIMPLEMENTED("SETUP")
    ELSE_IF_UNIMPLEMENTED("SHOW")
    ELSE_IF_UNIMPLEMENTED("SPINBOX")
    ELSE_IF_UNIMPLEMENTED("SWITCH")
    ELSE_IF_UNIMPLEMENTED("TEST")
    ELSE_IF_UNIMPLEMENTED("TEXTBOX")
    else {
        ERROR_UNKNOWN_SUBCOMMAND("GUI");
    }

    ERROR_ON_FAILURE(result);
}
