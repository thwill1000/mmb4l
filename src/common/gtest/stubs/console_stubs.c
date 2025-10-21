/*
 * Copyright (c) 2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include "../../console.h"

bool console_bell_sounded = false;

MmResult console_init(bool no_title) { return kOk; }
void console_background(int colour) { }
void console_bell() { console_bell_sounded = true; }
void console_cursor_up(int i) { }
void console_pump_input(void) { }
void console_clear(void) { }
void console_disable_raw_mode(void) { }
void console_enable_raw_mode(void) { }
void console_foreground(int colour) { }
int console_getc(void) { return -1; }
int console_get_cursor_pos(int *x, int *y, int timeout_ms) { return -1; }
int console_get_size(int *width, int *height, int timeout_ms) { return -1; }
void console_home_cursor(void) { }
MmResult console_inverse(bool inverse) { return kOk; }
int console_kbhit(void) { return -1; }
char console_putc(char c) { return -1; }
void console_puts(const char *s) { }
MmResult console_reset(void) { return kOk; }
MmResult console_scroll_down() { return kOk; }
MmResult console_scroll_up() { return kOk; }
void console_set_cursor_char_pos(int x, int y) { }
void console_set_cursor_pixel_pos(int x, int y) { }
int console_set_size(int width, int height) { return -1; }
void console_set_title(const char *title, bool command) { }
void console_show_cursor(bool show) { }
MmResult console_underline(bool underline) { return kOk; }
size_t console_write(const char *buf, size_t sz) { return 0; }
void console_put_keypress(char ch) { }
