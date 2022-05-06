/*-*****************************************************************************

MMBasic for Linux (MMB4L)

console.h

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

// Copyright (c) 2021 Thomas Hugo Williams

#if !defined(CONSOLE_H)
#define CONSOLE_H

// the values returned by the standard control keys
#define TAB 0x9
#define BKSP 0x8
#define ENTER 0xa
#define ESC 0x1b

// the values returned by the function keys
#define F1 0x91
#define F2 0x92
#define F3 0x93
#define F4 0x94
#define F5 0x95
#define F6 0x96
#define F7 0x97
#define F8 0x98
#define F9 0x99
#define F10 0x9a
#define F11 0x9b
#define F12 0x9c

// the values returned by special control keys
#define UP 0x80
#define DOWN 0x81
#define LEFT 0x82
#define RIGHT 0x83
#define INSERT 0x84
#define DEL 0x7f
#define HOME 0x86
#define END 0x87
#define PUP 0x88
#define PDOWN 0x89
#define NUM_ENT ENTER
#define SLOCK 0x8c
#define ALT 0x8b

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

extern int ListCnt;
extern int MMCharPos;

void console_init();
void console_background(int colour);
void console_bell();
void console_pump_input(void);
void console_clear(void);
void console_disable_raw_mode(void);
void console_enable_raw_mode(void);
void console_foreground(int colour);

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
 * @param   timeout_ms  how long (in milliseconds) to wait for a response
 *                      from the terminal before reporting a failure.
 * @return  0 on success, -1 on error.
 */
int console_get_cursor_pos(int *x, int *y, int timeout_ms);

/**
 * Gets the console size.
 *
 * @param   width   on return holds the width in characters.
 * @param   height  on return holds the height in characters.
 * @return  0 on success, -1 on error.
 */
int console_get_size(int *width, int *height);

void console_home_cursor(void);
void console_invert(int invert);

/** Gets the number of characters waiting in the console input queue. */
int console_kbhit(void);

/** Writes a character to the console. */
char console_putc(char c);

void console_reset(void);

/**
 * Sets the cursor position.
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
 * @return  0 on success, -1 on error.
 */
int console_set_size(int width, int height);

void console_set_title(const char *title);
void console_show_cursor(int show);


#endif
