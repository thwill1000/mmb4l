/*-*****************************************************************************

MMBasic for Linux (MMB4L)

display.c

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

#include <assert.h>

#include "console.h"
#include "display.h"
#include "error.h"
#include "fonttbl.h"
#include "graphics.h"
#include "options.h"
#include "termgfx.h"

#define TTY_TERMINAL_ENABLED()  (mmb_options.console & kSerial)
#define GFX_TERMINAL_ENABLED()  (graphics_current && (mmb_options.console & kScreen))

MmResult display_bell() {
    // Preferentially play the tty terminal bell.
    if (TTY_TERMINAL_ENABLED()) {
        console_bell();
        return kOk;
    }

    if (GFX_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(termgfx_bell());
    }

    return kOk;
}

MmResult display_cls() {
    if (TTY_TERMINAL_ENABLED()) {
        console_clear();
    }

    if (GFX_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(termgfx_cls());
    }

    return kOk;
}

MmResult display_colour_bg(MmGraphicsColour argb) {
    if (mmb_options.console & kSerial) {
        ON_FAILURE_RETURN(console_colour_bg(argb));
    }

    if (graphics_current && (mmb_options.console & kScreen)) {
        ON_FAILURE_RETURN(termgfx_colour_bg(argb));
    }

    return kOk;
}

MmResult display_colour_fg(MmGraphicsColour argb) {
    if (mmb_options.console & kSerial) {
        ON_FAILURE_RETURN(console_colour_fg(argb));
    }

    if (graphics_current && (mmb_options.console & kScreen)) {
        ON_FAILURE_RETURN(termgfx_colour_fg(argb));
    }

    return kOk;
}

MmResult display_cursor_left(int count, bool wrap) {
    if (TTY_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(console_cursor_left(count, wrap));
    }

    if (GFX_TERMINAL_ENABLED())  {
        ON_FAILURE_RETURN(termgfx_cursor_left(count, wrap));
    }

    return kOk;
}

MmResult display_cursor_up(int count) {
    if (TTY_TERMINAL_ENABLED()) {
        console_cursor_up(count);
    }

    if (GFX_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(termgfx_cursor_up(count));
    }

    return kOk;
}

MmResult display_get_cursor_pos(bool pixel, int *x, int *y) {
    // Preferentially get cursor position from graphics terminal.
    if (GFX_TERMINAL_ENABLED()) {
        return termgfx_get_cursor_pos(pixel, x, y);
    }

    if (TTY_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(console_get_cursor_pos(x, y));
        if (pixel) {
            *x *= font_width(graphics_font);
            *y *= font_height(graphics_font);
        }
    } else {
        *x = -1;
        *y = -1;
    }

    return kOk;
}

MmResult display_get_size(bool pixel, int *width, int *height) {
    // Preferentially get termianl size from graphics terminal.
    if (GFX_TERMINAL_ENABLED()) {
        return termgfx_get_size(pixel, width, height);
    }

    if (TTY_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(console_get_size(width, height));
        if (pixel) {
            *width *= font_width(graphics_font);
            *height *= font_height(graphics_font);
        }
    }
    return kOk;
}

MmResult display_inverse(bool inverse) {
    if (TTY_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(console_inverse(inverse));
    }

    if (GFX_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(termgfx_inverse(inverse));
    }

    return kOk;
}

MmResult display_putc(char c) {
    if (TTY_TERMINAL_ENABLED()) {
        (void) console_putc(c);
    }

    if (GFX_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(termgfx_putc(c));
    }

    return kOk;
}

MmResult display_puts(const char *s) {
    if (TTY_TERMINAL_ENABLED()) {
        console_puts(s);
    }

    if (GFX_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(termgfx_puts(s));
    }

    return kOk;
}

MmResult display_reset() {
    if (mmb_options.console & kSerial) {
        ON_FAILURE_RETURN(console_reset());
    }

    if (graphics_current && (mmb_options.console & kScreen)) {
        ON_FAILURE_RETURN(termgfx_reset());
    }

    return kOk;
}

MmResult display_scroll_down() {
    if (mmb_options.console & kSerial) {
        ON_FAILURE_RETURN(console_scroll_down());
    }

    if (graphics_current && (mmb_options.console & kScreen)) {
        ON_FAILURE_RETURN(termgfx_scroll_down());
    }

    return kOk;
}

MmResult display_scroll_up() {
    if (mmb_options.console & kSerial) {
        ON_FAILURE_RETURN(console_scroll_up());
    }

    if (graphics_current && (mmb_options.console & kScreen)) {
        ON_FAILURE_RETURN(termgfx_scroll_up());
    }

    return kOk;
}

MmResult display_set_cursor_pos(bool pixel, int x, int y) {
    if (TTY_TERMINAL_ENABLED()) {
        if (pixel) {
            console_set_cursor_pos(x / font_width(graphics_font), y / font_height(graphics_font));
        } else {
            console_set_cursor_pos(x, y);
        }
    }

    if (GFX_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(termgfx_set_cursor_pos(pixel, x, y));
    }

    return kOk;
}

MmResult display_show_cursor(bool show) {
    if (TTY_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(console_show_cursor(show));
    }

    if (GFX_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(termgfx_show_cursor(show));
    }

    return kOk;
}

MmResult display_sync() {
    if (TTY_TERMINAL_ENABLED()) {
        return console_sync();
    }

    return kOk;
}

MmResult display_underline(bool underline) {
    if (mmb_options.console & kSerial) {
        ON_FAILURE_RETURN(console_underline(underline));
    }

    if (graphics_current && (mmb_options.console & kScreen)) {
        ON_FAILURE_RETURN(termgfx_underline(underline));
    }

    return kOk;
}

MmResult display_update_cursor() {
    if (TTY_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(console_update_cursor());
    }

    if (GFX_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(termgfx_update_cursor());
    }

    return kOk;
}

MmResult display_wrapline() {
    if (TTY_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(console_wrapline());
    }

    if (GFX_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(termgfx_wrapline());
    }

    return kOk;
}

MmResult display_write(const char *buf, size_t *sz) {
    if (TTY_TERMINAL_ENABLED()) {
        *sz = console_write(buf, *sz);
    }

    if (GFX_TERMINAL_ENABLED()) {
        ON_FAILURE_RETURN(termgfx_write(buf, sz));
    }

    return kOk;
}
