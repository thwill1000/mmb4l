/*
 * Copyright (c) 2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include "../../error.h"
#include "../../display.h"

bool display_bell_sounded = false;

MmResult display_bell() {
    display_bell_sounded = true;
    return kOk;
}

MmResult display_colour_bg(MmGraphicsColour argb) { return kOk; }
MmResult display_colour_fg(MmGraphicsColour argb) { return kOk; }
MmResult display_cursor_up(int count) { return kOk; }
MmResult display_get_cursor_pos(bool pixel, int *x, int *y) { return kOk; }
MmResult display_get_size(bool pixel, int *width, int *height) { return kOk; }
MmResult display_inverse(bool inverse) { return kOk; }
MmResult display_putc(char c) { return kOk; }

MmResult display_puts(const char *s) {
    while (*s) ON_FAILURE_RETURN(display_putc(*s++));
    return kOk;
}

MmResult display_reset() { return kOk; }
MmResult display_scroll_down() { return kOk; }
MmResult display_scroll_up() { return kOk; }
MmResult display_set_cursor_char_pos(int x, int y) { return kOk; }
MmResult display_set_cursor_pixel_pos(int x, int y) { return kOk; }
MmResult display_set_cursor_pos(bool pixel, int x, int y) { return kOk; }
MmResult display_show_cursor(bool show) { return kOk; }
MmResult display_underline(bool underline) { return kOk; }
MmResult display_update_cursor() { return kOk; }
MmResult display_write(const char *buf, size_t *sz) { return kOk; }
