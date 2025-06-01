/*-*****************************************************************************

MMBasic for Linux (MMB4L)

display.h

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include "mmresult.h"

/**
 * Gets the current text cursor position.
 *
 * @param[in]   pixel  true:  to get value in pixels,
 *                     false: to get value in characters.
 * @param[out]  x      on exit, the x-coordinate of the cursor.
 * @param[out]  y      on exit, the y-coordinate of the cursor.
 * @return             kOK on success.
 */
MmResult display_get_cursor_pos(bool pixel, int *x, int *y);

/**
 * Gets the size of the display.
 *
 * @param[in]   pixel   true:  to get value in pixels,
 *                      false: to get value in characters.
 * @param[out]  width   on exit, the width of the display.
 * @param[out]  height  on exit, the height of the display.
 * @return              kOK on success.
 */
MmResult display_get_size(bool pixel, int *width, int *height);

/**
 * Gets the new text cursor position.
 *
 * @param[in]   pixel  true:  to set value in pixels,
 *                     false: to set value in characters.
 * @param[in]  x       the x-coordinate of the cursor.
 * @param[in]  y       the y-coordinate of the cursor.
 * @return             kOK on success.
 */
MmResult display_set_cursor_pos(bool pixel, int x, int y);

#endif // #if !defined(DISPLAY_H)
