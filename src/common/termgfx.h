/*-*****************************************************************************

MMBasic for Linux (MMB4L)

termgfx.h

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#if !defined(TERMGFX_H)
#define TERMGFX_H

#include <stdbool.h>
#include <stddef.h>

#include "graphics.h"
#include "mmresult.h"

// Terminal implementation using SDL graphics window.
// See display.h for function documentation.

MmResult termgfx_bell();
MmResult termgfx_clear_to_end_of_line();
MmResult termgfx_clear_to_end_of_screen();
MmResult termgfx_cls();
MmResult termgfx_colour_bg(MmGraphicsColour argb);
MmResult termgfx_colour_fg(MmGraphicsColour argb);
MmResult termgfx_cursor_left(int count, bool wrap);
MmResult termgfx_cursor_up(int count);
MmResult termgfx_get_cursor_pos(bool pixel, int *x, int *y);
MmResult termgfx_get_size(bool pixel, int *width, int *height);
MmResult termgfx_inverse(bool inverse);
MmResult termgfx_putc(char c);
MmResult termgfx_puts(const char *s);
MmResult termgfx_reset();
MmResult termgfx_scroll_down();
MmResult termgfx_scroll_up();
MmResult termgfx_set_cursor_pos(bool pixel, int x, int y);
MmResult termgfx_show_cursor(bool show);
MmResult termgfx_underline(bool underline);
MmResult termgfx_update_cursor();
MmResult termgfx_wrapline();
MmResult termgfx_write(const char *buf, size_t *sz);

#endif // #if !defined(TERMGFX_H)
