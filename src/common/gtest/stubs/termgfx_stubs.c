/*
 * Copyright (c) 2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include "../../error.h"
#include "../../termgfx.h"

MmResult termgfx_bell() { return kOk; }
MmResult termgfx_clear_to_end_of_line() { return kOk; }
MmResult termgfx_clear_to_end_of_screen() { return kOk; }
MmResult termgfx_cls() { return kOk; }
MmResult termgfx_colour_bg(MmGraphicsColour argb) { return kOk; }
MmResult termgfx_colour_fg(MmGraphicsColour argb) { return kOk; }
MmResult termgfx_cursor_left(int count, bool wrap) { return kOk; }
MmResult termgfx_cursor_up(int count) { return kOk; }
MmResult termgfx_get_cursor_pos(bool pixel, int *x, int *y) { return kOk; }
MmResult termgfx_get_size(bool pixel, int *width, int *height) { return kOk; }
MmResult termgfx_inverse(bool inverse) { return kOk; }
MmResult termgfx_putc(char c) { return kOk; }

MmResult termgfx_puts(const char *s) {
    while (*s) ON_FAILURE_RETURN(termgfx_putc(*s++));
    return kOk;
}

MmResult termgfx_reset() { return kOk; }
MmResult termgfx_scroll_down() { return kOk; }
MmResult termgfx_scroll_up() { return kOk; }
MmResult termgfx_set_cursor_pos(bool pixel, int x, int y) { return kOk; }
MmResult termgfx_show_cursor(bool show) { return kOk; }
MmResult termgfx_underline(bool underline) { return kOk; }
MmResult termgfx_wrapline() { return kOk; }
MmResult termgfx_write(const char *buf, size_t *sz) { return kOk; }
