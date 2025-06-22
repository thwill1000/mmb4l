/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_image.c

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
#include "../common/graphics.h"

/** SAVE [ COMPRESSED ] IMAGE [ 24BPP | RGB121 ] file$ [, x, y, w, h] */
static MmResult cmd_save_image(const char *p, bool compressed) {
    if (!graphics_current) error_throw(kGraphicsInvalidReadSurface);

    BmpFormat format = kBmpFormat24bpp;
    const char *p2;
    if ((p2 = checkstring(p, "24BPP"))) {
        // Do nothing.
    } else if ((p2 = checkstring(p, "RGB121"))) {
        format = compressed ? kBmpFormatCompressedRgb121 : kBmpFormatRgb121;
    } else {
        p2 = p;
    }

    if (format == kBmpFormat24bpp && compressed) {
        return kImageInvalidFormat;
    }

    getargs(&p2, 9, DELIM_COMMA);
    if (argc != 1 && argc != 9) return kArgumentCount;

    char *filename = GetTempStrMemory();
    ON_FAILURE_RETURN(parse_filename(argv[0], filename, STRINGSIZE));

    const int x = has_arg(2) ? getinteger(argv[2]) : 0;
    const int y = has_arg(4) ? getinteger(argv[4]) : 0;
    const int w = has_arg(6) ? getinteger(argv[6]) : graphics_current->width;
    const int h = has_arg(8) ? getinteger(argv[8]) : graphics_current->height;

    return graphics_save_bmp(graphics_current, filename, format, x, y, w, h);
}

void cmd_save(void) {
    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(cmdline, "COMPRESSED IMAGE"))) {
        result = cmd_save_image(p, true);
    } else if ((p = checkstring(cmdline, "IMAGE"))) {
        result = cmd_save_image(p, false);
    } else {
        result = mmresult_ex(kSyntax, "Unknown SAVE subcommand: %s", cmdline);
    }
    ON_FAILURE_ERROR(result);
}
