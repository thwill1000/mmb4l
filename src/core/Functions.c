/*-*****************************************************************************

MMBasic for Linux (MMB4L)

Functions.c

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

// Provides all the core functions in MMBasic.

#include "../Hardware_Includes.h"
#include "MMBasic_Includes.h"

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
    const char *p;
    char *s;
    MMFLOAT f;
    MMINTEGER i64;

    targ = T_INT;
    p = evaluate(ep, &f, &i64, &s, &targ, false);                   // get the value and type of the argument
    if(targ & T_NBR)
        fret = fabsf(f);
    else {
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
  fret = atanf(getnumber(ep));
    targ = T_NBR;
}


#if !defined(__mmb4l__)
// convert a number into a one character string
// s$ = CHR$(nbr)
void fun_chr(void) {
  int i;

  i = getint(ep, 0, 0xff);
  sret = GetTempStrMemory();                                        // this will last for the life of the command
  sret[0] = 1;
  sret[1] = i;
    targ = T_STR;
}
#endif



// Round numbers with fractional portions up or down to the next whole number or integer.
void fun_cint(void) {
  iret = getinteger(ep);
    targ = T_INT;
}



// return the cosine of a number in radians
void fun_cos(void) {
  fret = cosf(getnumber(ep));
    targ = T_NBR;
}


// convert radians to degrees.  Thanks to Alan Williams for the contribution
void fun_deg(void) {
  fret = (MMFLOAT)((double)getnumber(ep)*RADCONV);
    targ = T_NBR;
}



// Returns the exponential value of a number.
void fun_exp(void) {
  fret = expf(getnumber(ep));
    targ = T_NBR;
}


// utility function used by HEX$(), OCT$() and BIN$()
void DoHexOctBin(int base) {
    UNSIGNED_MMINTEGER i;
    int j = 1;
    getargs(&ep, 3, ",");
    i = (UNSIGNED_MMINTEGER) getinteger(argv[0]);                  // get the number
    if(argc == 3) j = getint(argv[2], 0, MAXSTRLEN);               // get the optional number of chars to return
    sret = GetTempStrMemory();                                     // this will last for the life of the command
    IntToStrPad(sret, (MMINTEGER) i, '0', j, base);
    CtoM(sret);
    targ = T_STR;
}



// return the hexadecimal representation of a number
// s$ = HEX$(nbr)
void fun_hex(void) {
    DoHexOctBin(16);
}



// return the octal representation of a number
// s$ = OCT$(nbr)
void fun_oct(void) {
    DoHexOctBin(8);
}



// return the binary representation of a number
// s$ = BIN$(nbr)
void fun_bin(void) {
    DoHexOctBin(2);
}



// syntax:  nbr = INSTR([start,] string1, string2)
//          find the position of string2 in string1 starting at start chars in string1
// returns an integer
void fun_instr(void) {
  char *s1 = NULL, *s2 = NULL;
  int start = 0;
  getargs(&ep, 5, ",");

  if(argc == 5) {
      start = getint(argv[0], 1, MAXSTRLEN + 1) - 1;
      s1 = getstring(argv[2]);
      s2 = getstring(argv[4]);
  }
  else if(argc == 3) {
      start = 0;
      s1 = getstring(argv[0]);
      s2 = getstring(argv[2]);
  }
  else
      error("Argument count");

    targ = T_INT;
  if(start > *s1 - *s2 + 1 || *s2 == 0)
      iret = 0;
  else {
      // find s2 in s1 using MMBasic strings
      int i;
      for(i = start; i < *s1 - *s2 + 1; i++) {
          if(memcmp(s1 + i + 1, s2 + 1, *s2) == 0) {
              iret = i + 1;
              return;
          }
      }
  }
  iret = 0;
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



// Return a substring offset by a number of characters from the left (beginning) of the string.
// s$ = LEFT$( string$, nbr )
void fun_left(void) {
  int i;
    char *s;
  getargs(&ep, 3, ",");

  if(argc != 3) error("Argument count");
  s = GetTempStrMemory();                                         // this will last for the life of the command
  Mstrcpy(s, getstring(argv[0]));
  i = getint(argv[2], 0, MAXSTRLEN);
  if(i < *s) *s = i;                                                // truncate if it is less than the current string length
    sret = s;
    targ = T_STR;
}



// Return a substring of ?string$? with ?number-of-chars? from the right (end) of the string.
// s$ = RIGHT$( string$, number-of-chars )
void fun_right(void) {
  int nbr;
  char *s, *p1, *p2;
  getargs(&ep, 3, ",");

  if(argc != 3) error("Argument count");
  s = getstring(argv[0]);
  nbr = getint(argv[2], 0, MAXSTRLEN);
  if(nbr > *s) nbr = *s;                                            // get the number of chars to copy
  sret = GetTempStrMemory();                                        // this will last for the life of the command
  p1 = sret; p2 = s + (*s - nbr) + 1;
  *p1++ = nbr;                                                      // inset the length of the returned string
  while(nbr--) *p1++ = *p2++;                                       // and copy the characters
    targ = T_STR;
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



// Returns a substring of ?string$? beginning at ?start? and continuing for ?nbr? characters.
// S$ = MID$(s, spos [, nbr])
void fun_mid(void) {
  char *s, *p1, *p2;
  int spos, nbr = 0, i;
  getargs(&ep, 5, ",");

  if(argc == 5) {                                                   // we have MID$(s, n, m)
      nbr = getint(argv[4], 0, MAXSTRLEN);                          // nbr of chars to return
  }
  else if(argc == 3) {                                              // we have MID$(s, n)
      nbr = MAXSTRLEN;                                              // default to all chars
  }
  else
      error("Argument count");

  s = getstring(argv[0]);                                           // the string
  spos = getint(argv[2], 1, MAXSTRLEN);                             // the mid position

  sret = GetTempStrMemory();                                        // this will last for the life of the command
    targ = T_STR;
  if(spos > *s || nbr == 0)                                         // if the numeric args are not in the string
      return;                                                       // return a null string
  else {
      i = *s - spos + 1;                                            // find how many chars remaining in the string
      if(i > nbr) i = nbr;                                          // reduce it if we don't need that many
      p1 = sret; p2 = s + spos;
      *p1++ = i;                                                    // set the length of the MMBasic string
      while(i--) *p1++ = *p2++;                                     // copy the nbr chars required
  }
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
    fret = (MMFLOAT)((double)getnumber(ep)/RADCONV);
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
  fret = sinf(getnumber(ep));
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
  fret = tanf(getnumber(ep));
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
          case 'H': while(isxdigit(*p)) {
                                      iret = (iret << 4) | ((toupper(*p) >= 'A') ? toupper(*p) - 'A' + 10 : *p - '0');
                                      p++;
                                  }
                                  break;
          case 'O': while(*p >= '0' && *p <= '7') {
                                      iret = (iret << 3) | (*p++ - '0');
                                  }
                                  break;
          case 'B': while(*p == '0' || *p == '1') {
                                      iret = (iret << 1) | (*p++ - '0');
                                  }
                                  break;
          default : iret = 0;
      }
  } else {
        fret = (MMFLOAT) strtod(p, &t1);
        iret = strtoll(p, &t2, 10);
        if (t1 > t2) targ = T_NBR;
    }
}



//#if !defined(MX170)
void fun_eval(void) {
    char *s, *st, *temp_tknbuf;
    temp_tknbuf = GetTempStrMemory();
    strcpy(temp_tknbuf, tknbuf);                                    // first save the current token buffer in case we are in immediate mode
    // we have to fool the tokeniser into thinking that it is processing a program line entered at the console
    st = GetTempStrMemory();
    strcpy(st, getstring(ep));                                      // then copy the argument
    MtoC(st);                                                       // and convert to a C string
    inpbuf[0] = 'r'; inpbuf[1] = '=';                               // place a dummy assignment in the input buffer to keep the tokeniser happy
    strcpy(inpbuf + 2, st);
    tokenise(true);                                                 // and tokenise it (the result is in tknbuf)
    strcpy(st, tknbuf + 3);
    targ = T_NOTYPE;
    evaluate(st, &fret, &iret, &s, &targ, false);                   // get the value and type of the argument
    if(targ & T_STR) {
        Mstrcpy(st, s);                                             // if it is a string then save it
        sret = st;
    }
    strcpy(tknbuf, temp_tknbuf);                                    // restore the saved token buffer
}
//#endif



void fun_errno(void) {
    iret = MMerrno;
    targ = T_INT;
}


void fun_errmsg(void) {
    sret = GetTempStrMemory();
    strcpy(sret, MMErrMsg);
    CtoM(sret);
    targ = T_STR;
}



// Returns a string of blank spaces 'number' bytes long.
// s$ = SPACE$( number )
void fun_space(void) {
  int i;

  i = getint(ep, 0, MAXSTRLEN);
  sret = GetTempStrMemory();                                        // this will last for the life of the command
  memset(sret + 1, ' ', i);
  *sret = i;
    targ = T_STR;
}



// Returns a string in the decimal (base 10) representation of  'number'.
// s$ = STR$( number, m, n, c$ )
void fun_str(void) {
    char *s;
    MMFLOAT f;
    MMINTEGER i64;
    int t;
    int m, n;
    char ch;
    const char *p;

    getargs(&ep, 7, ",");
    if((argc & 1) != 1) error("Syntax");
    t = T_NOTYPE;
    p = evaluate(argv[0], &f, &i64, &s, &t, false);                 // get the value and type of the argument
    if(t & T_STR) error("Expected a number");
    m = 0; n = STR_AUTO_PRECISION; ch = ' ';
    if(argc > 2) m = getint(argv[2], -128, 128);                    // get the number of digits before the point
    if(argc > 4) n = getint(argv[4], -20, 20);                      // get the number of digits after the point
    if(argc == 7) {
        p = getstring(argv[6]);
        if(*p == 0) error("Zero length argument");
        ch = ((unsigned char)p[1] & 0x7f);
    }

    sret = GetTempStrMemory();                                      // this will last for the life of the command
    if(t & T_NBR)
        FloatToStr(sret, f, m, n, ch);                              // convert the float
    else {
        if(n < 0)
            FloatToStr(sret, i64, m, n, ch);                        // convert as a float
        else {
            IntToStrPad(sret, i64, ch, m, 10);                      // convert the integer
            if(n != STR_AUTO_PRECISION && n > 0) {
                strcat(sret, ".");
                while(n--) strcat(sret, "0");                       // and add on any zeros after the point
            }
        }
    }
  CtoM(sret);
    targ = T_STR;
}



// Returns a string 'nbr' bytes long
// s$ = STRING$( nbr,  string$ )
// s$ = STRING$( nbr,  number )
void fun_string(void) {
    int i, j, t = T_NOTYPE;
    void *p;

    getargs(&ep, 3, ",");
    if(argc != 3) error("Syntax");

    i = getint(argv[0], 0, MAXSTRLEN);
    p = DoExpression(argv[2], &t);                                  // get the value and type of the argument
    if(t & T_STR) {
        if(!*(char *)p) error("Argument value: $", argv[2]);
        j = *((char *)p + 1);
    } else if(t & T_INT)
        j = *(MMINTEGER *)p;
    else
        j = FloatToInt32(*((MMFLOAT *)p));
    if(j < 0 || j > 255) error("Argument value: $", argv[2]);

    sret = GetTempStrMemory();                                      // this will last for the life of the command
    memset(sret + 1, j, i);
    *sret = i;
    targ = T_STR;
}



// Returns string$ converted to uppercase characters.
// s$ = UCASE$( string$ )
void fun_ucase(void) {
  char *s, *p;
  int i;

  s = getstring(ep);
  p = sret = GetTempStrMemory();                                    // this will last for the life of the command
  i = *p++ = *s++;                                                  // get the length of the string and save in the destination
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
  p = sret = GetTempStrMemory();                                    // this will last for the life of the command
  i = *p++ = *s++;                                                  // get the length of the string and save in the destination
  while(i--) {
      *p = tolower(*s);
      p++; s++;
  }
    targ = T_STR;
}


// function (which looks like a pre defined variable) to return the version number
// it pulls apart the VERSION string to generate the number
void fun_version(void){
  char *p;
    fret = strtol(VERSION, &p, 10);
    fret += (MMFLOAT)strtol(p + 1, &p, 10) / 100;
    fret += (MMFLOAT)strtol(p + 1, &p, 10) / 10000;
    targ = T_NBR;
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
  sret = p = GetTempStrMemory();                                    // this will last for the life of the command
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
    int i;

  sret = GetTempStrMemory();                                        // this buffer is automatically zeroed so the string is zero size

  i = getConsole();
  if(i != -1) {
      sret[0] = 1;                                                  // this is the length
      sret[1] = i;                                                  // and this is the character
  }
    targ = T_STR;
}



// used by ACos() and ASin() below
MMFLOAT arcsinus(MMFLOAT x) {
     return 2.0 * atanf(x / (1.0 + sqrtf(1.0 - x * x)));
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
    targ = T_NBR;
}


// Return the arccosine (in radians) of the argument 'number'.
// n = ACOS(number)
void fun_acos(void) {
     MMFLOAT f = getnumber(ep);
     if(f < -1.0 || f > 1.0) error("Number out of bounds");
     if (f == 1.0) {
          fret = 0.0;
     } else if (f == -1.0) {
          fret = PI_VALUE;
     } else {
          fret = PI_VALUE/2 - arcsinus(f);
     }
     targ = T_NBR;
}


// utility function to do the max/min comparison and return the value
// it is only called by fun_max() and fun_min() below.
void do_max_min(int cmp) {
    int i;
    MMFLOAT nbr, f;
    getargs(&ep, (MAX_ARG_COUNT * 2) - 1, ",");
    if((argc & 1) != 1) error("Syntax");
    if(cmp) nbr = -FLT_MAX; else nbr = FLT_MAX;
    for(i = 0; i < argc; i += 2) {
        f = getnumber(argv[i]);
      if(cmp && f > nbr) nbr = f;
      if(!cmp && f < nbr) nbr = f;
    }
    fret = nbr;
    targ = T_NBR;
}


void fun_max(void) {
    do_max_min(1);
}


void fun_min(void) {
    do_max_min(0);
}
