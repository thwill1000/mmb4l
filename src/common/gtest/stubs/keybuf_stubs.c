/*
 * Copyright (c) 2026 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include "../../keybuf.h"

int keybuf_count(void) { return 0; }
void keybuf_clear(void) { }
int keybuf_get(void) { return -1; }
bool keybuf_isatty(void) { return true; }
void keybuf_pump_tty(void) { }
void keybuf_put(char ch) { }
bool keybuf_exhausted() { return false; }
