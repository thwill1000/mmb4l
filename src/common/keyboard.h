/*-*****************************************************************************

MMBasic for Linux (MMB4L)

keyboard.h

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

#if !defined(MMB4L_KEYBOARD_H)
#define MMB4L_KEYBOARD_H

#include "mmresult.h"

typedef struct SDL_Keysym SDL_Keysym;

typedef enum {
    kLeftAlt    = 0x01,
    kLeftCtrl   = 0x02,
    kLeftGui    = 0x04,
    kLeftShift  = 0x08,
    kRightAlt   = 0x10,
    kRightCtrl  = 0x20,
    kRightGui   = 0x40,
    kRightShift = 0x80,
} KeyboardModifier;

typedef enum {
    kCapsLock   = 0x01,
    kNumLock    = 0x02,
    kScrollLock = 0x04,
} KeyboardLock;

/** Initialises the keyboard module. */
void keyboard_init();

/** Handles a key press. */
MmResult keyboard_key_down(const SDL_Keysym* keysym);

/** Handles a key release. */
MmResult keyboard_key_up(const SDL_Keysym* keysym);

/** Gets the number of keys (max 10) currently held down. */
int keyboard_num_keys();

/** Gets the n'th key (0..9) currently held down. */
int keyboard_get_key(int i);

/** Gets a bitmask of the keyboard modifier keys currently held down. */
int keyboard_get_modifiers();

/** Gets a bitmask of the keyboard lock keys currently held down. */
int keyboard_get_locks();

/** Gets the PS2 scancode of the last key press/release. */
uint64_t keyboard_get_last_ps2_scancode();

/** Writes current keydown state to STDOUT. */
void keyboard_dump_keys();

#endif // #if !defined(MMB4L_KEYBOARD_H)
