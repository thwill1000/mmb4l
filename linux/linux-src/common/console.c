// Copyright (c) 2021 Thomas Hugo Williams

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>

#include "console.h"
#include "error.h"
#include "global_aliases.h"
#include "utility.h"
#include "rx_buf.h"
#include "../Configuration.h" // For STRINGSIZE

void CheckAbort(void);

extern volatile int MMAbort;
extern char g_break_key;
extern int g_key_complete;
extern char *g_key_interrupt;
extern int g_key_select;

#define CONSOLE_RX_BUF_SIZE 256

// Jump through hoops so compiler doesn't complain about ignoring the return value.
#define WRITE_CODE(s)         (void)(write(STDOUT_FILENO, s, strlen(s)) + 1)
#define WRITE_CODE_2(s, len)  (void)(write(STDOUT_FILENO, s, len) + 1)

static struct termios orig_termios;
static char console_rx_buf_data[CONSOLE_RX_BUF_SIZE];
static RxBuf console_rx_buf;

void console_init(void) {
    rx_buf_init(
            &console_rx_buf,
            console_rx_buf_data,
            sizeof(console_rx_buf_data));
}

void console_bell(void) {
    WRITE_CODE_2("\07", 1);
}

void console_clear(void) {
    WRITE_CODE_2("\033[2J", 4); // Clear screen.
    console_home_cursor();
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
    ssize_t result = read(STDIN_FILENO, &ch, 1);
    switch (result) {
        case 0:
            // Nothing to read.
            return;
        case 1:
            // Read one character, drop out of the switch.
            break;
        default:
            error("Unexpected result from read()");
            break;
    }

    // Support for ON KEY ascii_code%, handler_sub().
    // Note that 'ch' does not get added to the buffer.
    if (ch == g_key_select && g_key_interrupt != NULL) {
        g_key_complete = 1;
        return;
    }

    if (ch == g_break_key) {
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
        case 0x0A:
            ch = ENTER;
            break;

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

void console_set_title(const char *title) {
    char buf[256];
    sprintf(buf, "\x1b]0;%s\x7", title);
    WRITE_CODE(buf);
}

enum ReadCursorPositionState {
        EXPECTING_ESCAPE,
        EXPECTING_SQUARE_BRACKET,
        EXPECTING_ROWS,
        EXPECTING_COLS,
        EXPECTING_FINISHED };

int console_get_cursor_pos(int *x, int *y) {
    rx_buf_clear(&console_rx_buf);

    // Send escape code to report cursor position.
    WRITE_CODE_2("\033[6n", 4);

    // Read characters one at a time to match the expected pattern ESC[n;mR
    // - fails if after 500 attempted reads the pattern has not been matched.
    // - will sleep briefly if there is nothing to read.
    int ch;
    enum ReadCursorPositionState state = EXPECTING_ESCAPE;
    char buf[32] = { 0 };
    char *p = buf;
    for (int count = 0; count < 500 && state != EXPECTING_FINISHED; ++count) {
        if (state == EXPECTING_ESCAPE) p = buf;
        ch = console_getc();
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
        }
    }

    if (state == EXPECTING_FINISHED) {
        // Parse output, rows (y) then columns (x).
        *p++ = '\0';
        sscanf(buf, "\033[%d;%dR", y, x);
        (*x)--; // adjust to account for VT100 origin being (1,1) not (0,0).
        (*y)--;
        return 1;
    } else {
        *x = 0;
        *y = 0;
        return 0;
    }
}

int console_get_size(int *width, int *height) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return 0;
    } else {
        *width = ws.ws_col;
        *height = ws.ws_row;
        return 1;
    }
}

void console_home_cursor(void) {
    WRITE_CODE_2("\x1b[H", 4);
}

void console_set_cursor_pos(int x, int y) {
    char buf[STRINGSIZE];
    sprintf(buf, "\033[%d;%dH", y + 1, x + 1); // VT100 origin is (1,1) not (0,0).
    WRITE_CODE(buf);
}

const int ANSI_COLOURS[] = { 0, 4, 2, 6, 1, 5, 3, 7, 10, 14, 12, 16, 11, 15, 13, 17 };

void console_background(int colour) {
    char buf[STRINGSIZE];
    int ansi_colour = ANSI_COLOURS[colour];
    sprintf(buf, "\033[%dm", ansi_colour + (ansi_colour < 10 ? 40 : 90));
    WRITE_CODE(buf);
}

void console_foreground(int colour) {
    char buf[STRINGSIZE];
    int ansi_colour = ANSI_COLOURS[colour];
    sprintf(buf, "\033[%dm", ansi_colour + (ansi_colour < 10 ? 30 : 80));
    WRITE_CODE(buf);
}

void console_invert(int invert) {
    if (invert) {
        WRITE_CODE_2("\033[7m", 4);
    } else {
        WRITE_CODE_2("\033[27m", 5);
    }
}

void console_reset() {
    WRITE_CODE_2("\033[0m", 4);
}

void console_show_cursor(int show) {
    if (show) {
        WRITE_CODE_2("\033[?25h", 6);
    } else {
        WRITE_CODE_2("\033[?25l", 6);
    }
}
