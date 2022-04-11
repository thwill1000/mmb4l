#include "../common/mmb4l.h"

static void fun_chr_ascii(char *p) {
    int i = getint(p, 0, 0xFF);
    sret = GetTempStrMemory();
    targ = T_STR;
    sret[0] = 1;
    sret[1] = i;
}

static void fun_chr_utf8(char *p) {
    int utf = getint(p, 0, 0x10FFFF);
    targ = T_STR;
    sret = GetTempStrMemory();

    if (utf <= 0x7F) {
        // Plain ASCII
        sret[0] = 1;
        sret[1] = (char) utf;
    } else if (utf <= 0x07FF) {
        // 2-byte unicode
        sret[0] = 2;
        sret[1] = (char) (((utf >> 6) & 0x1F) | 0xC0);
        sret[2] = (char) (((utf >> 0) & 0x3F) | 0x80);
    } else if (utf <= 0xFFFF) {
        // 3-byte unicode
        sret[0] = 3;
        sret[1] = (char) (((utf >> 12) & 0x0F) | 0xE0);
        sret[2] = (char) (((utf >> 6) & 0x3F) | 0x80);
        sret[3] = (char) (((utf >> 0) & 0x3F) | 0x80);
    } else if (utf <= 0x10FFFF) {
        // 4-byte unicode
        sret[0] = 4;
        sret[1] = (char) (((utf >> 18) & 0x07) | 0xF0);
        sret[2] = (char) (((utf >> 12) & 0x3F) | 0x80);
        sret[3] = (char) (((utf >> 6) & 0x3F) | 0x80);
        sret[4] = (char) (((utf >> 0) & 0x3F) | 0x80);
    } else {
        // error - use replacement character
        sret[0] = 3;
        sret[1] = (char) 0xEF;
        sret[2] = (char) 0xBF;
        sret[3] = (char) 0xBD;
    }
}

void fun_chr(void) {
    char* p;
    if ((p = checkstring(ep, "UTF8"))) {
        fun_chr_utf8(p);
    } else {
        fun_chr_ascii(ep);
    }
}
