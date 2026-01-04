/*-*****************************************************************************

MMBasic for Linux (MMB4L)

graphics.h

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

#if !defined(MMCOLOUR_H)
#define MMCOLOUR_H

#include <stdint.h>

// alpha == 0xFF is fully opaque.
#define RGB(red, green, blue, alpha) (uint32_t) (((alpha & 0xFF) << 24) | ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF))

#define RGB_BLACK     RGB(   0,    0,    0,    0)
#define RGB_BLUE      RGB(   0,    0, 0xFF, 0xFF)
#define RGB_GREEN     RGB(   0, 0xFF,    0, 0xFF)
#define RGB_CYAN      RGB(   0, 0xFF, 0xFF, 0xFF)
#define RGB_RED       RGB(0xFF,    0,    0, 0xFF)
#define RGB_MAGENTA   RGB(0xFF,    0, 0xFF, 0xFF)
#define RGB_YELLOW    RGB(0xFF, 0xFF,    0, 0xFF)
#define RGB_BROWN     RGB(0xA5, 0x2A, 0x2A, 0xFF)
#define RGB_GREY      RGB(0x40, 0x40, 0x40, 0xFF)
#define RGB_GRAY      RGB_GREY
#define RGB_LITEGREY  RGB(0x80, 0x80, 0x80, 0xFF)
#define RGB_LITEGRAY  RGB_LITEGREY
#define RGB_WHITE     RGB(0xFF, 0xFF, 0xFF, 0xFF)
#define RGB_ORANGE    RGB(0xFF, 0xA5,    0, 0xFF)
#define RGB_PINK      RGB(0xFF, 0xA0, 0xAB, 0xFF)
#define RGB_GOLD      RGB(0xFF, 0xD7,    0, 0xFF)
#define RGB_SALMON    RGB(0xFA, 0x80, 0x72, 0xFF)
#define RGB_BEIGE     RGB(0xF5, 0xF5, 0xDC, 0xFF)
#define RGB_NOTBLACK  RGB(   0,    0,    0, 0xFF)

// Additional 4-bit colours defined on the PicoMite.
#define RGB_MYRTLE        RGB(   0, 0x40,    0, 0xFF)
#define RGB_COBALT        RGB(   0, 0x40, 0xFF, 0xFF)
#define RGB_MIDGREEN      RGB(   0, 0x80,    0, 0xFF)
#define RGB_CERULEAN      RGB(   0, 0x80, 0xFF, 0xFF)
#define RGB_MAGENTA_4BIT  RGB(0xFF,    0, 0xFF, 0xFF)
#define RGB_RUST          RGB(0xFF, 0x40,    0, 0xFF)
#define RGB_FUCHSIA       RGB(0xFF, 0x40, 0xFF, 0xFF)
#define RGB_BROWN_4BIT    RGB(0xFF, 0x80,    0, 0xFF)
#define RGB_LILAC         RGB(0xFF, 0x80, 0xFF, 0xFF)

// Additional ANSI colours (Windows Console colour scheme).
#define RGB_ANSI_DEFAULT         -2
#define RGB_ANSI_BLACK           RGB_BLACK
#define RGB_ANSI_RED             RGB(0x80,    0,    0, 0xFF)
#define RGB_ANSI_GREEN           RGB(   0, 0x80,    0, 0xFF)
#define RGB_ANSI_YELLOW          RGB(0x80, 0x80,    0, 0xFF)
#define RGB_ANSI_BLUE            RGB(   0,    0, 0x80, 0xFF)
#define RGB_ANSI_MAGENTA         RGB(0x80,    0, 0x80, 0xFF)
#define RGB_ANSI_CYAN            RGB(   0, 0x80, 0x80, 0xFF)
#define RGB_ANSI_WHITE           RGB(0xC0, 0xC0, 0xC0, 0xFF)
#define RGB_ANSI_BRIGHT_BLACK    RGB_LITEGREY
#define RGB_ANSI_BRIGHT_RED      RGB_RED
#define RGB_ANSI_BRIGHT_GREEN    RGB_GREEN
#define RGB_ANSI_BRIGHT_YELLOW   RGB_YELLOW
#define RGB_ANSI_BRIGHT_BLUE     RGB_BLUE
#define RGB_ANSI_BRIGHT_MAGENTA  RGB_MAGENTA
#define RGB_ANSI_BRIGHT_CYAN     RGB_CYAN
#define RGB_ANSI_BRIGHT_WHITE    RGB_WHITE

// 32-bit colour
// -1 for transparent background colour.
// -2 for default terminal colour (background or foreground depending on context).
typedef int64_t MmGraphicsColour;

#endif // #if !defined(MMCOLOUR_H)
