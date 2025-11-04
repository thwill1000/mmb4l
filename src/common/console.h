/*-*****************************************************************************

MMBasic for Linux (MMB4L)

console.h

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

#if !defined(CONSOLE_H)
#define CONSOLE_H

#include <stdbool.h>
#include <stddef.h>

#include "graphics.h"
#include "mmresult.h"

// Ordinals match those used by MMBasic for DOS and original CMM.
#define BLACK           0
#define BLUE            1
#define GREEN           2
#define CYAN            3
#define RED             4
#define MAGENTA         5
#define YELLOW          6
#define WHITE           7
#define BRIGHT_BLACK    8
#define GREY            8
#define GRAY            8
#define BRIGHT_BLUE     9
#define BRIGHT_GREEN    10
#define BRIGHT_CYAN     11
#define BRIGHT_RED      12
#define BRIGHT_MAGENTA  13
#define BRIGHT_YELLOW   14
#define BRIGHT_WHITE    15

// Nominal dimensions of console font.
#define CONSOLE_FONT_HEIGHT  12
#define CONSOLE_FONT_WIDTH   8

extern int ListCnt;

/** @param  no_title  Set true to make console_set_title() a NOP. */
MmResult console_init(bool no_title);

void console_background(int colour);
void console_bell();

/**
 * Clears from the current cursor position to the end of the line.
 *
 * @return  kOk on success.
 */
MmResult console_clear_to_end_of_line();

/**
 * Clears from the current cursor position to the end of the screen.
 *
 * @return  kOk on success.
 */
MmResult console_clear_to_end_of_screen();

/**
 * Moves the cursor left, and optionally move up a line if in first column.
 *
 * @param[in]  count  number of characters to move left.
 * @param[in]  wrap   true:  move up a line if in first column,
 *                    false: do not wrap.
 * @return            kOk on success.
 */
MmResult console_cursor_left(int count, bool wrap);

/**
 * Moves the cursor up.
 *
 * @param[in]  count  number of characters to move up.
 * @return            kOk on success.
 */
MmResult console_cursor_up(int count);

void console_pump_input(void);
void console_clear(void);
void console_disable_raw_mode(void);
void console_enable_raw_mode(void);
void console_foreground(int colour);

/**
 * Sets the ANSI/tty terminal foreground and background colours.
 *
 * @param[in]  fg  the ARGB foreground colour value.
 * @param[in]  bg  the ARGB background colour value.
 * @return         kOk on success.
 */
MmResult console_colour(MmGraphicsColour fg, MmGraphicsColour bg);

/**
 * Sets the ANSI/tty terminal background colour.
 *
 * @param[in]  argb  the ARGB colour value.
 * @return           kOk on success.
 */
MmResult console_colour_bg(MmGraphicsColour argb);

/**
 * Sets the ANSI/tty terminal foreground colour.
 *
 * @param[in]  argb  the ARGB colour value.
 * @return           kOk on success.
 */
MmResult console_colour_fg(MmGraphicsColour argb);

/**
 * Flushes any buffered output to the ANSI/tty terminal.
 *
 * @return  kOk on success.
 */
MmResult console_flush();

/**
 * Gets a character from the console without blocking.
 *
 * @return  -1 if no character.
 */
int console_getc(void);

/**
 * Gets the cursor position.
 *
 * @param   x           on return holds the x-position.
 * @param   y           on return holds the y-position.
 */
MmResult console_get_cursor_pos(int *x, int *y);

/**
 * Gets the console size.
 *
 * @param  width   on return holds the width in characters.
 * @param  height  on return holds the height in characters.
 */
MmResult console_get_size(int *width, int *height);

void console_home_cursor(void);

/**
 * Enables/disables tty inverse mode.
 *
 * @param[in]  inverse  true:  to set inverse mode,
 *                      false: to set normal mode.
 * @return              kOK on success.
 */
MmResult console_inverse(bool inverse);

/** Gets the number of characters waiting in the console input queue. */
int console_kbhit(void);

/**
 * Writes a character to the ANSI/tty terminal.
 *
 * @param[in]  c  the character to write.
 * @return        kOk on success.
 */
char console_putc(char c);

/**
 * Writes a character to the ANSI/tty terminal without flushing.
 *
 * @param[in]  c  the character to write.
 * @return        kOk on success.
 */
char console_putc_noflush(char c);

/** Write a NULL terminated string to the console. */
void console_puts(const char *s);

/**
 * Resets the ANSI/tty terminal to its initial state.
 *
 * @return  kOK on success.
 */
MmResult console_reset(void);

/**
 * Scrolls the ANSI/tty terminal down by one text line.
 *
 * @return  kOK on success.
 */
MmResult console_scroll_down();

/**
 * Scrolls the ANSI/tty terminal up by one text line.
 *
 * @return  kOK on success.
 */
MmResult console_scroll_up();

/**
 * Sets the cursor position in character coordinates.
 *
 * @param  x  the new x-position.
 * @param  y  the new y-position.
 */
void console_set_cursor_pos(int x, int y);

/**
 * Resizes the console.
 *
 * @param   width   width in characters.
 * @param   height  height in characters.
 */
MmResult console_set_size(int width, int height);

/**
 * Sets the console title.
 *
 * @param  title    The new title.
 * @param  command  If true then title change was explicitly requested by a
 *                  command in the running MMBasic program. If false then the
 *                  title change is in response to a program being loaded or
 *                  the NEW command.
 */
void console_set_title(const char *title, bool command);

/** Shows or hides cursor. */
MmResult console_show_cursor(bool show);

/**
 * Synchronizes cached TTY terminal size with actual values.
 *
 * @return  kOK on success.
 */
MmResult console_sync();

/**
 * Enables or disables underline mode.
 *
 * @param[in]  underline  true:  enable underline mode,
 *                        false: disable underline mode.
 * @return                kOK on success.
 */
MmResult console_underline(bool underline);

/** No-op. The graphics terminal version flashes the cursor. */
static inline MmResult console_update_cursor() { return kOk; }

/**
 * If there is a "pending newline" (i.e. cursor beyond last column) then
 * print a CRLF and place the cursor to the start of the next line.
 *
 * @return  kOk on success.
 */
MmResult console_wrapline();

size_t console_write(const char *buf, size_t sz);

/** Adds a character to the console input buffer. */
void console_put_keypress(char ch);

#endif // #if !defined(CONSOLE_H)
