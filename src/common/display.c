/*-*****************************************************************************

MMBasic for Linux (MMB4L)

display.c

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

#include "console.h"
#include "display.h"
#include "error.h"
#include "fonttbl.h"
#include "graphics.h"

MmResult display_get_cursor_pos(bool pixel, int *x, int *y) {
    if (graphics_current) {
        return kUnimplemented;
    } else {
        int console_x, console_y;
        if (FAILED(console_get_cursor_pos(&console_x, &console_y, 10000))) {
           ON_FAILURE_RETURN(mmresult_ex(kError, "Cannot determine terminal cursor position"));
        }
        if (pixel) {
            *x = console_x * font_width(graphics_font);
            *y = console_y * font_height(graphics_font);
        } else {
            *x = console_x;
            *y = console_y;
        }
    }
    return kOk;
}

MmResult display_get_size(bool pixel, int *width, int *height) {
    if (graphics_current) {
        if (pixel) {
            *width = graphics_current->width;
            *height = graphics_current->height;
        } else {
            *width = graphics_current->width / font_width(graphics_font);
            *height = graphics_current->height / font_height(graphics_font);
        }
    } else {
        int console_width, console_height;
        if (FAILED(console_get_size(&console_width, &console_height, 0))) {
            ON_FAILURE_RETURN(mmresult_ex(kError, "Cannot determine terminal size"));
        }
        if (pixel) {
            *width = console_width * font_width(graphics_font);
            *height = console_height * font_height(graphics_font);
        } else {
            *width = console_width;
            *height = console_height;
        }
    }
    return kOk;
}

MmResult display_set_cursor_pos(bool pixel, int x, int y) {
   if (graphics_current) {
      return kUnimplemented;
   } else {
      if (pixel) {
         x /= font_width(graphics_font);
         y /= font_height(graphics_font);
      }
      console_set_cursor_pos(x, y);
   }
   return kOk;
}
