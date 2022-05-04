/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_field.c

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

#include "../common/mmb4l.h"
#include "../common/error.h"

// return true if the char 'c' is contained in the string 'srch$'
// used only by scan_for_delimiter()  below
// Note: this operates on MMBasic strings
static int MInStr(const char *srch, char c) {
    int i;
    for (i = 1; i <= *(unsigned char *)srch; i++)
        if (c == srch[i]) return true;
    return false;
}

// scan through string p and return p if it points to any char in delims
// this will skip any quoted text (quote delimiters in quotes)
// used only by fun_field() below
// Note: this operates on MMBasic strings
static int scan_for_delimiter(int start, const char *p, const char *delims, const char *quotes) {
    int i;
    char qidx;
    for (i = start; i <= *(unsigned char *)p && !MInStr(delims, p[i]); i++) {
        if (MInStr(quotes, p[i])) {  // if we have a quote
            qidx = p[i];
            i++;  // step over the opening quote
            while (i < *(unsigned char *)p && p[i] != qidx)
                i++;  // skip the quoted text
        }
    }
    return i;
}

// syntax:  str$ = FIELD$(string1, nbr, string2, string3)
//          find field nbr in string1 using the delimiters in string2 to separate the fields
//          if string3 is present any chars quoted by chars in string3 will not be searched for delimiters
// Note: this operates on MMBasic strings
void fun_field(void) {
    const char *p, *delims = "\1,", *quotes = "\0";
    int fnbr, i, j, k;
    getargs(&ep, 7, ",");
    if (!(argc == 3 || argc == 5 || argc == 7)) ERROR_SYNTAX;
    p = getstring(argv[0]);                // the string containing the fields
    fnbr = getint(argv[2], 1, MAXSTRLEN);  // field nbr to return
    if (argc > 3 && *argv[4])
        delims = getstring(argv[4]);             // delimiters for fields
    if (argc == 7) quotes = getstring(argv[6]);  // delimiters for quoted text
    sret = GetTempStrMemory();  // this will last for the life of the command
    targ = T_STR;
    i = 1;
    while (--fnbr > 0) {
        i = scan_for_delimiter(i, p, delims, quotes);
        if (i > *p) return;
        i++;  // step over the delimiter
    }
    while (p[i] == ' ') i++;                       // trim leading spaces
    j = scan_for_delimiter(i, p, delims, quotes);  // find the end of the field
    *sret = k = j - i;
    for (j = 1; j <= k; j++, i++) sret[j] = p[i];  // copy to the return string
    for (k = *sret; k > 0 && sret[k] == ' '; k--)
        ;  // trim trailing spaces
    *sret = k;
}
