/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_chr.c

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
