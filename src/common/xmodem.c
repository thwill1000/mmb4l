/*-*****************************************************************************

MMBasic for Linux (MMB4L)

xmodem.c

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

#include <stddef.h>

#include "mmb4l.h"
#include "console.h"
#include "error.h"
#include "file.h"
#include "mmtime.h"
#include "serial.h"
#include "xmodem.h"

/*
 * Derived from the work of Georges Menie (www.menie.org) Copyright 2001-2010
 * Georges Menie very much debugged and changed
 *
 * this is just the basic XModem protocol (no 1K blocks, crc, etc).  It has been
 * tested on Terra Term and is intended for use with that software.
 */

#define X_BLOCK_SIZE  128
#define X_BUF_SIZE  X_BLOCK_SIZE + 6  // 128 for XModem + 3 head chars + 2 crc + nul

#define SOH 0x01
#define STX 0x02
#define EOT 0x04
#define ACK 0x06
#define NAK 0x15
#define CAN 0x18
#define PAD 0x1a

#define DLY_1S 1000
#define MAXRETRANS 25

#define ERROR_CANCELLED         error_throw_ex(kError, "Cancelled by remote")
#define ERROR_CLOSING           error_throw_ex(kError, "Error closing")
#define ERROR_NO_RESPONSE       error_throw_ex(kError, "Remote did not respond")
#define ERROR_TOO_MANY_ERRORS   error_throw_ex(kError, "Too many errors")

static int xmodem_check(const unsigned char *buf, int sz) {
    unsigned char cks = 0;
    for (int i = 0; i < sz; ++i) cks += buf[i];
    return cks == buf[sz];
}

static int _inbyte(int timeout_ms, int serial_fnbr) {
    int64_t expiry_ns = mmtime_now_ns() + MILLISECONDS_TO_NANOSECONDS(timeout_ms);
    while (mmtime_now_ns() < expiry_ns) {
        int ch = serial_getc(serial_fnbr);
        if (ch != -1) return ch;
    }
    return -1;
}

static void flushinput(int serial_fnbr) {
    while (_inbyte(((DLY_1S)*3) >> 1, serial_fnbr) >= 0) {
        // Do nothing.
    }
}

static inline void xmodem_putc(int serial_fnbr, char ch) {
    serial_putc(serial_fnbr, ch);
}

void xmodem_transmit(int file_fnbr, int serial_fnbr, bool verbose) {
    char xbuff[X_BUF_SIZE];
    unsigned char packetno = 1;
    char prevchar = 0;
    int i, c, len, total = 0;
    int retry;
    char sbuf[128];

    // first establish communication with the remote
    while (1) {
        for (retry = 0; retry < 32; ++retry) {
            if ((c = _inbyte((DLY_1S) << 1, serial_fnbr)) >= 0) {
                switch (c) {
                    case NAK:  // start sending
                        goto start_trans;
                    case CAN:
                        if ((c = _inbyte(DLY_1S, serial_fnbr)) == CAN) {
                            xmodem_putc(serial_fnbr, ACK);
                            flushinput(serial_fnbr);
                            ERROR_CANCELLED;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        xmodem_putc(serial_fnbr, CAN);
        xmodem_putc(serial_fnbr, CAN);
        xmodem_putc(serial_fnbr, CAN);
        flushinput(serial_fnbr);
        ERROR_NO_RESPONSE;  // no sync

        // send a packet
        while (1) {
        start_trans:
            memset(xbuff, 0, X_BUF_SIZE);  // start with an empty buffer

            xbuff[0] = SOH;  // copy the header
            xbuff[1] = packetno;
            xbuff[2] = ~packetno;

            if (verbose) {
                if (total > 0) console_cursor_up(1);
                sprintf(sbuf, "Sent %d bytes\n", total);
                console_puts(sbuf);
            }

            // Copy data from the file into the packet.
            len = file_read(file_fnbr, xbuff + 3, 128);
//            for (len = 0; len < 128 && !file_eof(file_fnbr); len++) {
//                xbuff[len + 3] = file_getc(file_fnbr);
//            }

            if (len > 0) {
                unsigned char ccks = 0;
                for (i = 3; i < X_BLOCK_SIZE + 3; ++i) {
                    ccks += xbuff[i];
                }
                xbuff[X_BLOCK_SIZE + 3] = ccks;

                // now send the block
                for (retry = 0; retry < MAXRETRANS && !MMAbort; ++retry) {
                    // send the block
                    serial_write(serial_fnbr, xbuff, X_BLOCK_SIZE + 4);
                    //for (i = 0; i < X_BLOCK_SIZE + 4 && !MMAbort; ++i) {
                    //    xmodem_putc(serial_fnbr, xbuff[i]);
                    //}
                    // check the response
                    if ((c = _inbyte(DLY_1S, serial_fnbr)) >= 0) {
                        switch (c) {
                            case ACK:
                                ++packetno;
                                total += len;
                                goto start_trans;
                            case CAN:  // cancelled by remote
                                xmodem_putc(serial_fnbr, ACK);
                                flushinput(serial_fnbr);
                                ERROR_CANCELLED;
                                break;
                            case NAK:  // receiver got a corrupt block
                            default:
                                break;
                        }
                    }
                }
                // too many retrys... give up
                xmodem_putc(serial_fnbr, CAN);
                xmodem_putc(serial_fnbr, CAN);
                xmodem_putc(serial_fnbr, CAN);
                flushinput(serial_fnbr);
                ERROR_TOO_MANY_ERRORS;
            }

            // finished sending - send end of text
            else {
                for (retry = 0; retry < 10; ++retry) {
                    xmodem_putc(serial_fnbr, EOT);
                    if ((c = _inbyte((DLY_1S) << 1, serial_fnbr)) == ACK) break;
                }
                flushinput(serial_fnbr);
                if (c == ACK) return;
                ERROR_CLOSING;
            }
        }
    }
}

void xmodem_receive(int file_fnbr, int serial_fnbr, bool verbose) {
    unsigned char xbuff[X_BUF_SIZE];
    unsigned char *p;
    unsigned char trychar = NAK;  //'C';
    unsigned char packetno = 1;
    int i, c, total = 0;
    int retry, retrans = MAXRETRANS;
    char sbuf[128];

    // first establish communication with the remote
    while (1) {
        if (verbose) {
            if (total > 0) console_cursor_up(1);
            sprintf(sbuf, "Received %d bytes\n", total);
            console_puts(sbuf);
        }

        for (retry = 0; retry < 32; ++retry) {
            if (trychar) xmodem_putc(serial_fnbr, trychar);
            if ((c = _inbyte((DLY_1S) << 1, serial_fnbr)) >= 0) {
                switch (c) {
                    case SOH:
                        goto start_recv;
                    case EOT:
                        flushinput(serial_fnbr);
                        xmodem_putc(serial_fnbr, ACK);
                        return;  // no more data
                    case CAN:
                        flushinput(serial_fnbr);
                        xmodem_putc(serial_fnbr, ACK);
                        ERROR_CANCELLED;
                        break;
                    default:
                        break;
                }
            }
        }
        flushinput(serial_fnbr);
        xmodem_putc(serial_fnbr, CAN);
        xmodem_putc(serial_fnbr, CAN);
        xmodem_putc(serial_fnbr, CAN);
        ERROR_NO_RESPONSE;

    start_recv:
        trychar = 0;
        p = xbuff;
        *p++ = SOH;
        for (i = 0; i < (X_BLOCK_SIZE + 3); ++i) {
            if ((c = _inbyte(DLY_1S, serial_fnbr)) < 0) goto reject;
            *p++ = c;
        }
        if (xbuff[1] == (unsigned char)(~xbuff[2]) &&
            (xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno - 1) &&
            xmodem_check(&xbuff[3], X_BLOCK_SIZE)) {
            if (xbuff[1] == packetno) {
                for (i = 0; i < X_BLOCK_SIZE; i++) {
                    file_putc(file_fnbr, xbuff[i + 3]);
                }
                ++packetno;
                retrans = MAXRETRANS + 1;
                total += X_BLOCK_SIZE;
            }
            if (--retrans <= 0) {
                flushinput(serial_fnbr);
                xmodem_putc(serial_fnbr, CAN);
                xmodem_putc(serial_fnbr, CAN);
                xmodem_putc(serial_fnbr, CAN);
                ERROR_TOO_MANY_ERRORS;
            }
            xmodem_putc(serial_fnbr, ACK);
            continue;
        }
    reject:
        flushinput(serial_fnbr);
        xmodem_putc(serial_fnbr, NAK);
    }
}
