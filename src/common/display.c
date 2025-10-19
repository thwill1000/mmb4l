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
#include "mmtime.h"

#define CURSOR_PERIOD  SECONDS_TO_NANOSECONDS(1) / 2

MmResult display_bell() {
    console_bell();
    return kOk;
}

MmResult display_cls() {
    console_clear();
    if (graphics_current && mmb_options.console != kSerial) {
        ON_FAILURE_RETURN(graphics_cls(graphics_current, graphics_bcolour));
    }
    return kOk;
}

MmResult display_cursor_up(int i) {
    assert(i > 0);
    console_cursor_up(i);
    if (graphics_current && mmb_options.console != kSerial) {
        graphics_current->cursor_y -= font_height(graphics_font);
    }
    return kOk;
}

MmResult display_get_cursor_pos(bool pixel, int *x, int *y) {
    if (graphics_current) {
        *x = graphics_current->cursor_x;
        *y = graphics_current->cursor_y;
        if (!pixel) {
            *x /= font_width(graphics_font);
            *y /= font_height(graphics_font);
        }
    } else {
        int console_x, console_y;
        if (FAILED(console_get_cursor_pos(&console_x, &console_y, 10000))) {
           return mmresult_ex(kError, "Cannot determine terminal cursor position");
        }
        *x = console_x;
        *y = console_y;
        if (pixel) {
            *x *= font_width(graphics_font);
            *y *= font_height(graphics_font);
        }
    }
    return kOk;
}

MmResult display_get_size(bool pixel, int *width, int *height) {
    if (graphics_current) {
        *width = graphics_current->width;
        *height = graphics_current->height;
        if (!pixel) {
            *width /= font_width(graphics_font);
            *height /= font_height(graphics_font);
        }
    } else {
        int console_width, console_height;
        if (FAILED(console_get_size(&console_width, &console_height, 0))) {
            return mmresult_ex(kError, "Cannot determine terminal size");
        }
        *width = console_width;
        *height = console_height;
        if (pixel) {
            *width *= font_width(graphics_font);
            *height *= font_height(graphics_font);
        }
    }
    return kOk;
}

static MmResult display_draw_cursor(MmGraphicsColour colour) {
    const int fh = (int) font_height(graphics_font);
    const int fw = (int) font_width(graphics_font);
    MmSurface *s = graphics_current;
    return graphics_draw_line(
            s,
            s->cursor_x,
            s->cursor_y + fh - (fh <= 12 ? 1 : 2),
            s->cursor_x + fw - 1,
            s->cursor_y + fh - (fh <= 12 ? 1 : 2),
            1,
            colour);
}

MmResult display_hide_cursor() {
    if (!graphics_current || mmb_options.console == kSerial) return kOk;

    return display_draw_cursor(graphics_bcolour);
}

static MmResult display_putc_graphics(char c) {
    assert(graphics_current && mmb_options.console != kSerial);

    MmSurface *s = graphics_current;
    const uint32_t font = graphics_font;
    const int fh = (int) font_height(graphics_font);
    const int fw = (int) font_width(graphics_font);

    // If 'c' is printable and it is going to take us off the right hand end of the display
    // then print a CRLF.
    if (c >= font_first_char(font) && c <= font_last_char(font)) {
        if (s->cursor_x + fw > s->width) {
            ON_FAILURE_RETURN(display_putc_graphics('\r'));
            ON_FAILURE_RETURN(display_putc_graphics('\n'));
        }
    }

    // Handle the standard control chars.
    switch(c) {
        case '\b':
            s->cursor_x -= fw;
            if (s->cursor_x < 0) {   // Go to end of previous line
                s->cursor_y -= fh ;  // Go up one line
                if (s->cursor_y < 0) s->cursor_y = 0;
                const int width = s->width  / fw;
                s->cursor_x = (width - 1) * fw;  //go to last character
            }
            break;

        case '\r':
            s->cursor_x = 0;
            break;

        case '\n':
            if (s->cursor_y + 2 * fh > s->height) {
                ON_FAILURE_RETURN(graphics_scroll(s, 0, fh, graphics_bcolour));
            } else {
                s->cursor_y += fh;
            }
            break;

        case '\t':
            do {
                ON_FAILURE_RETURN(display_putc_graphics(' '));
            } while ((s->cursor_x / fw) % mmb_options.tab);
            break;

        default:
            ON_FAILURE_RETURN(
                graphics_draw_char(s, &s->cursor_x, &s->cursor_y, graphics_font, graphics_fcolour,
                                   graphics_bcolour, c, kOrientNormal));
    }

    return kOk;
}

MmResult display_putc(char c) {
    console_putc(c);
    if (graphics_current && mmb_options.console != kSerial) {
        return display_putc_graphics(c);
    }
    return kOk;
}

MmResult display_puts(const char *s) {
    console_puts(s);
    if (graphics_current && mmb_options.console != kSerial) {
        while (*s) ON_FAILURE_RETURN(display_putc_graphics(*s++));
    }
    return kOk;
}

MmResult display_set_cursor_pos(bool pixel, int x, int y) {
    if (pixel) {
        console_set_cursor_pos(x / font_width(graphics_font), y / font_height(graphics_font));
    } else {
        console_set_cursor_pos(x, y);
    }
    if (graphics_current && mmb_options.console != kSerial) {
        if (!pixel) {
            x *= font_width(graphics_font);
            y *= font_height(graphics_font);
        }
        graphics_current->cursor_x = x;
        graphics_current->cursor_y = y;
    }
    return kOk;
}

MmResult display_show_cursor() {
    if (!graphics_current || mmb_options.console == kSerial) return kOk;

    static int64_t t = 0;
    static bool visible = false;

    const int64_t now = mmtime_now_ns();
    bool new_visible = visible;
    if (now > t + CURSOR_PERIOD) {
        t = now;
        new_visible = !visible;
    }

    if (new_visible == visible) return kOk;

    visible = new_visible;

    return display_draw_cursor(visible ? graphics_fcolour : graphics_bcolour);
}

MmResult display_write(const char *buf, size_t *sz) {
    *sz = console_write(buf, *sz);

    if (graphics_current && mmb_options.console != kSerial) {
        for (size_t idx = 0; idx < *sz; ++idx) {
            ON_FAILURE_RETURN(display_putc_graphics(buf[idx]));
        }
    }

    return kOk;
}
