/*-*****************************************************************************

MMBasic for Linux (MMB4L)

Functions.c

Copyright 2011-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

// Provides all the core functions in MMBasic.

#include "Functions.h"
#include "MMBasic.h"
#include "tokentbl.h"
#include "../Hardware_Includes.h"
#include "../Version.h"
#include "../common/console.h"
#include "../common/error.h"
#include "../common/memory.h"
#include "../common/options.h"

/********************************************************************************************************************************************
 basic functions
 each function is responsible for decoding a basic function
 all function names are in the form fun_xxxx() so, if you want to search for the function responsible for the ASC() function look for fun_asc

 There are 4 globals used by these functions:

 char *ep       This is a pointer to the argument of the function
                Eg, in the case of INT(35/7) ep would point to "35/7)"

 fret           Is the return value for a basic function that returns a float

 iret           Is the return value for a basic function that returns an integer

 sret           Is the return value for a basic function that returns a string

 tret           Is the type of the return value.  normally this is set by the caller and is not changed by the function

 ********************************************************************************************************************************************/



// return the absolute value of a number (ie, without the sign)
// a = ABS(nbr)
void fun_abs(void) {
    char *s;
    MMFLOAT f;
    MMINTEGER i64;

    targ = T_INT;
    (void) evaluate(ep, &f, &i64, &s, &targ, false);                 // get the value and type of the argument
    if (targ & T_NBR) {
        fret = fabsf(f);
    } else {
        iret = i64;
        if(iret < 0) iret = -iret;
    }
}



// return the ASCII value of the first character in a string (ie, its number value)
// a = ASC(str$)
void fun_asc(void) {
    char *s;

    s = getstring(ep);
    if(*s == 0)
        iret = 0;
    else
        iret = *(s + 1);
    targ = T_INT;
}



// return the arctangent of a number in radians
void fun_atn(void) {
    fret = atanf(getnumber(ep)) * ANGLE_CONVERSION;
    targ = T_NBR;
}



// Round numbers with fractional portions up or down to the next whole number or integer.
void fun_cint(void) {
    iret = getinteger(ep);
    targ = T_INT;
}



// return the cosine of a number in radians
void fun_cos(void) {
    fret = cosf(getnumber(ep) / ANGLE_CONVERSION);
    targ = T_NBR;
}



// convert radians to degrees.  Thanks to Alan Williams for the contribution
void fun_deg(void) {
    fret = (MMFLOAT)((MMFLOAT)getnumber(ep)*RADCONV);
    targ = T_NBR;
}



// Returns the exponential value of a number.
void fun_exp(void) {
    fret = expf(getnumber(ep));
    targ = T_NBR;
}



// Truncate an expression to the next whole number less than or equal to the argument.
void fun_int(void) {
    iret = floorf(getnumber(ep));
    targ = T_INT;
}



// Truncate a number to a whole number by eliminating the decimal point and all characters
// to the right of the decimal point.
void fun_fix(void) {
    iret = getnumber(ep);
    targ = T_INT;
}



// return the length of a string
// nbr = LEN( string$ )
void fun_len(void) {
    iret = *(unsigned char *)getstring(ep);                           // first byte is the length
    targ = T_INT;
}



// Return the natural logarithm of the argument 'number'.
// n = LOG( number )
void fun_log(void) {
    MMFLOAT f;
    f = getnumber(ep);
    if(f == 0) error("Divide by zero");
    if(f < 0) error("Negative argument");
    fret = logf(f);
    targ = T_NBR;
}



// Return the value of Pi.  Thanks to Alan Williams for the contribution
// n = PI
void fun_pi(void) {
    fret = PI_VALUE;
    targ = T_NBR;
}



// convert degrees to radians.  Thanks to Alan Williams for the contribution
// r = RAD( degrees )
void fun_rad(void) {
    fret = (MMFLOAT)((MMFLOAT)getnumber(ep)/RADCONV);
    targ = T_NBR;
}



// generate a random number that is greater than or equal to 0 but less than 1
// n = RND()
void fun_rnd(void) {
    fret = (MMFLOAT)rand()/((MMFLOAT)RAND_MAX + (MMFLOAT)RAND_MAX/1000000);
    targ = T_NBR;
}



// Return the sign of the argument
// n = SGN( number )
void fun_sgn(void) {
    MMFLOAT f;
    f = getnumber(ep);
    if(f > 0)
        iret = +1;
    else if(f < 0)
        iret = -1;
    else
        iret = 0;
    targ = T_INT;
}



// Return the sine of the argument 'number' in radians.
// n = SIN( number )
void fun_sin(void) {
    fret = sinf(getnumber(ep) / ANGLE_CONVERSION);
    targ = T_NBR;
}



// Return the square root of the argument 'number'.
// n = SQR( number )
void fun_sqr(void) {
    MMFLOAT f;
    f = getnumber(ep);
    if(f < 0) error("Negative argument");
    fret = sqrtf(f);
    targ = T_NBR;
}



// Return the tangent of the argument 'number' in radians.
// n = TAN( number )
void fun_tan(void) {
    fret = tanf(getnumber(ep)/ ANGLE_CONVERSION);
    targ = T_NBR;
}



// Returns the numerical value of a string.
// n = VAL( string$ )
void fun_val(void) {
    char *p, *t1, *t2;
    p = getCstring(ep);
    targ = T_INT;
    if(*p == '&') {
        p++; iret = 0;
        switch(toupper(*p++)) {
            case 'H':
                while(isxdigit(*p)) {
                    iret = (iret << 4) | ((toupper(*p) >= 'A') ? toupper(*p) - 'A' + 10 : *p - '0');
                    p++;
                }
                break;
            case 'O':
                while(*p >= '0' && *p <= '7') {
                    iret = (iret << 3) | (*p++ - '0');
                }
                break;
            case 'B':
                while(*p == '0' || *p == '1') {
                    iret = (iret << 1) | (*p++ - '0');
                }
                break;
            default:
                iret = 0;
        }
    } else {
        fret = (MMFLOAT) strtod(p, &t1);
        iret = strtoll(p, &t2, 10);
        if (t1 > t2) targ = T_NBR;
    }
}



void fun_errno(void) {
    iret = mmb_error_state_ptr->code;
    targ = T_INT;
}



void fun_errmsg(void) {
    sret = GetTempStrMemory();
    strcpy(sret, mmb_error_state_ptr->message);
    CtoM(sret);
    targ = T_STR;
}



// Returns a string of blank spaces 'number' bytes long.
// s$ = SPACE$( number )
void fun_space(void) {
    int i;

    i = getint(ep, 0, MAXSTRLEN);
    sret = GetTempStrMemory();                                      // this will last for the life of the command
    memset(sret + 1, ' ', i);
    *sret = i;
    targ = T_STR;
}



// Returns string$ converted to uppercase characters.
// s$ = UCASE$( string$ )
void fun_ucase(void) {
    char *s, *p;
    int i;

    s = getstring(ep);
    p = sret = GetTempStrMemory();                                  // this will last for the life of the command
    i = *p++ = *s++;                                                // get the length of the string and save in the destination
    while(i--) {
        *p = toupper(*s);
        p++; s++;
    }
    targ = T_STR;
}



// Returns string$ converted to lowercase characters.
// s$ = LCASE$( string$ )
void fun_lcase(void) {
    char *s, *p;
    int i;

    s = getstring(ep);
    p = sret = GetTempStrMemory();                                  // this will last for the life of the command
    i = *p++ = *s++;                                                // get the length of the string and save in the destination
    while(i--) {
        *p = tolower(*s);
        p++; s++;
    }
    targ = T_STR;
}



// Function (which looks like a pre defined variable) to return the version number.
void fun_version(void){
    iret = MM_VERSION;
    targ = T_INT;
}



// Returns the current cursor position in the line in characters.
// n = POS
void fun_pos(void){
    iret = MMCharPos;
    targ = T_INT;
}



// Outputs spaces until the column indicated by 'number' has been reached.
// PRINT TAB( number )
void fun_tab(void) {
    int i;
    char *p;

    i = getint(ep, 1, 255);
    sret = p = GetTempStrMemory();                                  // this will last for the life of the command
    if(MMCharPos > i) {
        i--;
        *p++ = '\r';
        *p++ = '\n';
    }
    else
        i -= MMCharPos;
    memset(p, ' ', i);
    p[i] = 0;
    CtoM(sret);
    targ = T_STR;
}



// get a character from the console input queue
// s$ = INKEY$
void fun_inkey(void){
    sret = GetTempStrMemory();                                      // this buffer is automatically zeroed so the string is zero size

    int i = console_getc();
    if(i != -1) {
        sret[0] = 1;                                                // this is the length
        sret[1] = i;                                                // and this is the character
    }
    targ = T_STR;
}



// used by ACos() and ASin() below
MMFLOAT arcsinus(MMFLOAT x) {
     return 2.0L * atanf(x / (1.0L + sqrtf(1.0L - x * x)));
}


// Return the arcsine (in radians) of the argument 'number'.
// n = ASIN(number)
void fun_asin(void) {
     MMFLOAT f = getnumber(ep);
     if(f < -1.0 || f > 1.0) error("Number out of bounds");
     if (f == 1.0) {
          fret = PI_VALUE/2;
     } else if (f == -1.0) {
          fret = -PI_VALUE/2;
     } else {
          fret = arcsinus(f);
     }
    fret *= ANGLE_CONVERSION;
    targ = T_NBR;
}


// Return the arccosine (in radians) of the argument 'number'.
// n = ACOS(number)
void fun_acos(void) {
     MMFLOAT f = getnumber(ep);
     if(f < -1.0L || f > 1.0L) error("Number out of bounds");
     if (f == 1.0L) {
          fret = 0.0L;
     } else if (f == -1.0L) {
          fret = PI_VALUE;
     } else {
          fret = PI_VALUE/2 - arcsinus(f);
     }
     fret *= ANGLE_CONVERSION;
     targ = T_NBR;
}
