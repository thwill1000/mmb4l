/*-*****************************************************************************

MMBasic for Linux (MMB4L)

termgfx.c

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
#include <string.h>

#include "error.h"
#include "fonttbl.h"
#include "graphics.h"
#include "mmtime.h"
#include "termgfx.h"

#define ASSERT_GFX() \
    assert(graphics_current != NULL); \
    assert(mmb_options.console & kScreen)

#define CURSOR_PERIOD  SECONDS_TO_NANOSECONDS(1) / 2

typedef struct {
    bool cursor_visible;
    bool inverse;
    bool show_cursor;
    bool underline;
} TermGfxState;

TermGfxState self = {
    .cursor_visible = false,
    .inverse = false,
    .show_cursor = false,
    .underline = false
};

MmResult termgfx_bell() {
    ASSERT_GFX();
    return kOk; // Currently a no-op.
}

MmResult termgfx_clear_to_end_of_line() {
    ASSERT_GFX();

    const int x1 = graphics_current->cursor_x;
    const int y1 = graphics_current->cursor_y;
    const int x2 = graphics_current->width - 1;
    const int y2 = y1 + font_height(graphics_font) - 1;
    return graphics_draw_box(graphics_current, x1, y1, x2, y2, 0, 0, graphics_bcolour);
}

MmResult termgfx_clear_to_end_of_screen() {
    ON_FAILURE_RETURN(termgfx_clear_to_end_of_line());

    const int x1 = 0;
    const int y1 = graphics_current->cursor_y + font_height(graphics_font);
    const int x2 = graphics_current->width - 1;
    const int y2 = graphics_current->height - 1;
    return graphics_draw_box(graphics_current, x1, y1, x2, y2, 0, 0, graphics_bcolour);
}

MmResult termgfx_cls() {
    ASSERT_GFX();
    return graphics_cls(graphics_current, graphics_bcolour);
}

MmResult termgfx_colour(MmGraphicsColour fg, MmGraphicsColour bg) {
    ASSERT_GFX();
    graphics_fcolour = fg;
    graphics_bcolour = bg;
    return kOk;
}

MmResult termgfx_colour_bg(MmGraphicsColour argb) {
    ASSERT_GFX();
    graphics_bcolour = argb;
    return kOk;
}

MmResult termgfx_colour_fg(MmGraphicsColour argb) {
    ASSERT_GFX();
    graphics_fcolour = argb;
    return kOk;
}

MmResult termgfx_cursor_left(int count, bool wrap) {
    ASSERT_GFX();
    MmSurface *s = graphics_current;
    const int fh = (int)font_height(graphics_font);
    const int fw = (int)font_width(graphics_font);
    for (; count > 0; count--) {
        s->cursor_x -= fw;
        if (s->cursor_x < 0) {
            if (wrap) {
                const int width = s->width / fw;
                s->cursor_x = (width - 1) * fw;
                s->cursor_y -= fh ;
                if (s->cursor_y < 0) s->cursor_y = 0;
            } else {
                s->cursor_x = 0;
            }
        }
    }
    return kOk;
}

MmResult termgfx_cursor_up(int count) {
    ASSERT_GFX();
    assert(count > 0);
    graphics_current->cursor_y -= font_height(graphics_font);
    if (graphics_current->cursor_y < 0) graphics_current->cursor_y = 0;
    return kOk;
}

MmResult termgfx_flush() {
    // Currently all graphical terminal output is "flushed".
    return kOk;
}

MmResult termgfx_get_cursor_pos(bool pixel, int *x, int *y) {
    ASSERT_GFX();
    *x = graphics_current->cursor_x;
    *y = graphics_current->cursor_y;
    if (!pixel) {
        *x /= font_width(graphics_font);
        *y /= font_height(graphics_font);
    }
    return kOk;
}

MmResult termgfx_get_size(bool pixel, int *width, int *height) {
    ASSERT_GFX();
    *width = graphics_current->width;
    *height = graphics_current->height;
    if (!pixel) {
        *width /= font_width(graphics_font);
        *height /= font_height(graphics_font);
    }
    return kOk;
}

MmResult termgfx_inverse(bool inverse) {
    ASSERT_GFX();
    self.inverse = inverse;
    return kOk;
}

MmResult termgfx_putc(char c) {
    ASSERT_GFX();

    MmSurface *s = graphics_current;
    const uint32_t font = graphics_font;
    const int fh = (int) font_height(graphics_font);
    const int fw = (int) font_width(graphics_font);

    // If 'c' is printable and it is going to take us off the right hand end of
    // the terminal then print a CRLF.
    if (c >= font_first_char(font) && c <= font_last_char(font)) {
        if (s->cursor_x + fw > s->width) {
            ON_FAILURE_RETURN(termgfx_putc('\r'));
            ON_FAILURE_RETURN(termgfx_putc('\n'));
        }
    }

    // Handle the standard control chars.
    switch(c) {
        case '\b':
            s->cursor_x -= fw;
            // Note that putting the backspace character in the first column
            // DOES NOT move the cursor up and to the end of the next line.
            if (s->cursor_x < 0) s->cursor_x = 0;
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
                ON_FAILURE_RETURN(termgfx_putc(' '));
            } while ((s->cursor_x / fw) % mmb_options.tab);
            break;

        default: {
            const MmGraphicsColour fg = self.inverse ? graphics_bcolour : graphics_fcolour;
            const MmGraphicsColour bg = self.inverse ? graphics_fcolour : graphics_bcolour;
            ON_FAILURE_RETURN(graphics_draw_char(s, &s->cursor_x, &s->cursor_y, graphics_font,
                                                 fg, bg, c, kOrientNormal));
            if (self.underline) {
                const int x = s->cursor_x - font_width(graphics_font);
                const int y = s->cursor_y + font_height(graphics_font) - 2;
                ON_FAILURE_RETURN(graphics_draw_line(s, x, y, s->cursor_x, y, 1, fg));
            }
        }
    }

    return kOk;
}

MmResult termgfx_putc_noflush(char c) {
    // Currently all graphical terminal output is "flushed".
    return termgfx_putc(c);
}

MmResult termgfx_puts(const char *s) {
    ASSERT_GFX();

    if (!s) {
        return mmresult_ex(kInternalFault, "Invalid null parameter: s");
    }

    // TODO: This special non-breaking space handling is to workaround an issue documented in
    //       editor.c#editor_draw_line() where Alacritty does not render underlines for normal
    //       spaces.
    //       However as a consequence we cannot sequentially print the MMBasic graphical characters
    //       0xC2 (box drawing: down and horizontal) and 0xA0 (pause symbol in circle).
    //       Perhaps this should have been handled at the higher-level in editor_draw_line() only.

    while (*s) {
        // Check for UTF-8 non-breaking space (0xC2 0xA0)
        if (*s == 0xC2 && s[1] == 0xA0) {
            // Replace non-breaking space with regular space
            ON_FAILURE_RETURN(termgfx_putc(' '));
            s += 2; // Skip both bytes of the UTF-8 sequence
        } else {
            // Regular character, output as-is
            ON_FAILURE_RETURN(termgfx_putc(*s++));
        }
    }
    return kOk;
}

MmResult termgfx_reset() {
    ASSERT_GFX();
    self.inverse = false;
    self.underline = false;
    // TODO: Reset colours
    return kOk;
}

MmResult termgfx_scroll_down() {
    ASSERT_GFX();
    const int fh = font_height(graphics_font);
    return graphics_scroll(graphics_current, 0, -fh, graphics_bcolour);
}

MmResult termgfx_scroll_up() {
    ASSERT_GFX();
    const int fh = font_height(graphics_font);
    return graphics_scroll(graphics_current, 0, fh, graphics_bcolour);
}


MmResult termgfx_set_cursor_pos(bool pixel, int x, int y) {
    ASSERT_GFX();
    if (!pixel) {
        x *= font_width(graphics_font);
        y *= font_height(graphics_font);
    }
    graphics_current->cursor_x = x;
    graphics_current->cursor_y = y;
    return kOk;
}

MmResult termgfx_show_cursor(bool show) {
    ASSERT_GFX();
    self.show_cursor = show;
    return termgfx_update_cursor();
}

MmResult termgfx_update_cursor() {
    ASSERT_GFX();
    static int64_t t = 0;

    if (!self.show_cursor) {
        if (self.cursor_visible) goto draw_cursor; // Which will actually clear the cursor
        return kOk;
    }

    const int64_t now = mmtime_now_ns();

    if (now < t + CURSOR_PERIOD) {
        return kOk;
    }

    t = now;

draw_cursor:

    self.cursor_visible = !self.cursor_visible;

    const int fh = (int) font_height(graphics_font);
    const int fw = (int) font_width(graphics_font);
    MmSurface *s = graphics_current;
    return graphics_blit(s->cursor_x, s->cursor_y, s->cursor_x, s->cursor_y, fw, fh, s, s,
                         kBlitInvert, -1);
}

MmResult termgfx_underline(bool underline) {
    ASSERT_GFX();
    self.underline = underline;
    return kOk;
}

MmResult termgfx_wrapline() {
    ASSERT_GFX();
    MmSurface *s = graphics_current;
    const int fw = (int) font_width(graphics_font);
    if (s->cursor_x + fw > s->width) {
        ON_FAILURE_RETURN(termgfx_puts("\r\n"));
    }
    return kOk;
}

MmResult termgfx_write(const char *buf, size_t *sz) {
    ASSERT_GFX();
    for (size_t idx = 0; idx < *sz; ++idx) {
        ON_FAILURE_RETURN(termgfx_putc(buf[idx]));
    }
    return kOk;
}
