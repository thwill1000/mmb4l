// Copyright (c) 2021 Thomas Hugo Williams

#include "console.h"
#include "option.h"

void console_clear(void) {
    // TODO
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
    Option.Height = 40;
    Option.Width = 80;
}
