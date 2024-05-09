/*
 * Copyright (c) 2024-2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "../keyboard.h"

#include <SDL.h>

int mod_state = 0x0;

const char* SDL_GetKeyName(SDL_Keycode key) { return NULL; }
SDL_Keymod SDL_GetModState() { return (SDL_Keymod) mod_state; }
void console_put_keypress(char ch) { }

}

TEST(KeyboardTest, KeyboardKeyDown_GivenOneKeyDown_AddsKeyToBuffer) {
    keyboard_init();

    SDL_Keysym keysym = {
        .sym = SDLK_a,
        .mod = 0x0,
    };
    EXPECT_EQ(kOk, keyboard_key_down(&keysym));

    EXPECT_EQ(1, keyboard_num_keys());
    EXPECT_EQ('a', keyboard_get_key(0));
    EXPECT_EQ('\0', keyboard_get_key(1));
    EXPECT_EQ(0x0, keyboard_get_locks());
    EXPECT_EQ(0x0, keyboard_get_modifiers());
}

TEST(KeyboardTest, KeyboardKeyDown_GivenTwoKeysDown_AddsBothKeysToBuffer) {
    keyboard_init();

    SDL_Keysym keysym = {
        .sym = SDLK_a,
        .mod = 0x0,
    };
    EXPECT_EQ(kOk, keyboard_key_down(&keysym));
    keysym.sym = SDLK_b;
    EXPECT_EQ(kOk, keyboard_key_down(&keysym));

    EXPECT_EQ(2, keyboard_num_keys());
    EXPECT_EQ('a', keyboard_get_key(0));
    EXPECT_EQ('b', keyboard_get_key(1));
    EXPECT_EQ('\0', keyboard_get_key(2));
    EXPECT_EQ(0x0, keyboard_get_locks());
    EXPECT_EQ(0x0, keyboard_get_modifiers());
}

TEST(KeyboardTest, KeyboardKeyDown_GivenKeyAlreadyDown_MovesKeyToEndOfBuffer) {
    keyboard_init();

    SDL_Keysym keysym = {
        .sym = SDLK_a,
        .mod = 0x0,
    };
    EXPECT_EQ(kOk, keyboard_key_down(&keysym));
    keysym.sym = SDLK_b;
    EXPECT_EQ(kOk, keyboard_key_down(&keysym));
    keysym.sym = SDLK_a;
    EXPECT_EQ(kOk, keyboard_key_down(&keysym));

    EXPECT_EQ(2, keyboard_num_keys());
    EXPECT_EQ('b', keyboard_get_key(0));
    EXPECT_EQ('a', keyboard_get_key(1));
    EXPECT_EQ('\0', keyboard_get_key(2));
    EXPECT_EQ(0x0, keyboard_get_locks());
    EXPECT_EQ(0x0, keyboard_get_modifiers());
}

TEST(KeyboardTest, KeyboardKeyDown_GivenBufferFull_Succeeds) {
    keyboard_init();

    SDL_Keysym keysym = {
        .sym = SDLK_a,
        .mod = 0x0,
    };
    EXPECT_EQ(kOk, keyboard_key_down(&keysym));
    for (int i = 1; i < 20; ++i) {
        keysym.sym = SDLK_a + i;
        EXPECT_EQ(kOk, keyboard_key_down(&keysym));
    }

    EXPECT_EQ(10, keyboard_num_keys());
    EXPECT_EQ('a', keyboard_get_key(0));
    EXPECT_EQ('b', keyboard_get_key(1));
    EXPECT_EQ('c', keyboard_get_key(2));
    EXPECT_EQ('d', keyboard_get_key(3));
    EXPECT_EQ('e', keyboard_get_key(4));
    EXPECT_EQ('f', keyboard_get_key(5));
    EXPECT_EQ('g', keyboard_get_key(6));
    EXPECT_EQ('h', keyboard_get_key(7));
    EXPECT_EQ('i', keyboard_get_key(8));
    EXPECT_EQ('j', keyboard_get_key(9));
    EXPECT_EQ(0x0, keyboard_get_locks());
    EXPECT_EQ(0x0, keyboard_get_modifiers());
}

TEST(KeyboardTest, KeyboardKeyDown_GivenKeyUp_RemovesKeyFromBuffer) {
    keyboard_init();

    SDL_Keysym keysym = {
        .sym = SDLK_a,
        .mod = 0x0,
    };
    EXPECT_EQ(kOk, keyboard_key_down(&keysym));
    keysym.sym = SDLK_b;
    EXPECT_EQ(kOk, keyboard_key_down(&keysym));
    keysym.sym = SDLK_a;
    EXPECT_EQ(kOk, keyboard_key_up(&keysym));

    EXPECT_EQ(1, keyboard_num_keys());
    EXPECT_EQ('b', keyboard_get_key(0));
    EXPECT_EQ('\0', keyboard_get_key(1));
    EXPECT_EQ(0x0, keyboard_get_locks());
    EXPECT_EQ(0x0, keyboard_get_modifiers());
}

TEST(KeyboardTest, KeyboardKeyDown_GivenKeyUp_WithEmptyBuffer_Succeeds) {
    keyboard_init();

    SDL_Keysym keysym = {
        .sym = SDLK_a,
        .mod = 0x0,
    };
    EXPECT_EQ(kOk, keyboard_key_up(&keysym));

    EXPECT_EQ(0, keyboard_num_keys());
    EXPECT_EQ('\0', keyboard_get_key(0));
    EXPECT_EQ(0x0, keyboard_get_locks());
    EXPECT_EQ(0x0, keyboard_get_modifiers());
}

TEST(KeyboardTest, GetModifiers) {
    keyboard_init();
    mod_state = 0x0;

    EXPECT_EQ(0x0, keyboard_get_modifiers());

    mod_state = KMOD_LALT;
    EXPECT_EQ(kLeftAlt, keyboard_get_modifiers());

    mod_state = KMOD_LALT | KMOD_LCTRL;
    EXPECT_EQ(kLeftAlt | kLeftCtrl, keyboard_get_modifiers());

    mod_state = KMOD_LALT | KMOD_LCTRL | KMOD_LGUI;
    EXPECT_EQ(kLeftAlt | kLeftCtrl | kLeftGui, keyboard_get_modifiers());

    mod_state = KMOD_LALT | KMOD_LCTRL | KMOD_LGUI | KMOD_LSHIFT;
    EXPECT_EQ(kLeftAlt | kLeftCtrl | kLeftGui | kLeftShift, keyboard_get_modifiers());

    mod_state = KMOD_LALT | KMOD_LCTRL | KMOD_LGUI | KMOD_LSHIFT | KMOD_RALT;
    EXPECT_EQ(kLeftAlt | kLeftCtrl | kLeftGui | kLeftShift | kRightAlt, keyboard_get_modifiers());

    mod_state = KMOD_LALT | KMOD_LCTRL | KMOD_LGUI | KMOD_LSHIFT | KMOD_RALT | KMOD_RCTRL;
    EXPECT_EQ(kLeftAlt | kLeftCtrl | kLeftGui | kLeftShift | kRightAlt | kRightCtrl,
              keyboard_get_modifiers());

    mod_state = KMOD_LALT | KMOD_LCTRL | KMOD_LGUI | KMOD_LSHIFT | KMOD_RALT | KMOD_RCTRL
            | KMOD_RGUI;
    EXPECT_EQ(kLeftAlt | kLeftCtrl | kLeftGui | kLeftShift | kRightAlt | kRightCtrl | kRightGui,
              keyboard_get_modifiers());

    mod_state = KMOD_LALT | KMOD_LCTRL | KMOD_LGUI | KMOD_LSHIFT | KMOD_RALT | KMOD_RCTRL
            | KMOD_RGUI | KMOD_RSHIFT;
    EXPECT_EQ(kLeftAlt | kLeftCtrl | kLeftGui | kLeftShift | kRightAlt | kRightCtrl | kRightGui
              | kRightShift, keyboard_get_modifiers());
}

TEST(KeyboardTest, GetLocks) {
    keyboard_init();
    mod_state = 0x0;

    EXPECT_EQ(0x0, keyboard_get_locks());

    mod_state = KMOD_CAPS;
    EXPECT_EQ(kCapsLock, keyboard_get_locks());

    mod_state = KMOD_CAPS | KMOD_NUM;
    EXPECT_EQ(kCapsLock | kNumLock, keyboard_get_locks());

    // KMOD_SCROLL is not available in earlier versions of SDL2.
    // mod_state = KMOD_CAPS | KMOD_NUM | KMOD_SCROLL;
    // EXPECT_EQ(kCapsLock | kNumLock | kScrollLock, keyboard_get_locks());

    mod_state = KMOD_CAPS | KMOD_NUM;
    EXPECT_EQ(kCapsLock | kNumLock, keyboard_get_locks());
}
