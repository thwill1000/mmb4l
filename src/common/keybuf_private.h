/*-*****************************************************************************

MMBasic for Linux (MMB4L)

keybuf_private.h

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

#if !defined(MMB4L_KEYBUF_PRIVATE_H)
#define MMB4L_KEYBUF_PRIVATE_H

#include <SDL_atomic.h>

typedef enum {
    KEYBUF_RUNNING,         ///< Thread is actively reading input
    KEYBUF_PAUSE_REQUESTED, ///< Thread has been asked to pause
    KEYBUF_PAUSED,          ///< Thread is paused, not reading input
    KEYBUF_STOP_REQUESTED,  ///< Thread has been asked to stop
    KEYBUF_STOPPED,         ///< Thread has exited (or not been started)
} KeybufState;

extern SDL_atomic_t keybuf_state;

/**
 * Reads a single character from STDIN, using poll() to allow periodic
 * checking of the thread state rather than blocking indefinitely.
 *
 * @return the character read, or 0 on EOF, or -1 on error or if the
 *         thread state is no longer KEYBUF_RUNNING (e.g. pause or stop
 *         requested).
 */
int keybuf_read_char(void);

#endif // MMB4L_KEYBUF_PRIVATE_H
