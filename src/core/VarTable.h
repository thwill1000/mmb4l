/*-*****************************************************************************

MMBasic for Linux (MMB4L)

VarTable.h

Copyright 2011-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#if !defined(MMB4L_VAR_TABLE_H)
#define MMB4L_VAR_TABLE_H

#include "../Configuration.h"

// TODO: change to int16_t
#define DIMTYPE  short int

struct s_vartbl {                                     // structure of the variable table
    char name[MAXVARLEN];                             // variable's name
    char type;                                        // its type (T_NBR, T_INT or T_STR)
    char level;                                       // its subroutine or function level (used to track local variables)
    DIMTYPE dims[MAXDIM];                             // the dimensions. it is an array if the first dimension is NOT zero
    unsigned char size;                               // the number of chars to allocate for each element in a string array
    union u_val {
        MMFLOAT f;                                    // the value if it is a float
        MMINTEGER i;                                  // the value if it is an integer
        MMFLOAT *fa;                                  // pointer to the allocated memory if it is an array of floats
        MMINTEGER *ia;                                // pointer to the allocated memory if it is an array of integers
        char *s;                                      // pointer to the allocated memory if it is a string
    } __attribute__ ((aligned (8))) val;
};

#endif // #if !defined(MMB4L_VAR_TABLE_H)
