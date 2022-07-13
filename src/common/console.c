/*-*****************************************************************************

MMBasic for Linux (MMB4L)

console.c

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
#include <unistd.h>
#include <sys/ioctl.h>

#include "mmb4l.h"
#include "console.h"
#include "error.h"
#include "interrupt.h"
#include "mmtime.h"
#include "utility.h"
#include "rx_buf.h"

#define CONSOLE_RX_BUF_SIZE 256

static struct termios orig_termios;
static char console_rx_buf_data[CONSOLE_RX_BUF_SIZE];
static RxBuf console_rx_buf;

int ListCnt = 0;
int MMCharPos = 0;

void console_init(void) {
    rx_buf_init(
            &console_rx_buf,
            console_rx_buf_data,
            sizeof(console_rx_buf_data));
}

void console_bell(void) {
    printf("\07");
    fflush(stdout);
}

void console_clear(void) {
    printf("\033[2J");      // Clear screen.
    console_home_cursor();  // Which will also call fflush().
}

void console_cursor_up(int i) {
    assert(i > 0);
    printf("\033[%dA", i);
    fflush(stdout);
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
    return;
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

    // Support for ON KEY ascii_code%, handler_sub().
    // Note that 'ch' does not get added to the buffer.
    if (interrupt_check_key_press(ch)) return;

    if (ch == mmb_options.break_key) {
        // User wishes to stop the program.
        // Set the abort flag so the interpreter will halt and empty the console buffer.
        MMAbort = 1;
        rx_buf_clear(&console_rx_buf);
    } else {
        // If the buffer is full then this will throw away ch.
        rx_buf_put(&console_rx_buf, ch);
    }
}

int console_kbhit(void) {
    return rx_buf_size(&console_rx_buf);
}

char KEY_TO_STRING_MAP[] = {
    0x20,   'S', 'P',  'A',  'C',  'E', '\0', '\0',
    TAB,    'T', 'A',  'B', '\0', '\0', '\0', '\0',
    BKSP,   'B', 'K',  'S',  'P', '\0', '\0', '\0',
    ENTER,  'E', 'N',  'T',  'E',  'R', '\0', '\0',
    ESC,    'E', 'S',  'C', '\0', '\0', '\0', '\0',
    F1,     'F', '1', '\0', '\0', '\0', '\0', '\0',
    F2,     'F', '2', '\0', '\0', '\0', '\0', '\0',
    F3,     'F', '3', '\0', '\0', '\0', '\0', '\0',
    F4,     'F', '4', '\0', '\0', '\0', '\0', '\0',
    F5,     'F', '5', '\0', '\0', '\0', '\0', '\0',
    F6,     'F', '6', '\0', '\0', '\0', '\0', '\0',
    F7,     'F', '7', '\0', '\0', '\0', '\0', '\0',
    F8,     'F', '8', '\0', '\0', '\0', '\0', '\0',
    F9,     'F', '9', '\0', '\0', '\0', '\0', '\0',
    F10,    'F', '1',  '0', '\0', '\0', '\0', '\0',
    F11,    'F', '1',  '1', '\0', '\0', '\0', '\0',
    F12,    'F', '1',  '2', '\0', '\0', '\0', '\0',
    UP,     'U', 'P', '\0', '\0', '\0', '\0', '\0',
    DOWN,   'D', 'O',  'W',  'N', '\0', '\0', '\0',
    LEFT,   'L', 'E',  'F',  'T', '\0', '\0', '\0',
    RIGHT,  'R', 'I',  'G',  'H',  'T', '\0', '\0',
    INSERT, 'I', 'N',  'S',  'E',  'R',  'T', '\0',
    DEL,    'D', 'E',  'L', '\0', '\0', '\0', '\0',
    HOME,   'H', 'O',  'M',  'E', '\0', '\0', '\0',
    END,    'E', 'N',  'D', '\0', '\0', '\0', '\0',
    PUP,    'P', 'U',  'P', '\0', '\0', '\0', '\0',
    PDOWN,  'P', 'D',  'O',  'W',  'N', '\0', '\0',
    SLOCK,  'S', 'L',  'O',  'C',  'K', '\0', '\0',
    ALT,    'A', 'L',  'T', '\0', '\0', '\0', '\0',
    0xFF
};

void console_key_to_string(int ch, char *buf) {
    char *p = KEY_TO_STRING_MAP;
    while (*p != 0xFF) {
        if (*p == ch) {
            sprintf(buf, "[%s]", p + 1);
            return;
        }
        p += 8;
    }
    sprintf(buf, "'%c'", ch);
}

void console_ungetc(char ch) {
    rx_buf_unget(&console_rx_buf, ch);
}

int console_match_chars(char *pattern) {
    if (*pattern == '\0') return 1;

    if (rx_buf_size(&console_rx_buf) == 0) CheckAbort(); // Which calls console_pump_input();

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

static char ESCAPE_MAP[] = {
         'O',   'P', '\0', '\0', '\0', F1,
         'O',   'Q', '\0', '\0', '\0', F2,
         'O',   'R', '\0', '\0', '\0', F3,
         'O',   'S', '\0', '\0', '\0', F4,
         '[',   '1',  '5',  '~', '\0', F5,
         '[',   '1',  '7',  '~', '\0', F6,
         '[',   '1',  '8',  '~', '\0', F7,
         '[',   '1',  '9',  '~', '\0', F8,
         '[',   '2',  '0',  '~', '\0', F9,
         // F10 - is captured by the Gnome WM
         // F11 - is captured by the Gnome WM
         '[',   '2',  '4',  '~', '\0', F12,
         '[',   '2',  '~', '\0', '\0', INSERT,
         '[',   '3',  '~', '\0', '\0', DEL,
         '[',   '5',  '~', '\0', '\0', PUP,
         '[',   '6',  '~', '\0', '\0', PDOWN,
         '[',   'A', '\0', '\0', '\0', UP,
         '[',   'B', '\0', '\0', '\0', DOWN,
         '[',   'C', '\0', '\0', '\0', RIGHT,
         '[',   'D', '\0', '\0', '\0', LEFT,
         '[',   'F', '\0', '\0', '\0', END,
         '[',   'H', '\0', '\0', '\0', HOME,
         0xFF };

int console_getc(void) {

    CheckAbort(); // Which calls console_pump_input();
    int ch = rx_buf_get(&console_rx_buf);

    switch (ch) {
        // case 0x0A:
        //     ch = ENTER;
        //     break;

        case ESC: {
            char *p = ESCAPE_MAP;
            while (*p != 0xFF) {
                if (console_match_chars(p)) {
                    ch = *(p + 5);
                    break;
                }
                p += 6;
            }
            break;
        }

        case DEL:
            ch = '\b';
            break;

        default:
            break;
    }

    return ch;
}

static char console_putc_noflush(char c) {
    if (mmb_options.codepage && c > 127) {
        const char *ptr = mmb_options.codepage + 4 * (c - 128);
        putc(*ptr++, stdout);           // 1st byte.
        if (ptr) putc(*ptr++, stdout);  // Optional 2nd byte.
        if (ptr) putc(*ptr++, stdout);  // Optional 3rd byte.
        if (ptr) putc(*ptr++, stdout);  // Optional 4th byte.
        MMCharPos++;
    } else {
        putc(c, stdout);
        if (isprint(c))
            MMCharPos++;
        else {
            switch (c) {
                case '\b':
                    MMCharPos--;
                    break;
                case '\r':
                case '\n':
                    MMCharPos = 1;
                    ListCnt++;
                    break;
                default:
                    break;
            }
        }
    }
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

void console_set_title(const char *title) {
    printf("\x1b]0;%s\x7", title);
    fflush(stdout);
}

enum ReadCursorPositionState {
        EXPECTING_ESCAPE,
        EXPECTING_SQUARE_BRACKET,
        EXPECTING_ROWS,
        EXPECTING_COLS,
        EXPECTING_FINISHED };

int console_get_cursor_pos(int *x, int *y, int timeout_ms) {

    rx_buf_clear(&console_rx_buf);

    // Send escape code to report cursor position.
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
        sscanf(buf, "\033[%d;%dR", y, x);
        (*x)--; // adjust to account for VT100 origin being (1,1) not (0,0).
        (*y)--;
        return 0; // Success
    } else {
        *x = 0;
        *y = 0;
        return -1; // Failure
    }
}

int console_get_size(int *width, int *height) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1; // Failure
    }

    *width = ws.ws_col;
    *height = ws.ws_row;
    return 0; // Success
}

void console_home_cursor(void) {
    printf("\x1b[H");
    fflush(stdout);
}

void console_set_cursor_pos(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1); // VT100 origin is (1,1) not (0,0).
    fflush(stdout);
}

int console_set_size(int width, int height) {
    printf("\033[8;%d;%dt", height, width);
    fflush(stdout);

    // Wait 250ms for the change to take effect.
    // Note that if the requested height and width are not possible (e.g. too big)
    // then console_get_size() can still briefly return the requested value even
    // if it does not represent reality.
    mmtime_sleep_ns(MILLISECONDS_TO_NANOSECONDS(250));

    int new_height = 0;
    int new_width = 0;
    if (SUCCEEDED(console_get_size(&new_width, &new_height))
            && (new_width == width)
            && (new_height == height)) return 0; // Success

    return -1; // Failure
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

void console_invert(int invert) {
    printf(invert ? "\033[7m" : "\033[27m");
    fflush(stdout);
}

void console_reset() {
    printf("\033[0m");
    fflush(stdout);
}

void console_show_cursor(bool show) {
    printf(show ? "\033[?25h" : "\033[?25l");
    fflush(stdout);
}

size_t console_write(const char *buf, size_t sz) {
    for (size_t idx = 0; idx < sz; ++idx) {
        console_putc_noflush(buf[idx]);
    }
    fflush(stdout);
    return sz;
}
