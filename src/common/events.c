/*-*****************************************************************************

MMBasic for Linux (MMB4L)

events.c

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

#include "console.h"
#include "events.h"
#include "utility.h"

#include <stdbool.h>

#include <SDL.h>

static const char* NO_ERROR = "";
static bool events_initialised = false;

MmResult events_init() {
    if (events_initialised) return kOk;
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_VIDEO) < 0) {
        return kEventsApiError;
    }
    events_initialised = true;
    return kOk;
}

const char* events_last_error() {
    const char* emsg = SDL_GetError();
    return emsg && *emsg ? emsg : NO_ERROR;
}

/* Prints key modifier info. */
static void print_key_modifiers(SDL_Keymod mod) {
    printf("Modifers: ");

    /* If there are none then say so and return */
    if (mod == KMOD_NONE) {
        printf("None\n");
        return;
    }

    /* Check for the presence of each SDLMod value */
    /* This looks messy, but there really isn't    */
    /* a clearer way.                              */
    if (mod & KMOD_NUM) printf("NUMLOCK ");
    if (mod & KMOD_CAPS) printf("CAPSLOCK ");
    if (mod & KMOD_LCTRL) printf("LCTRL ");
    if (mod & KMOD_RCTRL) printf("RCTRL ");
    if (mod & KMOD_RSHIFT) printf("RSHIFT ");
    if (mod & KMOD_LSHIFT) printf("LSHIFT ");
    if (mod & KMOD_RALT) printf("RALT ");
    if (mod & KMOD_LALT) printf("LALT ");
    if (mod & KMOD_CTRL) printf("CTRL ");
    if (mod & KMOD_SHIFT) printf("SHIFT ");
    if (mod & KMOD_ALT) printf("ALT ");
    printf("\n");
}

/* Prints all information about a key event. */
static void print_key_info(SDL_KeyboardEvent *key){
    /* Is it a release or a press? */
    if (key->type == SDL_KEYUP)
        printf("Release:- ");
    else
        printf("Press:- ");

    /* Print the hardware scancode first */
    printf("Scancode: 0x%02X", key->keysym.scancode);
    /* Print the name of the key */
    printf(", Name: %s", SDL_GetKeyName(key->keysym.sym));
    /* We want to print the unicode info, but we need to make */
    /* sure its a press event first (remember, release events */
    /* don't have unicode info                                */
    if (key->type == SDL_KEYDOWN) {
        /* If the Unicode value is less than 0x80 then the    */
        /* unicode value can be used to get a printable       */
        /* representation of the key, using (char)unicode.    */
        printf(", Unicode: ");
        if (key->keysym.sym < 0x80 && key->keysym.sym > 0){
            printf("%c (0x%04X)", (char)key->keysym.sym, key->keysym.sym);
        }
        else{
            printf("? (0x%04X)", key->keysym.sym);
        }
    }
    printf("\n");
    print_key_modifiers(key->keysym.mod);
}

void events_pump() {
    if (!events_initialised) return;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_RIGHT:
                        console_put_keypress(RIGHT);
                        break;
                    case SDLK_LEFT:
                        console_put_keypress(LEFT);
                        break;
                    case SDLK_DOWN:
                        console_put_keypress(DOWN);
                        break;
                    case SDLK_UP:
                        console_put_keypress(UP);
                        break;
                    case SDLK_c:
                        console_put_keypress(event.key.keysym.mod & KMOD_CTRL ? 3 : (char) event.key.keysym.sym);
                        break;
                    default:
                        if (event.key.keysym.sym > 0 && event.key.keysym.sym < 0x80) {
                            console_put_keypress((char) event.key.keysym.sym);
                        }
                        break;
                }
                break;

            case SDL_KEYUP:
                break;
        }
    }
}
