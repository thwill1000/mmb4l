/*-*****************************************************************************

MMBasic for Linux (MMB4L)

complex_compat.h

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
   be displayed on the console at startup (additional copyright messages may
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

#ifndef COMPLEX_COMPAT_H
#define COMPLEX_COMPAT_H

#if defined(_MSC_VER)

#define _CRT_USE_C_COMPLEX_H
#include <complex.h>
#define _USE_MATH_DEFINES
#include <math.h>

typedef _Dcomplex cplx;  // double_complex
typedef _Fcomplex fcplx; // float_complex

/* MSVC does not support C99 complex arithmetic operators.
 * These macros provide equivalents for +, -, *, / on complex types. */

/* Double precision */
#define cadd(a, b)    _Cbuild(creal(a) + creal(b), cimag(a) + cimag(b))
#define csub(a, b)    _Cbuild(creal(a) - creal(b), cimag(a) - cimag(b))
#define cmul(a, b)    _Cmulcc(a, b)
#define cdiv(a, b)    _Cdivcc(a, b)
#define cdivr(a, r)   _Cbuild(creal(a) / (r), cimag(a) / (r))  /* divide by real scalar */

/* Single precision */
#define caddf(a, b)   _FCbuild(crealf(a) + crealf(b), cimagf(a) + cimagf(b))
#define csubf(a, b)   _FCbuild(crealf(a) - crealf(b), cimagf(a) - cimagf(b))
#define cmulf(a, b)   _FCmulcc(a, b)
#define cdivf(a, b)   _FCbuild( \
    (crealf(a) * crealf(b) + cimagf(a) * cimagf(b)) / (crealf(b) * crealf(b) + cimagf(b) * cimagf(b)), \
    (cimagf(a) * crealf(b) - crealf(a) * cimagf(b)) / (crealf(b) * crealf(b) + cimagf(b) * cimagf(b)))
#define cdivrf(a, r)  _FCbuild(crealf(a) / (r), cimagf(a) / (r))  /* divide by real scalar */

#define FCOMPLEX(r, i)  _FCbuild((float)(r), (float)(i))
#define DCOMPLEX(r, i)  _Cbuild((double)(r), (double)(i))

#else

#include <complex.h>
#include <math.h>

#include "../Configuration.h"  // for MMFLOAT definition

typedef MMFLOAT complex cplx;
typedef float complex fcplx;

/* On GCC/Clang, operators work natively so these macros just use them directly. */
#define cadd(a, b)    ((a) + (b))
#define csub(a, b)    ((a) - (b))
#define cmul(a, b)    ((a) * (b))
#define cdiv(a, b)    ((a) / (b))
#define cdivr(a, r)   ((a) / (r))

#define caddf(a, b)   ((a) + (b))
#define csubf(a, b)   ((a) - (b))
#define cmulf(a, b)   ((a) * (b))
#define cdivf(a, b)   ((a) / (b))
#define cdivrf(a, r)  ((a) / (r))

#define FCOMPLEX(r, i)  ((float)(r) + (float)(i) * I)
#define DCOMPLEX(r, i)  ((double)(r) + (double)(i) * I)

#endif /* _MSC_VER */
#endif /* COMPLEX_COMPAT_H */
