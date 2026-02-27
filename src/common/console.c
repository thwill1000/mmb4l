/*-*****************************************************************************

MMBasic for Linux (MMB4L)

console.c

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

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "console.h"
#include "console_private.h"
#include "error.h"
#include "interrupt.h"
#include "keybuf.h"
#include "logger.h"
#include "keycodes.h"
#include "mmb4l.h"
#include "mmtime.h"
#include "utility.h"

static ConsoleState self;

int ListCnt = 0;

MmResult console_init(bool no_title) {
    LOG_FN_ENTRY();

    self.no_title = no_title;
    self.requires_sync = true;

    RETURN_RESULT(console_init_platform(&self));
}

MmResult console_term(void) {
    LOG_FN_ENTRY();

    RETURN_RESULT(console_term_platform());
}

void console_bell(void) {
    printf("\07");
    fflush(stdout);
}

void console_clear(void) {
    printf("\033[2J");      // Clear screen.
    console_home_cursor();  // Which will also call fflush().
}

MmResult console_cursor_left(int count, bool wrap) {
    // LOG_FN_ENTRY("count=%d, wrap=%d, self.x=%d, self.y=%d", count, wrap, self.x, self.y);
    assert(count > 0);

    if (self.requires_sync) console_sync();
    for (; count > 0; count--) {
        self.x--;
        if (self.x < 0) {
            if (wrap) {
                self.x = self.width - 1;
                self.y--;
                if (self.y < 0) self.y = 0;
            } else {
                self.x = 0;
            }
        }
    }

    printf("\033[%d;%dH", self.y + 1, self.x + 1); // VT100 origin is (1,1) not (0,0).
    fflush(stdout);

    RETURN_RESULT_EX(kOk, "self.x=%d, self.y=%d", self.x, self.y);
}

MmResult console_cursor_up(int count) {
    // LOG_FN_ENTRY("count=%d", count);
    assert(count > 0);

    if (self.requires_sync) console_sync();
    self.y -= count;
    if (self.y < 0) self.y = 0;

    printf("\033[%d;%dH", self.y + 1, self.x + 1); // VT100 origin is (1,1) not (0,0).
    fflush(stdout);

    RETURN_RESULT_EX(kOk, "self.x=%d, self.y=%d", self.x, self.y);
}

char console_putc_noflush(char c) {
    // LOG_FN_ENTRY("c='%c'", c);
    bool printable = false; // Is 'c' a printable character?

    if (mmb_options.codepage && c > 127) {
        const char *ptr = mmb_options.codepage + 4 * (c - 128);
        // Count how many bytes to write (up to 4, stopping at '\0')
        int count = 0;
        while (count < 4 && ptr[count]) count++;
        if (count > 0) console_putc_raw_n(ptr, count);
        printable = true;
    } else {
        console_putc_raw(c);
        if (isprint(c)) {
            printable = true;
        } else {
            switch (c) {
                case '\b':
                    if (self.x > 0) self.x--;
                    break;
                case '\r':
                    self.x = 0;
                    break;
                case '\n':
                    self.y++;
                    ListCnt++;
                    break;
                default:
                    break;
            }
        }
    }

    if (printable) {
        if (self.x >= self.width) {
            // Handle "pending wrap".
            self.x = 0;
            self.y++;
        }
        self.x++;
        // If x == self.width we have a "pending wrap".
    }

    if (self.y >= self.height) {
        self.y = self.height - 1;
    }

    // LOG_FN_EXIT("c='%c'(0x%2x), self.x=%d, self.y=%d", printable ? c : '?', c, self.x, self.y);

    return c;
}

char console_putc(char c) {
    // LOG_DEBUG("STDOUT: %c", c);
    char rval = console_putc_noflush(c);
    fflush(stdout);
    return rval;
}

void console_puts(const char *s) {
    // LOG_FN_ENTRY("s=\"%s\"", s);
    while (*s) (void) console_putc_noflush(*s++);
    fflush(stdout);
}

void console_set_title(const char *title, bool command) {
    if (!command && self.no_title) return;
    printf("\x1b]0;%s\x7", title);
    fflush(stdout);
}

MmResult console_get_cursor_pos(int *x, int *y) {
    if (self.requires_sync) ON_FAILURE_RETURN(console_sync());
    *x = self.x;
    *y = self.y;
    return kOk;
}

MmResult console_get_size(int *width, int *height) {
    if (self.requires_sync) ON_FAILURE_RETURN(console_sync());
    *width = self.width;
    *height = self.height;
    return kOk;
}

void console_home_cursor(void) {
    printf("\033[H");
    fflush(stdout);
    self.x = 0;
    self.y = 0;
}

void console_set_cursor_pos(int x, int y) {
    if (x < 0) {
        x = 0;
    } else if (x >= self.width) {
        x = self.width - 1;
    }

    if (y < 0) {
        y = 0;
    } else if (y >= self.height) {
        y = self.height - 1;
    }

    printf("\033[%d;%dH", y + 1, x + 1); // VT100 origin is (1,1) not (0,0).
    fflush(stdout);

    self.x = x;
    self.y = y;
}

MmResult console_set_size(int width, int height) {
    printf("\033[8;%d;%dt", height, width);
    fflush(stdout);

    // Wait 250ms for the change to take effect.
    // Note that if the requested height and width are not possible (e.g. too big)
    // then console_get_size() can still briefly return the requested value even
    // if it does not represent reality.
    mmtime_sleep_ns(MILLISECONDS_TO_NANOSECONDS(250));

    ON_FAILURE_RETURN(console_sync());
    if (self.width == width && self.height == height) {
        return kOk;
    } else {
        return mmresult_ex(kError, "Failed to set TTY size");
    }
}

const int ANSI_COLOURS[] = { 0, 4, 2, 6, 1, 5, 3, 7, 10, 14, 12, 16, 11, 15, 13, 17 };

void console_background(int colour) {
    int ansi_colour = ANSI_COLOURS[colour];
    printf("\033[%dm", ansi_colour + (ansi_colour < 10 ? 40 : 90));
    fflush(stdout);
}

void console_foreground(int colour) {
    int ansi_colour = ANSI_COLOURS[colour];
    printf("\033[%dm", ansi_colour + (ansi_colour < 10 ? 30 : 80));
    fflush(stdout);
}

MmResult console_clear_to_end_of_line() {
    printf("\033[K");
    fflush(stdout);
    return kOk;
}

MmResult console_clear_to_end_of_screen() {
    printf("\033[J");
    fflush(stdout);
    return kOk;
}

static int argb_to_ansi(MmGraphicsColour argb) {
    switch (argb) {
        case RGB_ANSI_DEFAULT:        return 39;
        case RGB_ANSI_BLACK:          return 30;
        case RGB_ANSI_RED:            return 31;
        case RGB_ANSI_GREEN:          return 32;
        case RGB_ANSI_YELLOW:         return 33;
        case RGB_ANSI_BLUE:           return 34;
        case RGB_ANSI_MAGENTA:        return 35;
        case RGB_ANSI_CYAN:           return 36;
        case RGB_ANSI_WHITE:          return 37;
        case RGB_ANSI_BRIGHT_BLACK:   return 90;
        case RGB_ANSI_BRIGHT_RED:     return 91;
        case RGB_ANSI_BRIGHT_GREEN:   return 92;
        case RGB_ANSI_BRIGHT_YELLOW:  return 93;
        case RGB_ANSI_BRIGHT_BLUE:    return 94;
        case RGB_ANSI_BRIGHT_MAGENTA: return 95;
        case RGB_ANSI_BRIGHT_CYAN:    return 96;
        case RGB_ANSI_BRIGHT_WHITE:   return 97;
        default:                      return -1;
    }
}

MmResult console_colour(MmGraphicsColour fg, MmGraphicsColour bg) {
    const int ansi_fg = argb_to_ansi(fg);
    const int ansi_bg = argb_to_ansi(bg);
    if (ansi_fg == -1 || ansi_bg == -1) return kUnsupportedTerminalColour;
    printf("\033[%d;%dm", ansi_fg, ansi_bg + 10);
    fflush(stdout);
    return kOk;
}

MmResult console_colour_bg(MmGraphicsColour argb) {
    const int ansi_colour = argb_to_ansi(argb);
    if (ansi_colour == -1) return kUnsupportedTerminalColour;
    printf("\033[%dm", ansi_colour + 10);
    fflush(stdout);
    return kOk;
}

MmResult console_colour_fg(MmGraphicsColour argb) {
    const int ansi_colour = argb_to_ansi(argb);
    if (ansi_colour == -1) return kUnsupportedTerminalColour;
    printf("\033[%dm", ansi_colour);
    fflush(stdout);
    return kOk;
}

MmResult console_flush() {
    fflush(stdout);
    return kOk;
}

MmResult console_inverse(bool inverse) {
    printf(inverse ? "\033[7m" : "\033[27m");
    fflush(stdout);
    return kOk;
}

MmResult console_reset() {
    printf("\033[0m");
    fflush(stdout);
    return kOk;
}

MmResult console_scroll_down() {
    printf("\033[1T");
    fflush(stdout);
    return kOk;
}

MmResult console_scroll_up() {
    printf("\033[1S");
    fflush(stdout);
    return kOk;
}

MmResult console_show_cursor(bool show) {
    printf(show ? "\033[?25h" : "\033[?25l");
    fflush(stdout);
    return kOk;
}

// TODO
MmResult console_sync_cursor_pos(int timeout_ms);

MmResult console_sync() {
    // LOG_FN_ENTRY();
    ON_FAILURE_RETURN(console_sync_size(100));
    ON_FAILURE_RETURN(console_sync_cursor_pos(10000));
    self.requires_sync = false;
    RETURN_RESULT_EX(kOk, "self.width=%d, self.height=%d, self.x=%d, self.y=%d", self.width,
                     self.height, self.x, self.y);
}

MmResult console_underline(bool underline) {
    if (underline) {
        printf("\033[4m"); // Enable underline.
    } else {
        printf("\033[24m"); // Disable underline.
    }
    fflush(stdout);
    return kOk;
}

MmResult console_wrapline() {
    LOG_FN_ENTRY();
    LOG_DEBUG("self.requires_sync=%d, self.x=%d, self.y=%d, self.width=%d, self.height=%d", self.requires_sync, self.x, self.y, self.width, self.height);
    if (self.requires_sync) {
        ON_FAILURE_RETURN(console_sync());
    }
    if (self.x >= self.width) {
        console_puts("\r\n");
    }
    RETURN_RESULT_EX(kOk, "self.x=%d, self.y=%d", self.x, self.y);
}

size_t console_write(const char *buf, size_t sz) {
    for (size_t idx = 0; idx < sz; ++idx) {
        console_putc_noflush(buf[idx]);
    }
    fflush(stdout);
    return sz;
}
