/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_bin2str.c

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

#include "../common/error.h"
#include "../common/mmb4l.h"

void fun_bin2str(void) {
    int j, len = 0;
    union binmap {
        int8_t c[8];
        uint8_t uc[8];
        float f;
        double d;
        int64_t l;
        uint64_t ul;
        int i;
        uint32_t ui;
        short s;
        uint16_t us;
    } map;
    int64_t i64;
    getargs(&ep, 5, ",");
    if (!(argc == 3 || argc == 5)) ERROR_SYNTAX;
    if (argc == 5 && !(checkstring(argv[4], "BIG"))) ERROR_SYNTAX;
    sret = GetTempMemory(STRINGSIZE);
    if (checkstring(argv[0], "DOUBLE")) {
        len = 8;
        map.d = (double)getnumber(argv[2]);
    } else if (checkstring(argv[0], "SINGLE")) {
        len = 4;
        map.f = (float)getnumber(argv[2]);
    } else {
        i64 = getinteger(argv[2]);
        if (checkstring(argv[0], "INT64")) {
            len = 8;
            map.l = (int64_t)i64;
        } else if (checkstring(argv[0], "INT32")) {
            len = 4;
            if (i64 > 2147483647 || i64 < -2147483648) ERROR_OVERFLOW;
            map.i = (int32_t)i64;
        } else if (checkstring(argv[0], "INT16")) {
            len = 2;
            if (i64 > 32767 || i64 < -32768) ERROR_OVERFLOW;
            map.s = (int16_t)i64;
        } else if (checkstring(argv[0], "INT8")) {
            len = 1;
            if (i64 > 127 || i64 < -128) ERROR_OVERFLOW;
            map.c[0] = (int8_t)i64;
        } else if (checkstring(argv[0], "UINT64")) {
            len = 8;
            map.ul = (uint64_t)i64;
        } else if (checkstring(argv[0], "UINT32")) {
            len = 4;
            if (i64 > 4294967295 || i64 < 0) ERROR_OVERFLOW;
            map.ui = (uint32_t)i64;
        } else if (checkstring(argv[0], "UINT16")) {
            len = 2;
            if (i64 > 65535 || i64 < 0) ERROR_OVERFLOW;
            map.us = (uint16_t)i64;
        } else if (checkstring(argv[0], "UINT8")) {
            len = 1;
            if (i64 > 255 || i64 < 0) ERROR_OVERFLOW;
            map.uc[0] = (uint8_t)i64;
        } else
            ERROR_SYNTAX;
    }

    for (j = 0; j < len; j++) sret[j] = map.c[j];

    if (argc == 5) {  // big endian so swap byte order
        unsigned char k;
        int m;
        for (j = 0; j < (len >> 1); j++) {
            m = len - j - 1;
            k = sret[j];
            sret[j] = sret[m];
            sret[m] = k;
        }
    }
    // convert from c type string but it can contain zeroes
    char *p1, *p2;
    j = len;
    p1 = sret + len;
    p2 = sret + len - 1;
    while (j--) *p1-- = *p2--;
    *sret = len;
    targ = T_STR;
}
