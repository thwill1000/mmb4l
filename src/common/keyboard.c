/*-*****************************************************************************

MMBasic for Linux (MMB4L)

keyboard.c

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
#include "keyboard.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include <SDL.h>

#define MAX_KEYS  10

static const char uk_key_map[512] = {
    // 0     1     2     3     4     5     6     7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0
    BKSP,  TAB, 0x00, 0x00, 0x00, ENTER, 0x00, 0x00, // 8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 16
    0x00, 0x00, 0x00,  ESC, 0x00, 0x00, 0x00, 0x00, // 24
     ' ', 0x00, 0x00,  '#', 0x00, 0x00, 0x00, '\'', // 32
    0x00, 0x00, 0x00, 0x00,  ',',  '-',  '.',  '/', // 40
     '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7', // 48
     '8',  '9', 0x00,  ';', 0x00,  '=', 0x00, 0x00, // 56
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 64
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 72
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 80
    0x00, 0x00, 0x00,  '[', '\\',  ']', 0x00, 0x00, // 88
     '`',  'a',  'b',  'c',  'd',  'e',  'f',  'g', // 96
     'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o', // 104
     'p',  'q',  'r',  's',  't',  'u',  'v',  'w', // 112
     'x',  'y',  'z', 0x00, 0x00, 0x00, 0x00,  DEL, // 120
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 128
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 136
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 144
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 152
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 160
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 168
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 176
    0x00, 0x00,   F1,   F2,   F3,   F4,   F5,   F6, // 184
      F7,   F8,   F9,  F10,  F11,  F12, PSCRN, SLOCK, // 192
    BREAK, INSERT, HOME, PUP, 0x00, END, PDOWN, RIGHT, // 200
    LEFT, DOWN,   UP, 0x00,  '/',  '*',  '-',  '+', // 208
    NUM_ENT, END, DOWN, PDOWN, LEFT, 0x00, RIGHT, HOME, // 216
     UP,   PUP, INSERT, DEL, 0x00, 0x00, 0x00, 0x00, // 224
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 232
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 240
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 248

    // Shifted
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0
    BKSP, STAB, 0x00, 0x00, 0x00, ENTER, 0x00, 0x00, // 8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 16
    0x00, 0x00, 0x00,  ESC, 0x00, 0x00, 0x00, 0x00, // 24
     ' ', 0x00, 0x00,  '~', 0x00, 0x00, 0x00,  '@', // 32
    0x00, 0x00, 0x00, 0x00,  '<',  '_',  '>',  '?', // 40
     ')',  '!',  '"',  '#',  '$',  '%',  '^',  '&', // 48
     '*',   '(', 0x00, ':', 0x00,  '+', 0x00, 0x00, // 56
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 64
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 72
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 80
    0x00, 0x00, 0x00,  '{',  '|',  '}', 0x00, 0x00, // 88
     '~',  'A',  'B',  'C',  'D',  'E',  'F',  'G', // 96
     'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O', // 104
     'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W', // 112
     'X',  'Y',  'Z', 0x00, 0x00, 0x00, 0x00, SDEL, // 120
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 128
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 136
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 144
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 152
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 160
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 168
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 176
    0x00, 0x00, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, // 184
    0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, PSCRN, SLOCK, // 192
    BREAK, INSERT, HOME, PUP, 0x00, END, PDOWN, SRIGHT, // 200
    LEFT, SDOWN,  UP, 0x00,  '/',  '*',  '-',  '+', // 208
    NUM_ENT, END, SDOWN, PDOWN, LEFT, 0x00, SRIGHT, HOME, // 216
      UP,  PUP, INSERT, DEL, 0x00, 0x00, 0x00, 0x00, // 224
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 232
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 240
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 248
};

static bool keyboard_initialised = false;
static char keyboard_keys[MAX_KEYS];
static int keyboard_keys_count = 0;

void keyboard_init() {
    keyboard_initialised = true;
    keyboard_keys_count = 0;
    memset(keyboard_keys, 0, sizeof(keyboard_keys));
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

/**
 * Converts SDL_Keysym to MMBasic key codes.
 *
 * SDL_Keysym#sym is of type SDL_Keycode which is used for SDL virtual key
 * codes and should in theory respect the current keyboard layout.
 */
static char keyboard_convert(const SDL_Keysym* keysym) {
    switch (keysym->sym) {
        case SDLK_CAPSLOCK:
        case SDLK_NUMLOCKCLEAR:
            return 0x0;
        case SDLK_LALT:
        case SDLK_RALT:
            return ALT;
        default: {
            int i = keysym->sym > 128 ? keysym->sym - (1 << 30) + 128 : keysym->sym;
            if (i > 255) {
                return 0x0;
            } else if (i >= 'a' && i <= 'z' && keysym->mod & KMOD_CTRL) {
                return i + 1 - 'a';
            } else if (keysym->mod & KMOD_SHIFT) {
                if (!(keysym->mod & KMOD_CAPS)) i += 256;
            } else if (keysym->mod & KMOD_CAPS) {
                if (!(keysym->mod & KMOD_SHIFT)) i += 256;
            } else if (keysym->mod & KMOD_NUM && i >= 217 && i <= 227) {
                switch (i) {
                    case 226: return '0';
                    case 227: return '.';
                    default:  return '1' + (i - 217);
                }
            }
            return uk_key_map[i];
        }
    }
}

/**
 * Adds 'ch' to the end of the list of keys that are down,
 * if it is already in the list then move it to the end of the list.
 */
static void keyboard_keys_add(char ch) {
    assert(keyboard_initialised);
    size_t dst = 0;
    for (size_t src = 0; src < MAX_KEYS; ++src) {
        if (keyboard_keys[src] == '\0') {
            keyboard_keys[dst] = ch;
            keyboard_keys_count++;
            break;
        } else if (keyboard_keys[src] == ch) {
            keyboard_keys_count--;
        } else {
            keyboard_keys[dst++] = keyboard_keys[src];
        }
    }
}

/**
 * Removes 'ch' from the list of keys that are down.
 */
static void keyboard_keys_remove(char ch) {
    assert(keyboard_initialised);
	size_t dst = 0;
	for (size_t src = 0; src < MAX_KEYS; ++src) {
        if (keyboard_keys[src] == ch) {
            keyboard_keys_count--;
        } else {
            keyboard_keys[dst++] = keyboard_keys[src];
            if (keyboard_keys[src] == '\0') break;
        }
	}
    if (keyboard_keys_count < 10) keyboard_keys[MAX_KEYS - 1] = '\0';
}

MmResult keyboard_key_down(const SDL_Keysym* keysym) {
    assert(keyboard_initialised);
    char ch = keyboard_convert(keysym);
    if (ch) {
        keyboard_keys_add(ch);
        console_put_keypress(ch);
    }
    return kOk;
}

MmResult keyboard_key_up(const SDL_Keysym* keysym) {
    assert(keyboard_initialised);
    char ch = keyboard_convert(keysym);
    if (ch) {
        keyboard_keys_remove(ch);
    }
    return kOk;
}

int keyboard_num_keys() {
    assert(keyboard_initialised);
    return keyboard_keys_count;
}

int keyboard_get_key(int i) {
    assert(keyboard_initialised);
    assert(i >= 0 && i < MAX_KEYS);
    return keyboard_keys[i];
}

int keyboard_get_modifiers() {
    assert(keyboard_initialised);
    int modifiers = 0x0;
    SDL_Keymod sdl_mods = SDL_GetModState();
    if (sdl_mods & KMOD_LALT) modifiers |= kLeftAlt;
    if (sdl_mods & KMOD_LCTRL) modifiers |= kLeftCtrl;
    if (sdl_mods & KMOD_LGUI) modifiers |= kLeftGui;
    if (sdl_mods & KMOD_LSHIFT) modifiers |= kLeftShift;
    if (sdl_mods & KMOD_RALT) modifiers |= kRightAlt;
    if (sdl_mods & KMOD_RCTRL) modifiers |= kRightCtrl;
    if (sdl_mods & KMOD_RGUI) modifiers |= kRightGui;
    if (sdl_mods & KMOD_RSHIFT) modifiers |= kRightShift;
    return modifiers;
}

int keyboard_get_locks() {
    assert(keyboard_initialised);
    int locks = 0x0;
    SDL_Keymod sdl_mods = SDL_GetModState();
    if (sdl_mods & KMOD_CAPS) locks |= kCapsLock;
    if (sdl_mods & KMOD_NUM) locks |= kNumLock;
#if SDL_VERSION_ATLEAST(2,0,18)
    if (sdl_mods & KMOD_SCROLL) locks |= kScrollLock;
#endif
    return locks;
}

void keyboard_dump_keys() {
    printf("Keysdown: ");
    for (int i = 0; i < MAX_KEYS; ++i) {
        if (i != 0) printf(", ");
        printf("%c", keyboard_keys[i]);
    }
    printf("\n");
}
