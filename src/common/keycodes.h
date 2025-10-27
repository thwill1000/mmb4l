/*-*****************************************************************************

MMBasic for Linux (MMB4L)

keycodes.h

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#if !defined(KEYCODES_H)
#define KEYCODES_H

// the values returned by the standard control keys
#define TAB 0x9
#define BKSP 0x8
#define ENTER 0xa
#define ESC 0x1b
#define BREAK 0x9e
#define PSCRN 0x9d

// the values returned by the function keys
#define F1 0x91
#define F2 0x92
#define F3 0x93
#define F4 0x94
#define F5 0x95
#define F6 0x96
#define F7 0x97
#define F8 0x98
#define F9 0x99
#define F10 0x9a
#define F11 0x9b
#define F12 0x9c

// the values returned by special control keys
#define UP 0x80
#define DOWN 0x81
#define LEFT 0x82
#define RIGHT 0x83
#define INSERT 0x84
#define DEL 0x7f
#define HOME 0x86
#define END 0x87
#define PUP 0x88
#define PDOWN 0x89
#define NUM_ENT ENTER
#define SLOCK 0x8c
#define ALT 0x8b

// Shifted values
#define STAB    0x9F
#define SDEL    0xA0
#define SDOWN   0xA1
#define SRIGHT  0xA3

#define CTRLKEY(key)   ((key) & 0x1f)
#define SHIFT_FN(key)  ((key) + 0x20)

#endif // #if !defined(KEYCODES_H)
