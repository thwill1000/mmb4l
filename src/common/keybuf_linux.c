/*-*****************************************************************************

MMBasic for Linux (MMB4L)

keybuf_linux.c

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
   be displayed on the keybuf at startup (additional copyright messages may
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

#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <unistd.h>

#include "keybuf.h"
#include "keybuf_private.h"

bool keybuf_isatty(void) {
#if defined(__ANDROID__)
    return true;
#else
    return isatty(STDIN_FILENO);
#endif
}

/**
 * Blocks until a character is available on STDIN then returns it.
 * Returns the character, or 0 on EOF or -1 on error.
 */
int keybuf_read_char(void) {
    while (SDL_AtomicGet(&keybuf_state) == KEYBUF_RUNNING) {
        struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
        int ready = poll(&pfd, 1, 10);
        if (ready > 0) {
            char ch;
            ssize_t result;
            do {
                result = read(STDIN_FILENO, &ch, 1);
            } while (result == -1 && errno == EINTR);
            if (result == 1) return (unsigned char) ch;
            if (result == 0) return 0;
            return -1;
        }
    }
    return -1;
}
