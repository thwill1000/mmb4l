/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_page.c

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

#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/graphics.h"
#include "../core/tokentbl.h"

MmResult cmd_graphics_copy(const char *p);
MmResult cmd_graphics_write(const char *p);

/** PAGE COPY src_id TO dst_id [, when] [, transparent] */
static inline MmResult cmd_page_copy(const char *p) {
    return cmd_graphics_copy(p);
}

/** PAGE DISPLAY n [, page] */
static MmResult cmd_page_display(const char *p) {
    ERROR_UNIMPLEMENTED("PAGE DISPLAY");
    return kUnimplemented;
}

/** PAGE RESIZE page, w, h */
static MmResult cmd_page_resize(const char *p) {
    ERROR_UNIMPLEMENTED("PAGE RESIZE");
    return kUnimplemented;
}

/** PAGE SCROLL page, x, y [, fill_colour] */
static MmResult cmd_page_scroll(const char *p) {
    getargs(&p, 7, ",");
    if (argc != 5 && argc != 7) return kArgumentCount;
    MmSurfaceId page_id = -1;
    MmResult result = parse_page(p, &page_id);
    if (SUCCEEDED(result)) {
        MmSurface *page = &graphics_surfaces[page_id];
        const int maxW = page->width;
        const int maxH = page->height;
        int x = getint(argv[2], -maxW / 2 - 1, maxW);
        int y = getint(argv[4], -maxH / 2 - 1, maxH);
        // The default -2 is wrap around.
        MmGraphicsColour colour = (argc == 7) ? getint(argv[6], -2, RGB_WHITE) : -2;
        result = graphics_scroll(page, x, y, colour);
    }
    return result;
}

/** PAGE STITCH from_page_1, from_page_2, to_page, offset */
static MmResult cmd_page_stitch(const char *p) {
    ERROR_UNIMPLEMENTED("PAGE STITCH");
    return kUnimplemented;
}

/** PAGE AND_PIXELS src_page_1, src_page_2, dst_page */
static MmResult cmd_page_and_pixels(const char *p) {
    ERROR_UNIMPLEMENTED("PAGE AND_PIXELS");
    return kUnimplemented;
}

/** PAGE OR_PIXELS src_page_1, src_page_2, dst_page */
static MmResult cmd_page_or_pixels(const char *p) {
    ERROR_UNIMPLEMENTED("PAGE OR_PIXELS");
    return kUnimplemented;
}

/** PAGE XOR_PIXELS src_page_1, src_page_2, dst_page */
static MmResult cmd_page_xor_pixels(const char *p) {
    ERROR_UNIMPLEMENTED("PAGE XOR_PIXELS");
    return kUnimplemented;
}

void cmd_page(void) {
    if (mmb_options.simulate != kSimulateCmm2
            && mmb_options.simulate != kSimulateMmb4w) {
        ERROR_ON_FAILURE(kUnsupportedOnCurrentDevice);
    }
    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(cmdline, "WRITE"))) {
        result = cmd_graphics_write(p);
    } else if ((p = checkstring(cmdline, "COPY"))) {
        result = cmd_page_copy(p);
    } else if ((p = checkstring(cmdline, "DISPLAY"))) {
        result = cmd_page_display(p);
    } else if ((p = checkstring(cmdline, "RESIZE"))) {
        result = cmd_page_resize(p);
    } else if ((p = checkstring(cmdline, "SCROLL"))) {
        result = cmd_page_scroll(p);
    } else if ((p = checkstring(cmdline, "STITCH"))) {
        result = cmd_page_stitch(p);
    } else if ((p = checkstring(cmdline, "AND_PIXELS"))) {
        result = cmd_page_and_pixels(p);
    } else if ((p = checkstring(cmdline, "OR_PIXELS"))) {
        result = cmd_page_or_pixels(p);
    } else if ((p = checkstring(cmdline, "XOR_PIXELS"))) {
        result = cmd_page_xor_pixels(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("PAGE");
    }

    ERROR_ON_FAILURE(result);
}
