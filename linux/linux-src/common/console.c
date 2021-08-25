// Copyright (c) 2021 Thomas Hugo Williams

#include <termios.h>
#include <unistd.h>

#include "console.h"
#include "option.h"

static struct termios orig_termios;

void console_clear(void) {
    // TODO
}

void console_disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void console_enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    // atexit(console_disable_raw_mode); - done in main.c
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0; // 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int console_getc(void) {
    char ch;
    ssize_t result = read(STDIN_FILENO, &ch, 1);
    switch (result) {
        case 0:
            return -1;
        case 1:
            return ch;
        default:
            error("Unexpected result from read()");
    }
}

void console_set_title(const char *title) {
    // TODO
}

void console_get_size(void) {
#if 0
    CONSOLE_SCREEN_BUFFER_INFO consoleinfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleinfo);
    Option.Height = consoleinfo.srWindow.Bottom - consoleinfo.srWindow.Top;
    Option.Width = consoleinfo.srWindow.Right - consoleinfo.srWindow.Left;
#endif
    Option.Height = 50;
    Option.Width = 100;
}
