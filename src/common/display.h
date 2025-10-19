/*-*****************************************************************************

MMBasic for Linux (MMB4L)

display.h

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

#if !defined(DISPLAY_H)
#define DISPLAY_H

#include <stdbool.h>

#include "graphics.h"
#include "mmresult.h"

/** Makes a "bell" noise. */
MmResult display_bell();

/** Clears the display and moves the cursor to the home position. */
MmResult display_cls();

/**
 * Sets the display background colour.
 *
 * @param[in]  argb  the ARGB colour value.
 * @return           kOk on success.
 */
MmResult display_colour_bg(MmGraphicsColour argb);

/**
 * Sets the display foreground colour.
 *
 * @param[in]  argb  the ARGB colour value.
 * @return           kOk on success.
 */
MmResult display_colour_fg(MmGraphicsColour argb);

/**
 * Moves the cursor left, and optionally move up a line if in first column.
 *
 * @param[in]  count  number of characters to move left.
 * @param[in]  wrap   true:  move up a line if in first column,
 *                    false: do not wrap.
 * @return            kOk on success.
 */
MmResult display_cursor_left(int count, bool wrap);

/**
 * Moves the cursor up.
 *
 * @param[in]  count  number of characters to move up.
 * @return            kOk on success.
 */
MmResult display_cursor_up(int count);

/**
 * Gets the current text cursor position.
 *
 * @param[in]   pixel  true:  to get value in pixels,
 *                     false: to get value in characters.
 * @param[out]  x      on exit, the x-coordinate of the cursor.
 * @param[out]  y      on exit, the y-coordinate of the cursor.
 * @return             kOk on success.
 */
MmResult display_get_cursor_pos(bool pixel, int *x, int *y);

/**
 * Gets the size of the display.
 *
 * @param[in]   pixel   true:  to get value in pixels,
 *                      false: to get value in characters.
 * @param[out]  width   on exit, the width of the display.
 * @param[out]  height  on exit, the height of the display.
 * @return              kOk on success.
 */
MmResult display_get_size(bool pixel, int *width, int *height);

/**
 * Inverses the display colours.
 *
 * @param[in]  inverse  true:  to set inverse mode,
 *                      false: to set normal mode.
 * @return              kOK on success.
 */
MmResult display_inverse(bool inverse);

/**
 * Writes a character to the display.
 *
 * @param[in]  c  the character to write.
 * @return        kOk on success.
 */
MmResult display_putc(char c);

/**
 * Writes a C-string to the display.
 *
 * @param[in]  s  the C-string to write.
 * @return        kOk on success.
 */
MmResult display_puts(const char *s);

/**
 * Resets the display to its initial state.
 *
 * @return  kOK on success.
 */
MmResult display_reset();

/**
 * Scrolls the display down by one text line.
 *
 * @return  kOK on success.
 */
MmResult display_scroll_down();

/**
 * Scrolls the display up by one text line.
 *
 * @return  kOK on success.
 */
MmResult display_scroll_up();

/**
 * Sets the new text cursor position.
 *
 * @param[in]  pixel  true:  to set value in pixels,
 *                    false: to set value in characters.
 * @param[in]  x      the x-coordinate of the cursor.
 * @param[in]  y      the y-coordinate of the cursor.
 * @return            kOk on success.
 */
MmResult display_set_cursor_pos(bool pixel, int x, int y);

/**
 * Shows/hides the cursor.
 *
 * @param[in]  show  true:  show the cursor,
 *                   false: hide the cursor.
 * @return     kOk on success.
 */
MmResult display_show_cursor(bool show);

/**
 * Synchronizes cached TTY terminal size with actual values.
 *
 * @return  kOK on success.
 */
MmResult display_sync();

/**
 * Enables or disables underline mode.
 *
 * @param[in]  underline  true:  enable underline mode,
 *                        false: disable underline mode.
 * @return                kOK on success.
 */
MmResult display_underline(bool underline);

/**
 * Tells the terminal to update/blink the cursor.
 *
 * A terminal implementation can ignore this if insufficient time has passed
 * since the cursor was last blinked.
 *
 * @return  kOk on success.
 */
MmResult display_update_cursor();

/**
 * Write characters to the display.
 *
 * @param[in]       buf  write characters from this buffer.
 * @param[in, out]  sz   on entry, number of characters to write.
 *                       on exit, number of characters written.
 * @return               kOk on success.
 */
MmResult display_write(const char *buf, size_t *sz);

#endif //  !defined(MMB4L_DISPLAY_H)
