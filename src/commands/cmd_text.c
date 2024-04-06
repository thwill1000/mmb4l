/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_text.c

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
#include "../common/fonttbl.h"
#include "../common/graphics.h"
#include "../common/mmb4l.h"

static bool GetJustification(char* p, TextHAlign* jh, TextVAlign* jv, TextOrientation* jo) {
    switch (toupper(*p++)) {
        case 'L':
            *jh = kAlignLeft;
            break;
        case 'C':
            *jh = kAlignCenter;
            break;
        case 'R':
            *jh = kAlignRight;
            break;
        case 0:
            return true;
        default:
            p--;
    }
    skipspace(p);
    switch (toupper(*p++)) {
        case 'T':
            *jv = kAlignTop;
            break;
        case 'M':
            *jv = kAlignMiddle;
            break;
        case 'B':
            *jv = kAlignBottom;
            break;
        case 0:
            return true;
        default:
            p--;
    }
    skipspace(p);
    switch (toupper(*p++)) {
        case 'N':
            *jo = kOrientNormal;
            break;  // normal
        case 'V':
            *jo = kOrientVert;
            break;  // vertical text (top to bottom)
        case 'I':
            *jo = kOrientInverted;
            break;  // inverted
        case 'U':
            *jo = kOrientCounterClock;
            break;  // rotated CCW 90 degrees
        case 'D':
            *jo = kOrientClockwise;
            break;  // rotated CW 90 degrees
        case 0:
            return true;
        default:
            return false;
    }
    return *p == 0;
}

/** TEXT x, y, string$ [, alignment$] [, font] [, scale] [, fcolour] [, bcolour] */
void cmd_text(void) {
    if (!graphics_current) error_throw(kGraphicsSurfaceNotFound);

    TextHAlign jh = 0;
    TextVAlign jv = 0;
    TextOrientation jo = 0;

    getargs(&cmdline, 17, ",");
    if (!(argc & 1) || argc < 5) ERROR_ARGUMENT_COUNT;
    MMINTEGER x = getinteger(argv[0]);
    MMINTEGER y = getinteger(argv[2]);
    char* s = getCstring(argv[4]);

    if (argc > 5 && *argv[6]) {
        if (!GetJustification((char*)argv[6], &jh, &jv, &jo))
            if (!GetJustification((char*)getCstring(argv[6]), &jh, &jv, &jo))
                error_throw_ex(kSyntax, "Justification");
    }

    uint32_t font = (graphics_font >> 4) + 1;
    uint32_t scale = (graphics_font & 0b1111);
    MmGraphicsColour fcolour = graphics_fcolour;
    MmGraphicsColour bcolour = graphics_bcolour;

    if (argc > 7 && *argv[8]) {
        if (*argv[8] == '#') argv[8]++;
        font = (uint32_t) getint(argv[8], 1, FONT_TABLE_SIZE);
    }
    if (FontTable[font - 1] == NULL) error_throw_ex(kInvalidFont, "Invalid font #%", font);

    if (argc > 9 && *argv[10]) scale = (uint32_t)getint(argv[10], 1, 15);

    if (argc > 11 && *argv[12]) fcolour = (MmGraphicsColour) getint(argv[12], RGB_BLACK, RGB_WHITE);

    if (argc == 15) bcolour = (MmGraphicsColour) getint(argv[14], -1, RGB_WHITE);

    MmResult result = graphics_draw_string(graphics_current, x, y, ((font - 1) << 4) | scale, jh,
                                           jv, jo, fcolour, bcolour, s);
    if (FAILED(result)) error_throw(result);
}
