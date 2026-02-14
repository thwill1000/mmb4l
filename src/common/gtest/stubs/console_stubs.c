/*
 * Copyright (c) 2025-2026 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include "../../console.h"

MmResult (*mock_console_get_cursor_pos)(int *, int *) = NULL;
MmResult (*mock_console_get_size)(int *, int *) = NULL;
void (*mock_console_set_cursor_pos)(int, int) = NULL;

bool console_bell_sounded = false;

MmResult console_init(bool no_title) { return kOk; }
void console_background(int colour) { }
void console_bell() { console_bell_sounded = true; }
MmResult console_clear_to_end_of_line() { return kOk; }
MmResult console_clear_to_end_of_screen() { return kOk; }
MmResult console_cursor_left(int count, bool wrap) { return kOk; }
MmResult console_cursor_up(int count) { return kOk; }
void console_clear(void) { }
MmResult console_colour(MmGraphicsColour fg, MmGraphicsColour bg) { return kOk; }
MmResult console_colour_bg(MmGraphicsColour argb) { return kOk; }
MmResult console_colour_fg(MmGraphicsColour argb) { return kOk; }
void console_disable_raw_mode(void) { }
void console_enable_raw_mode(void) { }
void console_foreground(int colour) { }
MmResult console_flush() { return kOk; }

MmResult console_get_cursor_pos(int *x, int *y) {
    if (mock_console_get_cursor_pos) {
        return mock_console_get_cursor_pos(x, y);
    } else {
        return kOk;
    }
}

MmResult console_get_size(int *width, int *height) {
    if (mock_console_get_size) {
        return mock_console_get_size(width, height);
    } else {
        return kOk;
    }
}

void console_home_cursor(void) { }
MmResult console_inverse(bool inverse) { return kOk; }
char console_putc(char c) { return -1; }
char console_putc_noflush(char c) { return -1; }
void console_puts(const char *s) { }
MmResult console_reset(void) { return kOk; }
MmResult console_scroll_down() { return kOk; }
MmResult console_scroll_up() { return kOk; }

void console_set_cursor_pos(int x, int y) {
    if (mock_console_set_cursor_pos) {
        mock_console_set_cursor_pos(x, y);
    }
}

MmResult console_set_size(int width, int height) { return kOk; }
void console_set_title(const char *title, bool command) { }
MmResult console_show_cursor(bool show) { return kOk; }
MmResult console_sync() { return kOk; }
MmResult console_underline(bool underline) { return kOk; }
MmResult console_wrapline() { return kOk; }
size_t console_write(const char *buf, size_t sz) { return 0; }
