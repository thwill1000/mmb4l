#include "../common/error.h"
#include "../common/version.h"

#if 0
#define RGB(red, green, blue, trans) (unsigned int) (((trans & 0b1111) << 24) | ((red & 0b11111111) << 16) | ((green  & 0b11111111) << 8) | (blue & 0b11111111))

#define RGB_BLACK     RGB(   0,     0,     0,     0)
#define RGB_BLUE      RGB(   0,     0,   255,   255)
#define RGB_GREEN     RGB(   0,   255,     0,   255)
#define RGB_CYAN      RGB(   0,   255,   255,   255)
#define RGB_RED       RGB( 255,     0,     0,   255)
#define RGB_MAGENTA   RGB( 255,     0,   192,   255)
#define RGB_YELLOW    RGB( 255,   255,     0,   255)
#define RGB_BROWN     RGB(0xA5,  0x2A,  0x2A,   255)
#define RGB_GRAY      RGB(  64,    64,    64,   255)
#define RGB_LITEGRAY  RGB( 128,   128,   128,   255)
#define RGB_WHITE     RGB( 255,   255,   255,   255)
#define RGB_ORANGE    RGB(0xFF,  0xA5,     0,   255)
#define RGB_PINK      RGB(0xFF,  0xA0,  0xAB,   255)
#define RGB_GOLD      RGB(0xFF,  0xD7,  0x00,   255)
#define RGB_SALMON    RGB(0xFA,  0x80,  0x72,   255)
#define RGB_BEIGE     RGB(0xF5,  0xF5,  0xDC,   255)
#define RGB_NOTBLACK  (VideoColour==8? RGB(0,32, 0,15): (VideoColour==12? RGB(16,16,16,15): (VideoColour==16 ? RGB(0,4,0,15) : RGB(0,0,0,255))))

static int VideoColour = 8;

static int rgb(int r, int g, int b, int t) {
    return RGB(r, g, b, t);
}
#endif

// Version derived from PiCromite
void fun_rgb(void) {
    getargs(&ep, 5, ",");
    if (argc == 5) {
        iret = rgb(getint(argv[0], 0, 255), getint(argv[2], 0, 255),
                   getint(argv[4], 0, 255));
    } else if (argc == 1) {
        if (checkstring(argv[0], "WHITE")) {
            iret = RGB_WHITE;
        } else if (checkstring(argv[0], "BLACK")) {
            iret = RGB_BLACK;
        } else if (checkstring(argv[0], "BLUE")) {
            iret = RGB_BLUE;
        } else if (checkstring(argv[0], "GREEN")) {
            iret = RGB_GREEN;
        } else if (checkstring(argv[0], "CYAN")) {
            iret = RGB_CYAN;
        } else if (checkstring(argv[0], "RED")) {
            iret = RGB_RED;
        } else if (checkstring(argv[0], "MAGENTA")) {
            iret = RGB_MAGENTA;
        } else if (checkstring(argv[0], "YELLOW")) {
            iret = RGB_YELLOW;
        } else if (checkstring(argv[0], "BROWN")) {
            iret = RGB_BROWN;
        } else if (checkstring(argv[0], "GRAY")) {
            iret = RGB_GRAY;
        } else {
            error("Invalid colour: $", argv[0]);
        }
    } else {
        ERROR_SYNTAX;
    }

    targ = T_INT;
}

#if 0
// Version derived from CMM2
void fun_rgb(void) {
    getargs(&ep, 7, ",");
    if (argc == 5) {
        iret = rgb(getint(argv[0], 0, 255), getint(argv[2], 0, 255),
                   getint(argv[4], 0, 255), 15);
    } else if (argc == 1) {
        if (checkstring(argv[0], "WHITE"))
            iret = RGB_WHITE;
        else if (checkstring(argv[0], "BLACK"))
            iret = RGB_BLACK;
        else if (checkstring(argv[0], "NOTBLACK"))
            iret = RGB_NOTBLACK;
        else if (checkstring(argv[0], "BLUE"))
            iret = RGB_BLUE;
        else if (checkstring(argv[0], "GREEN"))
            iret = RGB_GREEN;
        else if (checkstring(argv[0], "CYAN"))
            iret = RGB_CYAN;
        else if (checkstring(argv[0], "RED"))
            iret = RGB_RED;
        else if (checkstring(argv[0], "MAGENTA"))
            iret = RGB_MAGENTA;
        else if (checkstring(argv[0], "YELLOW"))
            iret = RGB_YELLOW;
        else if (checkstring(argv[0], "BROWN"))
            iret = RGB_BROWN;
        else if (checkstring(argv[0], "GRAY"))
            iret = RGB_GRAY;
        else if (checkstring(argv[0], "GREY"))
            iret = RGB_GRAY;
        else if (checkstring(argv[0], "LIGHTGRAY"))
            iret = RGB_LITEGRAY;
        else if (checkstring(argv[0], "LIGHTGRAY"))
            iret = RGB_LITEGRAY;
        else if (checkstring(argv[0], "ORANGE"))
            iret = RGB_ORANGE;
        else if (checkstring(argv[0], "PINK"))
            iret = RGB_PINK;
        else if (checkstring(argv[0], "GOLD"))
            iret = RGB_GOLD;
        else if (checkstring(argv[0], "SALMON"))
            iret = RGB_SALMON;
        else if (checkstring(argv[0], "BEIGE"))
            iret = RGB_BEIGE;
        else
            error("Invalid colour: $", argv[0]);
        if (VideoColour != 32) iret &= 0xFFFFFFF;
    } else if (argc == 3) {
        if (VideoColour == 8 || VideoColour == 16)
            error("Transparency not valid for this mode");
        if (checkstring(argv[0], "WHITE"))
            iret = RGB_WHITE;
        else if (checkstring(argv[0], "BLACK"))
            iret = RGB_BLACK;
        else if (checkstring(argv[0], "NOTBLACK"))
            iret = RGB_NOTBLACK;
        else if (checkstring(argv[0], "BLUE"))
            iret = RGB_BLUE;
        else if (checkstring(argv[0], "GREEN"))
            iret = RGB_GREEN;
        else if (checkstring(argv[0], "CYAN"))
            iret = RGB_CYAN;
        else if (checkstring(argv[0], "RED"))
            iret = RGB_RED;
        else if (checkstring(argv[0], "MAGENTA"))
            iret = RGB_MAGENTA;
        else if (checkstring(argv[0], "YELLOW"))
            iret = RGB_YELLOW;
        else if (checkstring(argv[0], "BROWN"))
            iret = RGB_BROWN;
        else if (checkstring(argv[0], "GRAY"))
            iret = RGB_GRAY;
        else if (checkstring(argv[0], "GREY"))
            iret = RGB_GRAY;
        else if (checkstring(argv[0], "LIGHTGRAY"))
            iret = RGB_LITEGRAY;
        else if (checkstring(argv[0], "LIGHTGRAY"))
            iret = RGB_LITEGRAY;
        else if (checkstring(argv[0], "ORANGE"))
            iret = RGB_ORANGE;
        else if (checkstring(argv[0], "PINK"))
            iret = RGB_PINK;
        else if (checkstring(argv[0], "GOLD"))
            iret = RGB_GOLD;
        else if (checkstring(argv[0], "SALMON"))
            iret = RGB_SALMON;
        else if (checkstring(argv[0], "BEIGE"))
            iret = RGB_BEIGE;
        else
            error("Invalid colour: $", argv[0]);
        iret &= 0xFFFFFF;
        if (VideoColour == 12) iret |= (getint(argv[2], 0, 15) << 24);
        if (VideoColour == 32) iret |= (getint(argv[2], 0, 255) << 24);
    } else if (argc == 7) {
        if (VideoColour == 8 || VideoColour == 16)
            error("Transparency not valid for this mode");
        iret = rgb(getint(argv[0], 0, 255), getint(argv[2], 0, 255),
                   getint(argv[4], 0, 255),
                   (VideoColour == 12 ? getint(argv[6], 0, 15)
                                      : getint(argv[6], 0, 255)));
    } else
        error("Syntax");
    targ = T_INT;
}
#endif