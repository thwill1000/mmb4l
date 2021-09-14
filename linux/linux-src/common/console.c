// Copyright (c) 2021 Thomas Hugo Williams

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>

#include "console.h"
#include "global_aliases.h"
#include "utility.h"
#include "rx_buf.h"
#include "version.h"

void CheckAbort(void);

extern volatile int MMAbort;
extern char g_break_key;
extern int g_key_complete;
extern char *g_key_interrupt;
extern int g_key_select;

#define CONSOLE_RX_BUF_SIZE 256
#define WRITE_CODE(s)         write(STDOUT_FILENO, s, strlen(s))
#define WRITE_CODE_2(s, len)  write(STDOUT_FILENO, s, len)

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

void console_buffer_input(void) {
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
        g_key_complete = true;
        return;
    }

    if (ch == g_break_key) {
        // User wishes to stop the program.
        // Set the abort flag so the interpreter will halt and empty the console buffer.
        MMAbort = true;
        rx_buf_clear(&console_rx_buf);
    } else {
        // If the buffer is full then this will throw away ch.
        rx_buf_put(&console_rx_buf, ch);
    }
}

// get a char from the console input queue
// will return immediately with -1 if there is no character waiting
int console_get_buffered_char(void) {
    console_buffer_input();
    CheckAbort(/*0*/);
    return rx_buf_get(&console_rx_buf);
}

int console_kbhit(void) {
    return rx_buf_size(&console_rx_buf);
}

void console_key_to_string(int ch, char *buf) {
    char *key = NULL;
    switch (ch) {
        case 0x20:
            key = "SPACE";
            break;
        case TAB:
            key = "TAB";
            break;
        case BKSP:
            key = "BKSP";
            break;
        case ENTER:
            key = "ENTER";
            break;
        case ESC:
            key = "ESC";
            break;
        case F1:
            key = "F1";
            break;
        case F2:
            key = "F2";
            break;
        case F3:
            key = "F3";
            break;
        case F4:
            key = "F4";
            break;
        case F5:
            key = "F5";
            break;
        case F6:
            key = "F6";
            break;
        case F7:
            key = "F7";
            break;
        case F8:
            key = "F8";
            break;
        case F9:
            key = "F9";
            break;
        case F10:
            key = "F10";
            break;
        case F11:
            key = "F11";
            break;
        case F12:
            key = "F12";
            break;
        case UP:
            key = "UP";
            break;
        case DOWN:
            key = "DOWN";
            break;
        case LEFT:
            key = "LEFT";
            break;
        case RIGHT:
            key = "RIGHT";
            break;
        case INSERT:
            key = "INSERT";
            break;
        case DEL:
            key = "DEL";
            break;
        case HOME:
            key = "HOME";
            break;
        case END:
            key = "END";
            break;
        case PUP:
            key = "PUP";
            break;
        case PDOWN:
            key = "PDOWN";
            break;
        case SLOCK:
            key = "SLOCK";
            break;
        case ALT:
            key = "ALT";
            break;
    }

    if (key) {
        sprintf(buf, "[%s]", key);
    } else {
        sprintf(buf, "'%c'", ch);
    }
}

static int char_map[] = {
        0x00, 0x00, 0x00, 0x00, ESC,
        0x4F, 0x50, 0x00, 0x00, F1,
        0x4F, 0x51, 0x00, 0x00, F2,
        0x4F, 0x52, 0x00, 0x00, F3,
        0x4F, 0x53, 0x00, 0x00, F4,
        0x5B, 0x31, 0x35, 0x7E, F5,
        0x5B, 0x31, 0x37, 0x7E, F6,
        0x5B, 0x31, 0x38, 0x7E, F7,
        0x5B, 0x31, 0x39, 0x7E, F8,
        0x5B, 0x32, 0x30, 0x7E, F9,
        // F10 - is captured by the Gnome WM
        // F11 - is captured by the Gnome WM
        0x5B, 0x32, 0x7E, 0x00, INSERT,
        0x5B, 0x33, 0x7E, 0x00, DEL,
        0x5B, 0x32, 0x34, 0x7E, F12,
        0x5B, 0x35, 0x7E, 0x00, PUP,
        0x5B, 0x36, 0x7E, 0x00, PDOWN,
        0x5B, 0x41, 0x00, 0x00, UP,
        0x5B, 0x42, 0x00, 0x00, DOWN,
        0x5B, 0x43, 0x00, 0x00, RIGHT,
        0x5B, 0x44, 0x00, 0x00, LEFT,
        0x5B, 0x46, 0x00, 0x00, END,
        0x5B, 0x48, 0x00, 0x00, HOME,
        -1 };

int console_getc(void) {
    int chars[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int count = 0;
    int ch;
    for (int i = 0; i < 10; ++i) {
        ch = console_get_buffered_char();
        if (ch == -1) break;
        chars[count++] = ch;
    }

    if (count == 0) return -1;

#if 0
    printf("* ");
    for (int i = 0; i < count; ++i) {
        if (i > 0) printf(", ");
        printf("0x%X", chars[i]);
    }
#endif

    ch = -1;

    switch (chars[0]) {
        case 0x0A:
            ch = ENTER;
            break;
        case 0x1B: {
            ch = -1;
            int *p = char_map;
            while (*p != -1) {
                if (chars[1] == *p
             && chars[2] == *(p + 1)
             && chars[3] == *(p + 2)
             && chars[4] == *(p + 3)) {
         ch = *(p + 4);
         break;
                }
                p += 5;
            }
            break;
        }
        case 0x7F:
            ch = '\b';
            break;
        default:
            ch = chars[0];
            break;
    }

#if 0
    char buf[255];
    console_key_to_string(ch, buf);
    printf(" %s\n", buf);
#endif

    return ch;
}

void console_set_title(const char *title) {
    char buf[256];
    sprintf(buf, "\x1b]0;%s\x7", title);
    WRITE_CODE(buf);
}

int console_get_cursor_pos(int *x, int *y) {
    // Send escape code to report cursor position.
    WRITE_CODE_2("\033[6n", 4);

    // Discard input until we get ESC.
    int ch = 0;
    int result;
    while (ch != 0x1B) {
        if (read(STDIN_FILENO, &ch, 1) == -1) return 0;
    }

    // Collect output until we get R.
    char buf[10] = {0};
    buf[0] = ch;
    for (int i = 1, ch = 0; ch != 'R'; i++) {
        if (read(STDIN_FILENO, &ch, 1) == -1) return 0;
        buf[i] = ch;
    }

    // Parse output.
    sscanf(buf, "\033[%d;%dR", x, y);
    *x--; // VT100 origin is (1,1) not (0,0).
    *y--;

    return 1;
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
    write(STDOUT_FILENO, "\x1b[H", 4);
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
