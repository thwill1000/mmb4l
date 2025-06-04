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

MmResult display_cursor_up(int i) { return kOk; }
MmResult display_get_size(bool pixel, int *width, int *height) { return kOk; }
MmResult display_hide_cursor() { return kOk; }
MmResult display_putc(char c) { return kOk; }

MmResult display_puts(const char *s) {
    while (*s) ON_FAILURE_RETURN(display_putc(*s++));
    return kOk;
}

MmResult display_set_cursor_char_pos(int x, int y) { return kOk; }
MmResult display_set_cursor_pixel_pos(int x, int y) { return kOk; }
MmResult display_show_cursor() { return kOk; }
MmResult display_write(const char *buf, size_t *sz) { return kOk; }
