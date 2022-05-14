/*-*****************************************************************************

MMBasic for Linux (MMB4L)

Operators.c

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

// Provides all the operator functions used in MMBasic, i.e. +, -, *, etc.

#include "../Hardware_Includes.h"
#include "MMBasic_Includes.h"

/********************************************************************************************************************************************
 basic operators
 each function is responsible for decoding a basic operator
 all function names are in the form op_xxxx() so, if you want to search for the function responsible for the AND operator look for op_and

 There are 5 globals used by these finctions:

 farg1, farg2   These are the floating point arguments to the operator.  farg1 is the left argument

 sarg1, sarg2   These are the string pointers to the arguments for a the string operator.  sarg1 is the left argument

 fret           Is the return value for a basic operator that returns a float value

 iret           Is the return value for a basic operator that returns an integer

 sret           Is the return value for a basic operator that returns a string

 targ           Is the type of the arguments.  normally this is set by the caller and is not changed by the function

 ********************************************************************************************************************************************/




void op_invalid(void) {
  error("Syntax error");
}


void op_exp(void) {
    long long int i;
    errno = 0;
    if(targ & T_NBR)
        fret = (MMFLOAT)powf(farg1, farg2);
    else {
        if(iarg2 < 0) {
            targ = T_NBR;
            fret = (MMFLOAT)powf((MMFLOAT)iarg1, (MMFLOAT)iarg2);
        } else
            for(iret = i = 1; i <= iarg2; i++) iret *= iarg1;
    }
    if(errno) error("Overflow");
}


void op_mul(void) {
    if(targ & T_NBR)
        fret = farg1 * farg2;
    else
        iret = iarg1 * iarg2;
}


// division will always return a float even if given integer arguments
void op_div(void) {
    if(farg2 == 0) error("Divide by zero");
    fret = farg1 / farg2;
    targ = T_NBR;
}


void op_divint(void) {
    if(iarg2 == 0) error("Divide by zero");
    iret = iarg1 / iarg2;
}


void op_add(void) {
  if(targ & T_NBR)
      fret = farg1 + farg2;
  else if(targ & T_INT)
      iret = iarg1 + iarg2;
    else {
      if(*sarg1 + *sarg2 > MAXSTRLEN) error("String too long");
      sret = GetTempStrMemory();                                    // this will last for the life of the command
      Mstrcpy(sret, sarg1);
      Mstrcat(sret, sarg2);
  }
}



void op_subtract(void) {
  if(targ & T_NBR)
      fret = farg1 - farg2;
  else
      iret = iarg1 - iarg2;
}


void op_mod(void) {
    if(iarg2 == 0) error("Divide by zero");
    iret = iarg1 % iarg2;
}



long long int compare(void) {
    long long int r;
    MMFLOAT f;
    if(targ & T_NBR) {
      f = farg1 - farg2;
        if(f > 0)
            r = 1;
        else if(f < 0)
            r = -1;
        else
            r = 0;
    }
  else
        if(targ & T_INT)
            r = iarg1 - iarg2;
        else
            r = Mstrcmp(sarg1, sarg2);
     targ = T_INT;                                                  // always return an float, even if the args are string
     return r;
}


void op_ne(void) {
    if(targ & T_INT)
        iret = iarg1 != iarg2;
    else
        iret = (compare() != 0);
}



void op_gte(void) {
    iret = (compare() >= 0);
}


void op_lte(void) {
    iret = (compare() <= 0);
}


void op_lt(void) {
    iret = (compare() < 0);
}


void op_gt(void) {
    iret = (compare() > 0);
}


void op_equal(void) {
    if(targ & T_INT)
        iret = iarg1 == iarg2;
    else
        iret = (compare() == 0);
}


void op_shiftleft(void) {
    iret = (long long int)((unsigned long long int)iarg1 << (long long int)iarg2);
}


void op_shiftright(void) {
    iret = (long long int)((unsigned long long int)iarg1 >> (long long int)iarg2);
}


void op_and(void) {
    iret = (long long int)((unsigned long long int)iarg1 & (unsigned long long int)iarg2);
}


void op_or(void) {
    iret = (long long int)((unsigned long long int)iarg1 | (unsigned long long int)iarg2);
}


void op_xor(void) {
    iret = (long long int)((unsigned long long int)iarg1 ^ (unsigned long long int)iarg2);
}


void op_not(void){
  // don't do anything, just a place holder
  error("Syntax error");
}

