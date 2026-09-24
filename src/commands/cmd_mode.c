/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_mode.c

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
   be displayed on the console at startup (additional copyright messages may
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
#include "../common/graphics.h"
#include "../common/error.h"
#include "../common/parse.h"
#include "../common/utility.h"

/**
 * MODE mode [, colour_depth [, background [, interrupt]]]
 *
 * @param  colour_depth  On the real CMM2 this is the colour depth (8, 12, 16 or 32) of the video
 *                       page.
 *                       In MMB4L all the "video pages" are 32-bit and only the values 12 and 32
 *                       have any meaning (the same as MMB4W). A value of 12 makes MMB4L simulate
 *                       a three layer CMM2 display:
 *                         page/surface 1 -- top
 *                         page/surface 0
 *                         background     -- bottom
 *                       Pixels in the higher layer overwrite those in the lower levels as defined
 *                       by the transparency/alpha levels of the individual pixels.
 * @param  background    Colour of background layer when colour_depth == 12.
 * @param  interrupt     Unused by MMB4L; on the real CMM2 this is an interrupt routine called at
 *                       the start of frame blanking.
 */
static MmResult cmd_mode_cmm2(void) {
    getargs(&cmdline, 7, DELIM_COMMA);
    if ((argc % 2 == 0) || argc < 1) return kArgumentCount;

    const unsigned mode = getint(argv[0], MIN_CMM2_MODE, MAX_CMM2_MODE);
    unsigned colour_depth = (argc >= 3) ? getint(argv[2], 0, 32) : 32;
    if (colour_depth == 8 || colour_depth == 16) colour_depth = 32;
    const MmGraphicsColour background = (argc >= 5)
            ? getint(argv[4], RGB_BLACK, RGB_WHITE)
            : RGB_BLACK;
    // 'interrupt' parameter is ignored by MMB4L.
    return graphics_set_mode(mode, colour_depth, background);
}

static MmResult cmd_mode_picomite_hdmi(void) {
    getargs(&cmdline, 1, DELIM_COMMA);
    if (argc != 1) return kArgumentCount;

    const unsigned mode = getint(argv[0], MIN_PICOMITE_HDMI_MODE, MAX_PICOMITE_HDMI_MODE);
    return graphics_set_mode(mode, 32, RGB_BLACK);
}

static MmResult cmd_mode_picomite_vga(void) {
    getargs(&cmdline, 1, DELIM_COMMA);
    if (argc != 1) return kArgumentCount;

    const unsigned mode = getint(argv[0], MIN_PICOMITE_VGA_MODE, MAX_PICOMITE_VGA_MODE);
    return graphics_set_mode(mode, 32, RGB_BLACK);
}

void cmd_mode(void) {
    if (!mmb_features.has_cmd_mode) ON_FAILURE_ERROR(kUnsupportedOnCurrentDevice);

    MmResult result = kOk;
    switch (mmb_features.graphics_type) {
        case kGraphicsTypeCmm2:
            result = cmd_mode_cmm2();
            break;

        case kGraphicsTypePicomiteHdmi:
            result = cmd_mode_picomite_hdmi();
            break;

        case kGraphicsTypePicomiteVga:
            result = cmd_mode_picomite_vga();
            break;

        default:
            result = INTERNAL_FAULT_EX("invalid GraphicsType: %d", mmb_features.graphics_type);
            break;
    }
    ON_FAILURE_ERROR(result);
}
