/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_image.c

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

static void cmd_image_resize(const char *p) {
    ERROR_UNIMPLEMENTED("IMAGE RESIZE");
}

static void cmd_image_resize_fast(const char *p) {
    ERROR_UNIMPLEMENTED("IMAGE RESIZE FAST");
}

static void cmd_image_rotate(const char *p) {
    ERROR_UNIMPLEMENTED("IMAGE ROTATE");
}

static void cmd_image_rotate_fast(const char *p) {
    ERROR_UNIMPLEMENTED("IMAGE ROTATE FAST");
}

static void cmd_image_warp_h(const char *p) {
    ERROR_UNIMPLEMENTED("IMAGE WARP H");
}

static void cmd_image_warp_v(const char *p) {
    ERROR_UNIMPLEMENTED("IMAGE WARP V");
}

void cmd_image(void) {
    const char *p;
    if ((p = checkstring(cmdline, "RESIZE"))) {
        cmd_image_resize(p);
    } else if ((p = checkstring(cmdline, "RESIZE_FAST"))) {
        cmd_image_resize_fast(p);
    } else if ((p = checkstring(cmdline, "RESIZE FAST"))) {
        cmd_image_resize_fast(p);
    } else if ((p = checkstring(cmdline, "ROTATE"))) {
        cmd_image_rotate(p);
    } else if ((p = checkstring(cmdline, "ROTATE_FAST"))) {
        cmd_image_rotate_fast(p);
    } else if ((p = checkstring(cmdline, "ROTATE FAST"))) {
        cmd_image_rotate_fast(p);
    } else if ((p = checkstring(cmdline, "WARP_H"))) {
        cmd_image_warp_h(p);
    } else if ((p = checkstring(cmdline, "WARP H"))) {
        cmd_image_warp_h(p);
    } else if ((p = checkstring(cmdline, "WARP_V"))) {
        cmd_image_warp_v(p);
    } else if ((p = checkstring(cmdline, "WARP V"))) {
        cmd_image_warp_v(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("IMAGE");
    }
}