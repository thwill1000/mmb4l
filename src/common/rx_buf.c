/*-*****************************************************************************

MMBasic for Linux (MMB4L)

rx_buf.c

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

#include <stdio.h>

#include "rx_buf.h"

void rx_buf_init(RxBuf *buf, char *data, int data_sz) {
    buf->head = 0;
    buf->tail = 0;
    buf->data = data;
    buf->data_sz = data_sz;
}

void rx_buf_clear(RxBuf *buf) {
    buf->head = 0;
    buf->tail = 0;
}

int rx_buf_get(RxBuf *buf) {
    if (buf->head == buf->tail) {
        return -1;
    } else {
        int result = buf->data[buf->tail++];
        buf->tail %= buf->data_sz;
        return result;
    }
}

int rx_buf_put(RxBuf *buf, char ch) {
    buf->data[buf->head++] = ch;
    buf->head %= buf->data_sz;

    // If the buffer has overflowed then discard the oldest character.
    if (buf->head == buf->tail) {
        buf->tail = (buf->tail + 1) % buf->data_sz;
        return -1;
    } else {
        return 0;
    }
}

int rx_buf_size(RxBuf *buf) {
    if (buf->tail > buf->head) {
        return buf->data_sz + (buf->head - buf->tail);
    } else {
        return buf->head - buf->tail;
    }
}

int rx_buf_unget(RxBuf *buf, char ch) {
    buf->tail--;
    if (buf->tail < 0) buf->tail = buf->data_sz - 1;
    buf->data[buf->tail] = ch;
    if (buf->head == buf->tail) {
        buf->head--;
        if (buf->head < 0) buf->head = buf->data_sz - 1;
        return -1;
    } else {
        return 0;
    }
}

void rx_buf_dump(RxBuf *buf) {
    printf("\n");
    for (int i = 0; i < buf->data_sz; ++i) {
        if (i > 0) printf(", ");
        if (buf->data[i] > 31 && buf->data[i] < 128) {
            printf("'%c'", buf->data[i]);
        } else {
            printf("<0x%02X>", buf->data[i]);
        }
        if (i == buf->head) printf("H");
        if (i == buf->tail) printf("T");
    }
    printf("\n");
}
