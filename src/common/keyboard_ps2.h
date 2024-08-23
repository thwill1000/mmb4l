#if !defined(MMBASIC_KEYBOARD_PS2_H)
#define MMBASIC_KEYBOARD_PS2_H

#include <stdint.h>

#define MAX_SCANCODE 255

// clang-format off
/** Map from SDL_SCANCODE to PS2 scancode set 2. */
static const uint64_t keyboard_ps2_map[MAX_SCANCODE + 1] = {
    0x00, // SDL_SCANCODE_UNKNOWN
    0x00, // *unused* = 1
    0x00, // *unused* = 2
    0x00, // *unused* = 3

    0x1C, // SDL_SCANCODE_A = 4
    0x32, // SDL_SCANCODE_B = 5
    0x21, // SDL_SCANCODE_C = 6
    0x23, // SDL_SCANCODE_D = 7,
    0x24, // SDL_SCANCODE_E = 8,
    0x2B, // SDL_SCANCODE_F = 9,
    0x34, // SDL_SCANCODE_G = 10,
    0x33, // SDL_SCANCODE_H = 11,
    0x43, // SDL_SCANCODE_I = 12,
    0x3B, // SDL_SCANCODE_J = 13,
    0x42, // SDL_SCANCODE_K = 14,
    0x4B, // SDL_SCANCODE_L = 15,
    0x3A, // SDL_SCANCODE_M = 16,
    0x31, // SDL_SCANCODE_N = 17,
    0x44, // SDL_SCANCODE_O = 18,
    0x4D, // SDL_SCANCODE_P = 19,
    0x15, // SDL_SCANCODE_Q = 20,
    0x2D, // SDL_SCANCODE_R = 21,
    0x1B, // SDL_SCANCODE_S = 22,
    0x2C, // SDL_SCANCODE_T = 23,
    0x3C, // SDL_SCANCODE_U = 24,
    0x2A, // SDL_SCANCODE_V = 25,
    0x1D, // SDL_SCANCODE_W = 26,
    0x22, // SDL_SCANCODE_X = 27,
    0x35, // SDL_SCANCODE_Y = 28,
    0x1A, // SDL_SCANCODE_Z = 29,

    0x16, // SDL_SCANCODE_1 = 30,
    0x1E, // SDL_SCANCODE_2 = 31,
    0x26, // SDL_SCANCODE_3 = 32,
    0x25, // SDL_SCANCODE_4 = 33,
    0x2E, // SDL_SCANCODE_5 = 34,
    0x36, // SDL_SCANCODE_6 = 35,
    0x3D, // SDL_SCANCODE_7 = 36,
    0x3E, // SDL_SCANCODE_8 = 37,
    0x46, // SDL_SCANCODE_9 = 38,
    0x45, // SDL_SCANCODE_0 = 39,

    0x5A, // SDL_SCANCODE_RETURN = 40,
    0x76, // SDL_SCANCODE_ESCAPE = 41,
    0x66, // SDL_SCANCODE_BACKSPACE = 42,
    0x0D, // SDL_SCANCODE_TAB = 43,
    0x29, // SDL_SCANCODE_SPACE = 44,

    0x4E, // SDL_SCANCODE_MINUS = 45
    0x55, // SDL_SCANCODE_EQUALS = 46
    0x54, // SDL_SCANCODE_LEFTBRACKET = 47
    0x5B, // SDL_SCANCODE_RIGHTBRACKET = 48
    0x5D, // SDL_SCANCODE_BACKSLASH = 49
    0x4A, // SDL_SCANCODE_NONUSHASH = 50 *** same PS2 code as above
    0x4C, // SDL_SCANCODE_SEMICOLON = 51,
    0x52, // SDL_SCANCODE_APOSTROPHE = 52,
    0x0E, // SDL_SCANCODE_GRAVE = 53,
    0x41, // SDL_SCANCODE_COMMA = 54,
    0x49, // SDL_SCANCODE_PERIOD = 55,
    0x4A, // SDL_SCANCODE_SLASH = 56,

    0x58, // SDL_SCANCODE_CAPSLOCK = 57,

    0x05, // SDL_SCANCODE_F1 = 58,
    0x06, // SDL_SCANCODE_F2 = 59,
    0x04, // SDL_SCANCODE_F3 = 60,
    0x0C, // SDL_SCANCODE_F4 = 61,
    0x03, // SDL_SCANCODE_F5 = 62,
    0x0B, // SDL_SCANCODE_F6 = 63,
    0x83, // SDL_SCANCODE_F7 = 64,
    0x0A, // SDL_SCANCODE_F8 = 65,
    0x01, // SDL_SCANCODE_F9 = 66,
    0x09, // SDL_SCANCODE_F10 = 67,
    0x78, // SDL_SCANCODE_F11 = 68,
    0x07, // SDL_SCANCODE_F12 = 69,

    0xE012E07C, // SDL_SCANCODE_PRINTSCREEN = 70,
    0x7E, // SDL_SCANCODE_SCROLLLOCK = 71,
    0xE11477E1F014E077, // SDL_SCANCODE_PAUSE = 72,
    0xE070, // SDL_SCANCODE_INSERT = 73,
    0xE06C, // SDL_SCANCODE_HOME = 74,
    0xE07D, // SDL_SCANCODE_PAGEUP = 75,
    0xE071, // SDL_SCANCODE_DELETE = 76,
    0xE069, // SDL_SCANCODE_END = 77,
    0xEE7A, // SDL_SCANCODE_PAGEDOWN = 78,
    0xE074, // SDL_SCANCODE_RIGHT = 79,
    0xE06B, // SDL_SCANCODE_LEFT = 80,
    0xE072, // SDL_SCANCODE_DOWN = 81,
    0xE075, // SDL_SCANCODE_UP = 82,

    0x77, // SDL_SCANCODE_NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards */
    0xE04A, // SDL_SCANCODE_KP_DIVIDE = 84,
    0x7C, // SDL_SCANCODE_KP_MULTIPLY = 85,
    0x7B, // SDL_SCANCODE_KP_MINUS = 86,
    0x79, // SDL_SCANCODE_KP_PLUS = 87,
    0xE05A, // SDL_SCANCODE_KP_ENTER = 88,
    0x69, // SDL_SCANCODE_KP_1 = 89,
    0x72, // SDL_SCANCODE_KP_2 = 90,
    0x7A, // SDL_SCANCODE_KP_3 = 91,
    0x6B, // SDL_SCANCODE_KP_4 = 92,
    0x73, // SDL_SCANCODE_KP_5 = 93,
    0x74, // SDL_SCANCODE_KP_6 = 94,
    0x6C, // SDL_SCANCODE_KP_7 = 95,
    0x75, // SDL_SCANCODE_KP_8 = 96,
    0x7D, // SDL_SCANCODE_KP_9 = 97,
    0x70, // SDL_SCANCODE_KP_0 = 98,
    0x71, // SDL_SCANCODE_KP_PERIOD = 99,

    0x61, // SDL_SCANCODE_NONUSBACKSLASH = 100, /**< This is the additional key that ISO keyboards have over ANSI ones, */
    0x00, // SDL_SCANCODE_APPLICATION = 101, /**< windows contextual menu, compose */
    0x00, // SDL_SCANCODE_POWER = 102,
                              /**< The USB document says this is a status flag,
                               *   not a physical key - but some Mac keyboards
                               *   do have a power key. */
    0x00, // SDL_SCANCODE_KP_EQUALS = 103,
    0x00, // SDL_SCANCODE_F13 = 104,
    0x00, // SDL_SCANCODE_F14 = 105,
    0x00, // SDL_SCANCODE_F15 = 106,
    0x00, // SDL_SCANCODE_F16 = 107,
    0x00, // SDL_SCANCODE_F17 = 108,
    0x00, // SDL_SCANCODE_F18 = 109,
    0x00, // SDL_SCANCODE_F19 = 110,
    0x00, // SDL_SCANCODE_F20 = 111,
    0x00, // SDL_SCANCODE_F21 = 112,
    0x00, // SDL_SCANCODE_F22 = 113,
    0x00, // SDL_SCANCODE_F23 = 114,
    0x00, // SDL_SCANCODE_F24 = 115,
    0x00, // SDL_SCANCODE_EXECUTE = 116,
    0x00, // SDL_SCANCODE_HELP = 117,    /**< AL Integrated Help Center */
    0x00, // SDL_SCANCODE_MENU = 118,    /**< Menu (show menu) */
    0x00, // SDL_SCANCODE_SELECT = 119,
    0x00, // SDL_SCANCODE_STOP = 120,    /**< AC Stop */
    0x00, // SDL_SCANCODE_AGAIN = 121,   /**< AC Redo/Repeat */
    0x00, // SDL_SCANCODE_UNDO = 122,    /**< AC Undo */
    0x00, // SDL_SCANCODE_CUT = 123,     /**< AC Cut */
    0x00, // SDL_SCANCODE_COPY = 124,    /**< AC Copy */
    0x00, // SDL_SCANCODE_PASTE = 125,   /**< AC Paste */
    0x00, // SDL_SCANCODE_FIND = 126,    /**< AC Find */
    0x00, // SDL_SCANCODE_MUTE = 127,
    0x00, // SDL_SCANCODE_VOLUMEUP = 128,
    0x00, // SDL_SCANCODE_VOLUMEDOWN = 129,
    0x00, // SDL_SCANCODE_LOCKINGCAPSLOCK = 130,
    0x00, // SDL_SCANCODE_LOCKINGNUMLOCK = 131,
    0x00, // SDL_SCANCODE_LOCKINGSCROLLLOCK = 132,
    0x00, // SDL_SCANCODE_KP_COMMA = 133,
    0x00, // SDL_SCANCODE_KP_EQUALSAS400 = 134,

    0x00, // SDL_SCANCODE_INTERNATIONAL1 = 135, /**< used on Asian keyboards, see footnotes in USB doc */
    0x00, // SDL_SCANCODE_INTERNATIONAL2 = 136,
    0x00, // SDL_SCANCODE_INTERNATIONAL3 = 137, /**< Yen */
    0x00, // SDL_SCANCODE_INTERNATIONAL4 = 138,
    0x00, // SDL_SCANCODE_INTERNATIONAL5 = 139,
    0x00, // SDL_SCANCODE_INTERNATIONAL6 = 140,
    0x00, // SDL_SCANCODE_INTERNATIONAL7 = 141,
    0x00, // SDL_SCANCODE_INTERNATIONAL8 = 142,
    0x00, // SDL_SCANCODE_INTERNATIONAL9 = 143,
    0x00, // SDL_SCANCODE_LANG1 = 144, /**< Hangul/English toggle */
    0x00, // SDL_SCANCODE_LANG2 = 145, /**< Hanja conversion */
    0x00, // SDL_SCANCODE_LANG3 = 146, /**< Katakana */
    0x00, // SDL_SCANCODE_LANG4 = 147, /**< Hiragana */
    0x00, // SDL_SCANCODE_LANG5 = 148, /**< Zenkaku/Hankaku */
    0x00, // SDL_SCANCODE_LANG6 = 149, /**< reserved */
    0x00, // SDL_SCANCODE_LANG7 = 150, /**< reserved */
    0x00, // SDL_SCANCODE_LANG8 = 151, /**< reserved */
    0x00, // SDL_SCANCODE_LANG9 = 152, /**< reserved */

    0x00, // SDL_SCANCODE_ALTERASE = 153,    /**< Erase-Eaze */
    0x00, // SDL_SCANCODE_SYSREQ = 154,
    0x00, // SDL_SCANCODE_CANCEL = 155,      /**< AC Cancel */
    0x00, // SDL_SCANCODE_CLEAR = 156,
    0x00, // SDL_SCANCODE_PRIOR = 157,
    0x00, // SDL_SCANCODE_RETURN2 = 158,
    0x00, // SDL_SCANCODE_SEPARATOR = 159,
    0x00, // SDL_SCANCODE_OUT = 160,
    0x00, // SDL_SCANCODE_OPER = 161,
    0x00, // SDL_SCANCODE_CLEARAGAIN = 162,
    0x00, // SDL_SCANCODE_CRSEL = 163,
    0x00, // SDL_SCANCODE_EXSEL = 164,

    0x00, // *unused* = 165
    0x00, // *unused* = 166
    0x00, // *unused* = 167
    0x00, // *unused* = 168
    0x00, // *unused* = 169
    0x00, // *unused* = 170
    0x00, // *unused* = 171
    0x00, // *unused* = 172
    0x00, // *unused* = 173
    0x00, // *unused* = 174
    0x00, // *unused* = 175

    0x00, // SDL_SCANCODE_KP_00 = 176,
    0x00, // SDL_SCANCODE_KP_000 = 177,
    0x00, // SDL_SCANCODE_THOUSANDSSEPARATOR = 178,
    0x00, // SDL_SCANCODE_DECIMALSEPARATOR = 179,
    0x00, // SDL_SCANCODE_CURRENCYUNIT = 180,
    0x00, // SDL_SCANCODE_CURRENCYSUBUNIT = 181,
    0x00, // SDL_SCANCODE_KP_LEFTPAREN = 182,
    0x00, // SDL_SCANCODE_KP_RIGHTPAREN = 183,
    0x00, // SDL_SCANCODE_KP_LEFTBRACE = 184,
    0x00, // SDL_SCANCODE_KP_RIGHTBRACE = 185,
    0x00, // SDL_SCANCODE_KP_TAB = 186,
    0x00, // SDL_SCANCODE_KP_BACKSPACE = 187,
    0x00, // SDL_SCANCODE_KP_A = 188,
    0x00, // SDL_SCANCODE_KP_B = 189,
    0x00, // SDL_SCANCODE_KP_C = 190,
    0x00, // SDL_SCANCODE_KP_D = 191,
    0x00, // SDL_SCANCODE_KP_E = 192,
    0x00, // SDL_SCANCODE_KP_F = 193,
    0x00, // SDL_SCANCODE_KP_XOR = 194,
    0x00, // SDL_SCANCODE_KP_POWER = 195,
    0x00, // SDL_SCANCODE_KP_PERCENT = 196,
    0x00, // SDL_SCANCODE_KP_LESS = 197,
    0x00, // SDL_SCANCODE_KP_GREATER = 198,
    0x00, // SDL_SCANCODE_KP_AMPERSAND = 199,
    0x00, // SDL_SCANCODE_KP_DBLAMPERSAND = 200,
    0x00, // SDL_SCANCODE_KP_VERTICALBAR = 201,
    0x00, // SDL_SCANCODE_KP_DBLVERTICALBAR = 202,
    0x00, // SDL_SCANCODE_KP_COLON = 203,
    0x00, // SDL_SCANCODE_KP_HASH = 204,
    0x00, // SDL_SCANCODE_KP_SPACE = 205,
    0x00, // SDL_SCANCODE_KP_AT = 206,
    0x00, // SDL_SCANCODE_KP_EXCLAM = 207,
    0x00, // SDL_SCANCODE_KP_MEMSTORE = 208,
    0x00, // SDL_SCANCODE_KP_MEMRECALL = 209,
    0x00, // SDL_SCANCODE_KP_MEMCLEAR = 210,
    0x00, // SDL_SCANCODE_KP_MEMADD = 211,
    0x00, // SDL_SCANCODE_KP_MEMSUBTRACT = 212,
    0x00, // SDL_SCANCODE_KP_MEMMULTIPLY = 213,
    0x00, // SDL_SCANCODE_KP_MEMDIVIDE = 214,
    0x00, // SDL_SCANCODE_KP_PLUSMINUS = 215,
    0x00, // SDL_SCANCODE_KP_CLEAR = 216,
    0x00, // SDL_SCANCODE_KP_CLEARENTRY = 217,
    0x00, // SDL_SCANCODE_KP_BINARY = 218,
    0x00, // SDL_SCANCODE_KP_OCTAL = 219,
    0x00, // SDL_SCANCODE_KP_DECIMAL = 220,
    0x00, // SDL_SCANCODE_KP_HEXADECIMAL = 221,

    0x00, // *unused* = 222
    0x00, // *unused* = 223

    0x14, // SDL_SCANCODE_LCTRL = 224,
    0x12, // SDL_SCANCODE_LSHIFT = 225,
    0x11, // SDL_SCANCODE_LALT = 226, /**< alt, option */
    0xE01F, // SDL_SCANCODE_LGUI = 227, /**< windows, command (apple), meta */
    0xE014, // SDL_SCANCODE_RCTRL = 228,
    0x59, // SDL_SCANCODE_RSHIFT = 229,
    0xE011, // SDL_SCANCODE_RALT = 230, /**< alt gr, option */
    0xE027, // SDL_SCANCODE_RGUI = 231, /**< windows, command (apple), meta */
    0x00, // *unused* = 232
    0x00, // *unused* = 233
    0x00, // *unused* = 234
    0x00, // *unused* = 235
    0x00, // *unused* = 236
    0x00, // *unused* = 237
    0x00, // *unused* = 238
    0x00, // *unused* = 239
    0x00, // *unused* = 240
    0x00, // *unused* = 241
    0x00, // *unused* = 242
    0x00, // *unused* = 243
    0x00, // *unused* = 244
    0x00, // *unused* = 245
    0x00, // *unused* = 246
    0x00, // *unused* = 247
    0x00, // *unused* = 248
    0x00, // *unused* = 249
    0x00, // *unused* = 250
    0x00, // *unused* = 251
    0x00, // *unused* = 252
    0x00, // *unused* = 252
    0x00, // *unused* = 254
    0x00, // *unused* = 255
};
// clang-format on

#endif // #if !defined(MMBASIC_KEYBOARD_PS2_H)
