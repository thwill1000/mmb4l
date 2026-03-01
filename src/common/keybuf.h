/*-*****************************************************************************

MMBasic for Linux (MMB4L)

keybuf.h

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
   be displayed on the console at startup (additional copyright messages may
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

#if !defined(MMB4L_KEYBUF_H)
#define MMB4L_KEYBUF_H

#include <stdbool.h>

#include "mmresult.h"

/**
 * Initialises the keyboard buffer and starts the background input thread.
 * Must be called before any other keybuf functions.
 */
MmResult keybuf_init(void);

/**
 * Signals the background input thread to stop and cleans up resources.
 * Does not wait for the thread to exit - relies on the OS to clean up
 * any blocked read on process exit.
 */
void keybuf_term(void);

/**
 * Discards all characters currently waiting in the keyboard buffer.
 */
void keybuf_clear(void);

/**
 * Gets the number of characters waiting in the keyboard buffer.
 *
 * @return  Number of characters available.
 */
int keybuf_count(void);

/**
 * Gets a character from the keyboard buffer without blocking.
 *
 * @return  The next character, or -1 if no character is available.
 */
int keybuf_get(void);

/**
 * Is the keyboard buffer connected to a terminal (tty) ?
 *
 * @return  true if stdin is a terminal, false if it is a pipe or redirected file.
 */
bool keybuf_isatty(void);

/**
 * Adds a character to the keyboard buffer.
 * If the buffer is full the oldest character is discarded to make room,
 * except when reading from a pipe where the thread will block instead.
 *
 * @param  ch  The character to add.
 */
void keybuf_put(char ch);

/**
 * Has all piped or redirected input been read and consumed ?
 * Always returns false when stdin is a terminal.
 *
 * @return  true if the background thread has exited due to EOF on stdin
 *          and the keyboard buffer has been fully consumed.
 */
bool keybuf_exhausted(void);

#endif // MMB4L_KEYBUF_H
