/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_str2bin.c

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

void fun_str2bin(void) {
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
    int j;
    getargs(&ep, 5, ",");
    if (!(argc == 3 || argc == 5)) ERROR_SYNTAX;
    if (argc == 5 && !checkstring(argv[4], "BIG")) ERROR_SYNTAX;
    char *p;
    p = (char *)getstring(argv[2]);
    int len = p[0];
    map.l = 0;
    for (j = 0; j < len; j++) map.c[j] = p[j + 1];
    if (argc == 5) {  // big endian so swap byte order
        char k;
        int m;
        for (j = 0; j < (len >> 1); j++) {
            m = len - j - 1;
            k = map.c[j];
            map.c[j] = map.c[m];
            map.c[m] = k;
        }
    }
    if (checkstring(argv[0], "DOUBLE")) {
        if (len != 8) ERROR_STRING_LENGTH;
        targ = T_NBR;
        fret = (MMFLOAT)map.d;
    } else if (checkstring(argv[0], "SINGLE")) {
        if (len != 4) ERROR_STRING_LENGTH;
        targ = T_NBR;
        fret = (MMFLOAT)map.f;
    } else if (checkstring(argv[0], "INT64")) {
        if (len != 8) ERROR_STRING_LENGTH;
        targ = T_INT;
        iret = (int64_t)map.l;
    } else if (checkstring(argv[0], "INT32")) {
        if (len != 4) ERROR_STRING_LENGTH;
        targ = T_INT;
        iret = (int64_t)map.i;
    } else if (checkstring(argv[0], "INT16")) {
        if (len != 2) ERROR_STRING_LENGTH;
        targ = T_INT;
        iret = (int64_t)map.s;
    } else if (checkstring(argv[0], "INT8")) {
        if (len != 1) ERROR_STRING_LENGTH;
        targ = T_INT;
        iret = (int64_t)map.c[0];
    } else if (checkstring(argv[0], "UINT64")) {
        if (len != 8) ERROR_STRING_LENGTH;
        targ = T_INT;
        iret = (int64_t)map.ul;
    } else if (checkstring(argv[0], "UINT32")) {
        if (len != 4) ERROR_STRING_LENGTH;
        targ = T_INT;
        iret = (int64_t)map.ui;
    } else if (checkstring(argv[0], "UINT16")) {
        if (len != 2) ERROR_STRING_LENGTH;
        targ = T_INT;
        iret = (int64_t)map.us;
    } else if (checkstring(argv[0], "UINT8")) {
        if (len != 1) ERROR_STRING_LENGTH;
        targ = T_INT;
        iret = (int64_t)map.uc[0];
    } else {
        ERROR_SYNTAX;
    }
}
