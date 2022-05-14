/*-*****************************************************************************

MMBasic for Linux (MMB4L)

rx_buf.h

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

#if !defined(MMB4L_RX_BUFFER_H)
#define MMB4L_RX_BUFFER_H

typedef struct {
    int head;     // next write goes here.
    int tail;     // next read comes from here.
    char *data;
    int data_sz;  // capacity will be 1 less than this.
} RxBuf;

/**
 * Initialises an RxBuf structure.
 *
 * @param  buf      pointer to the buffer.
 * @param  data     character buffer to encapsulate.
 * @param  data_sz  size of 'data' buffer,
 *                  the capacity of the RxBuf will be one less.
 */
void rx_buf_init(RxBuf *buf, char *data, int data_sz);

/**
 * Clears the buffer.
 *
 * @param  buf  pointer to the buffer
 */
void rx_buf_clear(RxBuf *buf);

/**
 * Gets a character from the buffer.
 *
 * @param  buf  pointer to the buffer.
 * @return      the next character, or -1 if the buffer is empty.
 */
int rx_buf_get(RxBuf *buf);

/**
 * Puts a character on the end of the buffer.
 * If the buffer was full then the oldest character is discarded.
 *
 * @param  buf  pointer to the buffer.
 * @param  ch   the character.
 * @return      0 if the buffer was not full,
 *              -1 if the buffer was full; the oldest character is discarded.
 */
int rx_buf_put(RxBuf *buf, char ch);

/**
 * Gets the number of items in the buffer.
 *
 * @param  buf  pointer to the buffer
 * @return      the number of items in the buffer.
 */
int rx_buf_size(RxBuf *buf);

/**
 * Puts a character back onto the front of the buffer.
 *
 * @param  buf  pointer to the buffer.
 * @param  ch   the character.
 * @return      0 if the buffer was not full,
 *              -1 if the buffer was full; the newest character is discarded
 *              to make way for the 'ungot' character.
 */
int rx_buf_unget(RxBuf *buf, char ch);

/**
 * Writes the contents of the buffer to STDOUT.
 *
 * @param  buf  pointer to the buffer
 */
void rx_buf_dump(RxBuf *buf);

#endif
