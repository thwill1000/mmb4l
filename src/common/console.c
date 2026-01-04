/*-*****************************************************************************

MMBasic for Linux (MMB4L)

self.c

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
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "console.h"
#include "error.h"
#include "interrupt.h"
#include "logger.h"
#include "keycodes.h"
#include "mmb4l.h"
#include "mmtime.h"
#include "rx_buf.h"
#include "utility.h"

#define CONSOLE_RX_BUF_SIZE 256

typedef struct {
   int width;
   int height;
   int x;
   int y;
   bool requires_sync;
   bool no_title;
} ConsoleState;

static ConsoleState self;
static struct termios orig_termios;
static char console_rx_buf_data[CONSOLE_RX_BUF_SIZE];
static RxBuf console_rx_buf;

int ListCnt = 0;

static void handle_winch(int sig) {
    self.requires_sync = true;
}

MmResult console_init(bool no_title) {
    // Install signal handler for window size changes.
    struct sigaction sa;
    sa.sa_handler = handle_winch;
    ON_FAILURE_RETURN(sigemptyset(&sa.sa_mask));
    sa.sa_flags = 0;
    ON_FAILURE_RETURN(sigaction(SIGWINCH, &sa, NULL));

    rx_buf_init(
            &console_rx_buf,
            console_rx_buf_data,
            sizeof(console_rx_buf_data));
    self.no_title = no_title;
    self.requires_sync = true;

    return kOk;
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
    // LOG_DEBUG("ENTER: count=%d, wrap=%d, x=%d, y=%d", count, wrap, self.x, self.y);
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

    // LOG_DEBUG("EXIT:  x=%d, y=%d", self.x, self.y);

    return kOk;
}

MmResult console_cursor_up(int count) {
    assert(count > 0);

    if (self.requires_sync) console_sync();
    self.y -= count;
    if (self.y < 0) self.y = 0;

    printf("\033[%d;%dH", self.y + 1, self.x + 1); // VT100 origin is (1,1) not (0,0).
    fflush(stdout);

    // LOG_DEBUG("EXIT:  x=%d y=%d", self.x, self.y);

    return kOk;
}

void console_disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void console_enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    // atexit(console_disable_raw_mode); - done in main.c
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0; // 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    //fcntl(STDIN_FILENO, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
}

void console_pump_input(void) {
    char ch;
    errno = 0;
    ssize_t result = read(STDIN_FILENO, &ch, 1);
    switch (result) {
        case -1:
            error_throw(errno);
        case 0:
            return;
        case 1:
            // Read one character, drop out of the switch.
            // printf("<%d>", (int) ch);
            break;
        default:
            assert(false);
            break;
    }

    console_put_keypress(ch);
}

void console_put_keypress(char ch) {
    // Support for ON KEY ascii_code%, handler_sub().
    // Note that 'ch' does not get added to the buffer.
    if (interrupt_check_key_press(ch)) return;

    if (ch == mmb_options.break_key) {
        // User wishes to stop the program.
        // Set the abort flag so the interpreter will halt and empty the console buffer.
        MMAbort = true;
        rx_buf_clear(&console_rx_buf);
    } else {
        // If the buffer is full then this will throw away ch.
        rx_buf_put(&console_rx_buf, ch);
    }
}

int console_kbhit(void) {
    return rx_buf_size(&console_rx_buf);
}

const int KEY_TO_STRING_MAP_ENTRY_LEN = 11;

char KEY_TO_STRING_MAP[] = {
    0x20,          'S', 'P',  'A',  'C',  'E', '\0', '\0', '\0', '\0', '\0',
    TAB,           'T', 'A',  'B', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    BKSP,          'B', 'K',  'S',  'P', '\0', '\0', '\0', '\0', '\0', '\0',
    ENTER,         'E', 'N',  'T',  'E',  'R', '\0', '\0', '\0', '\0', '\0',
    ESC,           'E', 'S',  'C', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    F1,            'F', '1', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    F2,            'F', '2', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    F3,            'F', '3', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    F4,            'F', '4', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    F5,            'F', '5', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    F6,            'F', '6', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    F7,            'F', '7', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    F8,            'F', '8', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    F9,            'F', '9', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    F10,           'F', '1',  '0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    F11,           'F', '1',  '1', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    F12,           'F', '1',  '2', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    UP,            'U', 'P', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    DOWN,          'D', 'O',  'W',  'N', '\0', '\0', '\0', '\0', '\0', '\0',
    LEFT,          'L', 'E',  'F',  'T', '\0', '\0', '\0', '\0', '\0', '\0',
    RIGHT,         'R', 'I',  'G',  'H',  'T', '\0', '\0', '\0', '\0', '\0',
    INSERT,        'I', 'N',  'S',  'E',  'R',  'T', '\0', '\0', '\0', '\0',
    DEL,           'D', 'E',  'L', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    HOME,          'H', 'O',  'M',  'E', '\0', '\0', '\0', '\0', '\0', '\0',
    END,           'E', 'N',  'D', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    PUP,           'P', 'U',  'P', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    PDOWN,         'P', 'D',  'O',  'W',  'N', '\0', '\0', '\0', '\0', '\0',
    SLOCK,         'S', 'L',  'O',  'C',  'K', '\0', '\0', '\0', '\0', '\0',
    ALT,           'A', 'L',  'T', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    SHIFT_FN(F1),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '1', '\0', '\0',
    SHIFT_FN(F2),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '2', '\0', '\0',
    SHIFT_FN(F3),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '3', '\0', '\0',
    SHIFT_FN(F4),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '4', '\0', '\0',
    SHIFT_FN(F5),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '5', '\0', '\0',
    SHIFT_FN(F6),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '6', '\0', '\0',
    SHIFT_FN(F7),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '7', '\0', '\0',
    SHIFT_FN(F8),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '8', '\0', '\0',
    SHIFT_FN(F9),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '9', '\0', '\0',
    SHIFT_FN(F10), 'S', 'H',  'I',  'F',  'T',  '+',  'F',  '1',  '0', '\0',
    SHIFT_FN(F11), 'S', 'H',  'I',  'F',  'T',  '+',  'F',  '1',  '1', '\0',
    SHIFT_FN(F12), 'S', 'H',  'I',  'F',  'T',  '+',  'F',  '1',  '2', '\0',
    0xFF
};

void console_key_to_string(int ch, char *buf) {
    char *p = KEY_TO_STRING_MAP;
    while (*p != 0xFF) {
        if (*p == ch) {
            sprintf(buf, "[%s]", p + 1);
            return;
        }
        p += KEY_TO_STRING_MAP_ENTRY_LEN;
    }
    sprintf(buf, "'%c'", ch);
}

void console_ungetc(char ch) {
    rx_buf_unget(&console_rx_buf, ch);
}

int console_match_chars(char *pattern) {
    if (*pattern == '\0') return 1;

    if (rx_buf_size(&console_rx_buf) == 0) {
        perform_background_tasks(); // Which calls console_pump_input();
    }

    int ch = rx_buf_get(&console_rx_buf);
    if (ch == -1) {
        return 0;
    } else if (ch == *pattern && console_match_chars(++pattern)) {
        return 1;
    } else {
        console_ungetc(ch);
        return 0;
    }
}

const int ESCAPE_MAP_ENTRY_LEN = 8;

static char ESCAPE_MAP[] = {
         'O',   'P', '\0', '\0', '\0', '\0', '\0', F1,
         'O',   'Q', '\0', '\0', '\0', '\0', '\0', F2,
         'O',   'R', '\0', '\0', '\0', '\0', '\0', F3,
         'O',   'S', '\0', '\0', '\0', '\0', '\0', F4,
         '[',   '1',  '5',  '~', '\0', '\0', '\0', F5,
         '[',   '1',  '7',  '~', '\0', '\0', '\0', F6,
         '[',   '1',  '8',  '~', '\0', '\0', '\0', F7,
         '[',   '1',  '9',  '~', '\0', '\0', '\0', F8,
         '[',   '2',  '0',  '~', '\0', '\0', '\0', F9,
         '[',   '2',  '1',  '~', '\0', '\0', '\0', F10,  // F10 - is captured by the Gnome WM
         '[',   '2',  '3',  '~', '\0', '\0', '\0', F11,  // F11 - is captured by the Gnome WM
         '[',   '2',  '4',  '~', '\0', '\0', '\0', F12,
         '[',   '2',  '~', '\0', '\0', '\0', '\0', INSERT,
         '[',   '3',  '~', '\0', '\0', '\0', '\0', DEL,
         '[',   '5',  '~', '\0', '\0', '\0', '\0', PUP,
         '[',   '6',  '~', '\0', '\0', '\0', '\0', PDOWN,
         '[',   'A', '\0', '\0', '\0', '\0', '\0', UP,
         '[',   'B', '\0', '\0', '\0', '\0', '\0', DOWN,
         '[',   'C', '\0', '\0', '\0', '\0', '\0', RIGHT,
         '[',   'D', '\0', '\0', '\0', '\0', '\0', LEFT,
         '[',   'F', '\0', '\0', '\0', '\0', '\0', END,
         '[',   'H', '\0', '\0', '\0', '\0', '\0', HOME,
         '[',   '1',  ';',  '2',  'P', '\0', '\0', SHIFT_FN(F1),
         '[',   '1',  ';',  '2',  'Q', '\0', '\0', SHIFT_FN(F2),
         '[',   '1',  ';',  '2',  'R', '\0', '\0', SHIFT_FN(F3),
         '[',   '1',  ';',  '2',  'S', '\0', '\0', SHIFT_FN(F4),
         '[',   '1',  '5',  ';',  '2',  '~', '\0', SHIFT_FN(F5),
         '[',   '1',  '7',  ';',  '2',  '~', '\0', SHIFT_FN(F6),
         '[',   '1',  '8',  ';',  '2',  '~', '\0', SHIFT_FN(F7),
         '[',   '1',  '9',  ';',  '2',  '~', '\0', SHIFT_FN(F8),
         '[',   '2',  '0',  ';',  '2',  '~', '\0', SHIFT_FN(F9),
         '[',   '2',  '1',  ';',  '2',  '~', '\0', SHIFT_FN(F10),
         '[',   '2',  '3',  ';',  '2',  '~', '\0', SHIFT_FN(F11),
         '[',   '2',  '4',  ';',  '2',  '~', '\0', SHIFT_FN(F12),
         0xFF };

int console_getc(void) {

    perform_background_tasks(); // Which calls console_pump_input();
    int ch = rx_buf_get(&console_rx_buf);

    switch (ch) {
        // case 0x0A:
        //     ch = ENTER;
        //     break;

        case ESC: {
            char *p = ESCAPE_MAP;
            while (*p != 0xFF) {
                if (console_match_chars(p)) {
                    ch = *(p + ESCAPE_MAP_ENTRY_LEN - 1);
                    break;
                }
                p += ESCAPE_MAP_ENTRY_LEN;
            }
            break;
        }

        case DEL:
            // As the result of a historical quirk of terminals:
            //  - the [Backspace] key sends the ASCII code for "Delete" (0x7F)
            //  - the [Delete] keys sends the escape sequence \x1b[3~ which will
            //    be handled by the 'case ESC:' clause above.
            ch = BKSP;
            break;

        default:
            break;
    }

    return ch;
}

char console_putc_noflush(char c) {
    bool printable = false; // Is 'c' a printable character?

    if (mmb_options.codepage && c > 127) {
        const char *ptr = mmb_options.codepage + 4 * (c - 128);
        putc(*ptr++, stdout);           // 1st byte.
        if (ptr) putc(*ptr++, stdout);  // Optional 2nd byte.
        if (ptr) putc(*ptr++, stdout);  // Optional 3rd byte.
        if (ptr) putc(*ptr++, stdout);  // Optional 4th byte.
        printable = true;
    } else {
        putc(c, stdout);
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

    // LOG_DEBUG("EXIT:  c='%c'(0x%2x), x=%d, y=%d", printable ? c : '?', c, self.x, self.y);

    return c;
}

char console_putc(char c) {
    char rval = console_putc_noflush(c);
    fflush(stdout);
    return rval;
}

void console_puts(const char *s) {
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

static MmResult console_sync_size(int timeout_ms) {
    static int safe_width = 80;
    static int safe_height = 40;
    struct winsize ws= { 0 };
    int fd = open("/dev/tty", O_RDWR);
    if (fd >= 0) {
        int64_t timeout_ns = mmtime_now_ns() + MILLISECONDS_TO_NANOSECONDS(timeout_ms);
        do {
            // Alternatively consider: ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)
            if (SUCCEEDED(ioctl(fd, TIOCGWINSZ, &ws)) && ws.ws_col > 0) break;
            nanosleep(&ONE_MICROSECOND, NULL);
        } while (mmtime_now_ns() < timeout_ns);
        close(fd);
    }

    if (ws.ws_col > 0) {
        // Success.
        safe_width = ws.ws_col;
        safe_height = ws.ws_row;
    }

    // NOTE: Previously when the console size could not be determined this
    //       function would return a failure and "all hell would break loose" with
    //       endless "Cannot determine terminal size" errors being reported.
    //       Now we return the last successful values determined, or 80x40.
    self.width = safe_width;
    self.height = safe_height;

    return kOk;
}

enum ReadCursorPositionState {
    EXPECTING_ESCAPE,
    EXPECTING_SQUARE_BRACKET,
    EXPECTING_ROWS,
    EXPECTING_COLS,
    EXPECTING_FINISHED
};

MmResult console_sync_cursor_pos(int timeout_ms) {
    // Send escape code to report cursor position.
    rx_buf_clear(&console_rx_buf);
    printf("\033[6n");
    fflush(stdout);

    // Read characters one at a time to match the expected pattern ESC[n;mR
    // - fails if the pattern has not been matched within the timeout.
    // - will sleep briefly if there is nothing to read.
    int64_t timeout_ns = mmtime_now_ns() + MILLISECONDS_TO_NANOSECONDS(timeout_ms);
    enum ReadCursorPositionState state = EXPECTING_ESCAPE;
    char buf[32] = { 0 };
    char *p = NULL;
    while (mmtime_now_ns() < timeout_ns && state != EXPECTING_FINISHED) {
        if (state == EXPECTING_ESCAPE) p = buf;
        int ch = console_getc();
        if (ch == -1) {
            nanosleep(&ONE_MICROSECOND, NULL);
            continue;
        }
        *(p++) = (char) ch;

        switch (state) {
            case EXPECTING_ESCAPE:
                state = (ch == 0x1B ? EXPECTING_SQUARE_BRACKET : EXPECTING_ESCAPE);
                break;
            case EXPECTING_SQUARE_BRACKET:
                state = (ch == '[' ? EXPECTING_ROWS : EXPECTING_ESCAPE);
                break;
            case EXPECTING_ROWS:
                state = (ch == ';'
                        ? EXPECTING_COLS
                        : (isdigit(ch) ? EXPECTING_ROWS : EXPECTING_ESCAPE));
                break;
            case EXPECTING_COLS:
                state = (ch == 'R'
                        ? EXPECTING_FINISHED
                        : (isdigit(ch) ? EXPECTING_COLS : EXPECTING_ESCAPE));
                break;
            case EXPECTING_FINISHED:
                assert(0);  // Loop should have already exited.
                break;
        }
    }

    if (state == EXPECTING_FINISHED) {
        // Parse output, rows (y) then columns (x).
        *p++ = '\0';
        sscanf(buf, "\033[%d;%dR", &self.y, &self.x);
        self.x--; // adjust to account for VT100 origin being (1,1) not (0,0).
        self.y--;
        return kOk; // Success
    } else {
        return mmresult_ex(kError, "Failed to read TTY cursor position");
    }
}

MmResult console_sync() {
    ON_FAILURE_RETURN(console_sync_size(100));
    ON_FAILURE_RETURN(console_sync_cursor_pos(10000));
    self.requires_sync = false;
    // LOG_DEBUG("EXIT:  width=%d, height=%d, x=%d, y=%d", self.width, self.height, self.x, self.y);
    return kOk;
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
    if (self.requires_sync) {
        ON_FAILURE_RETURN(console_sync());
    }
    if (self.x >= self.width) {
        console_puts("\r\n");
    }
    // LOG_DEBUG("EXIT:  x=%d, y=%d", self.x, self.y);
    return kOk;
}

size_t console_write(const char *buf, size_t sz) {
    for (size_t idx = 0; idx < sz; ++idx) {
        console_putc_noflush(buf[idx]);
    }
    fflush(stdout);
    return sz;
}
