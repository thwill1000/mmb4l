/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_arc.c

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

/**
 * ARC x, y, r1, [r2], rad1, rad2 [, colour]
 *
 * @param  r2  outer radius of the arc, can be omitted if 1 pixel wide.
 */
void cmd_arc(void) {
    if (!graphics_current) error_throw(kGraphicsSurfaceNotFound);

    getargs(&cmdline, 13, ",");
    if (argc != 11 && argc != 13) error_throw(kArgumentCount);

    const int x = getinteger(argv[0]);
    const int y = getinteger(argv[2]);
    int r1 = getinteger(argv[4]);
    int r2 = -1;
    if (*argv[6]) {
        r2 = getinteger(argv[6]);
    } else {
        r2 = r1;
        r1--;
    }
    if (r2 < r1) error_throw_ex(kInvalidArgument, "Inner radius < outer");
    int arcrad1 = getinteger(argv[8]);
    while (arcrad1 < 0) arcrad1 += 360;
    int arcrad2 = getinteger(argv[10]);
    while (arcrad2 < 0) arcrad2 += 360;
    while (arcrad2 < arcrad1) arcrad2 += 360;
    if (arcrad1 == arcrad2) error_throw_ex(kInvalidArgument, "Radials");
    const MmGraphicsColour colour =
        (argc == 13) ? getint(argv[12], RGB_BLACK, RGB_WHITE) : graphics_fcolour;

    ERROR_ON_FAILURE(graphics_draw_arc(graphics_current, x, y, r1, r2, arcrad1, arcrad2, colour));
}
