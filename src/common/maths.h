/*-*****************************************************************************

MMBasic for Linux (MMB4L)

maths.h

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

#if !defined(MMB4L_MATHS_H)
#define MMB4L_MATHS_H

#include <stdint.h>

#include "../Configuration.h"

extern MMFLOAT optionangle;
extern const double chitable[51][15];

MMFLOAT *alloc1df(int n);
MMFLOAT **alloc2df(int m, int n);
void cofactor(MMFLOAT **matrix, MMFLOAT **newmatrix, int size);
void dealloc2df(MMFLOAT **array, int m, int n);
MMFLOAT determinant(MMFLOAT **matrix, int size);
void floatshellsort(MMFLOAT a[], int n);
void PRet(void);
void PFlt(MMFLOAT flt);
void PFltComma(MMFLOAT n);
void PInt(int64_t n);
void PIntComma(int64_t n);
void Q_Invert(MMFLOAT *q, MMFLOAT *n);
void Q_Mult(MMFLOAT *q1, MMFLOAT *q2, MMFLOAT *n);

void maths_fft(const char *pp);

#endif
