/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_sort.c

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include <ctype.h>
#include <string.h>

#include "../common/mmb4l.h"
#include "../common/error.h"

static void integersort(int64_t *iarray, int n, int64_t *index, int flags,
                 int startpoint) {
    int i, j = n, s = 1;
    int64_t t;
    if ((flags & 1) == 0) {
        while (s) {
            s = 0;
            for (i = 1; i < j; i++) {
                if (iarray[i] < iarray[i - 1]) {
                    t = iarray[i];
                    iarray[i] = iarray[i - 1];
                    iarray[i - 1] = t;
                    s = 1;
                    if (index != NULL) {
                        t = index[i - 1 + startpoint];
                        index[i - 1 + startpoint] = index[i + startpoint];
                        index[i + startpoint] = t;
                    }
                }
            }
            j--;
        }
    } else {
        while (s) {
            s = 0;
            for (i = 1; i < j; i++) {
                if (iarray[i] > iarray[i - 1]) {
                    t = iarray[i];
                    iarray[i] = iarray[i - 1];
                    iarray[i - 1] = t;
                    s = 1;
                    if (index != NULL) {
                        t = index[i - 1 + startpoint];
                        index[i - 1 + startpoint] = index[i + startpoint];
                        index[i + startpoint] = t;
                    }
                }
            }
            j--;
        }
    }
}

static void floatsort(MMFLOAT *farray, int n, int64_t *index, int flags,
               int startpoint) {
    int i, j = n, s = 1;
    int64_t t;
    MMFLOAT f;
    if ((flags & 1) == 0) {
        while (s) {
            s = 0;
            for (i = 1; i < j; i++) {
                if (farray[i] < farray[i - 1]) {
                    f = farray[i];
                    farray[i] = farray[i - 1];
                    farray[i - 1] = f;
                    s = 1;
                    if (index != NULL) {
                        t = index[i - 1 + startpoint];
                        index[i - 1 + startpoint] = index[i + startpoint];
                        index[i + startpoint] = t;
                    }
                }
            }
            j--;
        }
    } else {
        while (s) {
            s = 0;
            for (i = 1; i < j; i++) {
                if (farray[i] > farray[i - 1]) {
                    f = farray[i];
                    farray[i] = farray[i - 1];
                    farray[i - 1] = f;
                    s = 1;
                    if (index != NULL) {
                        t = index[i - 1 + startpoint];
                        index[i - 1 + startpoint] = index[i + startpoint];
                        index[i + startpoint] = t;
                    }
                }
            }
            j--;
        }
    }
}

static void stringsort(unsigned char *sarray, int n, int offset, int64_t *index,
                       int flags, int startpoint) {
    int ii, i, s = 1, isave;
    int k;
    unsigned char *s1, *s2, *p1, *p2;
    unsigned char temp;
    int reverse = 1 - ((flags & 1) << 1);
    while (s) {
        s = 0;
        for (i = 1; i < n; i++) {
            s2 = i * offset + sarray;
            s1 = (i - 1) * offset + sarray;
            ii = *s1 < *s2 ? *s1 : *s2;  // get the smaller  length
            p1 = s1 + 1;
            p2 = s2 + 1;
            k = 0;  // assume the strings match
            while ((ii--) && (k == 0)) {
                if (flags & 2) {
                    if (toupper(*p1) > toupper(*p2)) {
                        k = reverse;  // earlier in the array is bigger
                    }
                    if (toupper(*p1) < toupper(*p2)) {
                        k = -reverse;  // later in the array is bigger
                    }
                } else {
                    if (*p1 > *p2) {
                        k = reverse;  // earlier in the array is bigger
                    }
                    if (*p1 < *p2) {
                        k = -reverse;  // later in the array is bigger
                    }
                }
                p1++;
                p2++;
            }
            // if up to this point the strings match
            // make the decision based on which one is shorter
            if (k == 0) {
                if (*s1 > *s2) k = reverse;
                if (*s1 < *s2) k = -reverse;
            }
            if (k == 1) {  // if earlier is bigger swap them round
                ii = *s1 > *s2 ? *s1 : *s2;  // get the bigger length
                ii++;
                p1 = s1;
                p2 = s2;
                while (ii--) {
                    temp = *p1;
                    *p1 = *p2;
                    *p2 = temp;
                    p1++;
                    p2++;
                }
                s = 1;
                if (index != NULL) {
                    isave = index[i - 1 + startpoint];
                    index[i - 1 + startpoint] = index[i + startpoint];
                    index[i + startpoint] = isave;
                }
            }
        }
    }

    // Handle empty strings according to flag bit 2.
    // TODO: This should probably be done as part of the comparison step instead of post-processing.
    if ((flags & 0b101) == 0b101) {
        // Reverse sort, empty strings considered "biggest".
        for (i = n - 1; i >= 0; i--) {
            s2 = i * offset + sarray;
            if (*s2 != 0) break;
        }
        i++;
        if (i) {
            s2 = (n - i) * offset + sarray;
            memmove(s2, sarray, offset * i);
            memset(sarray, 0, offset * (n - i));
            if (index != NULL) {
                MMINTEGER *newindex = (MMINTEGER *)GetTempMemory(n * sizeof(MMINTEGER));
                memmove(&newindex[n - i], &index[startpoint], i * sizeof(MMINTEGER));
                memmove(newindex, &index[startpoint + i], (n - i) * sizeof(MMINTEGER));
                memmove(&index[startpoint], newindex, n * sizeof(MMINTEGER));
            }
        }
    } else if (flags & 0b100) {
        // Normal sort, empty strings considered "biggest".
        for (i = 0; i < n; i++) {
            s2 = i * offset + sarray;
            if (*s2 != 0) break;
        }
        if (i) {
            s2 = i * offset + sarray;
            memmove(sarray, s2, offset * (n - i));
            s2 = (n - i) * offset + sarray;
            memset(s2, 0, offset * i);
            if (index != NULL) {
                MMINTEGER *newindex = (MMINTEGER *)GetTempMemory(n * sizeof(MMINTEGER));
                memmove(newindex, &index[startpoint + i], (n - i) * sizeof(MMINTEGER));
                memmove(&newindex[n - i], &index[startpoint], i * sizeof(MMINTEGER));
                memmove(&index[startpoint], newindex, n * sizeof(MMINTEGER));
            }
         }
     }
}

/**
 * SORT array() [, indexarray()] [, flags] [, startposition] [, elementstosort]
 */
void cmd_sort(void) {
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    MMFLOAT *a3float = NULL;
    int64_t *a3int = NULL, *a4int = NULL;
    unsigned char *a3str = NULL;
    int i, size, truesize, flags = 0, maxsize = 0, startpoint = 0;
    getargs(&cmdline, 9, ",");
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if (vartbl[VarIndex].type & T_NBR) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_ARRAY(1);
        a3float = (MMFLOAT *)ptr1;
    } else if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_ARRAY(1);
        a3int = (int64_t *)ptr1;
    } else if (vartbl[VarIndex].type & T_STR) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_ARRAY(1);
        a3str = (unsigned char *)ptr1;
        maxsize = vartbl[VarIndex].size;
    } else ERROR_ARG_NOT_ARRAY(1);
    //if ((uint32_t)ptr1 != (uint32_t)vartbl[VarIndex].val.s)
    if (ptr1 != vartbl[VarIndex].val.s) ERROR_ARG_NOT_ARRAY(1);
    truesize = size = (vartbl[VarIndex].dims[0] - mmb_options.base);
    if (argc >= 3 && *argv[2]) {
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
            if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(2);
            a4int = (int64_t *)ptr2;
        } else ERROR_ARG_NOT_INTEGER_ARRAY(2);
        if ((vartbl[VarIndex].dims[0] - mmb_options.base) != size) ERROR_ARRAY_SIZE_MISMATCH;
        // if ((uint32_t)ptr2 != (uint32_t)vartbl[VarIndex].val.s)
        if (ptr2 != vartbl[VarIndex].val.s) ERROR_ARG_NOT_INTEGER_ARRAY(2);
    }
    if (argc >= 5 && *argv[4]) flags = getint(argv[4], 0b0, 0b111);
    if (argc >= 7 && *argv[6])
        startpoint = getint(argv[6], mmb_options.base, size + mmb_options.base);
    size -= startpoint;
    if (argc == 9) size = getint(argv[8], 1, size + 1 + mmb_options.base) - 1;
    if (startpoint) startpoint -= mmb_options.base;
    if (a3float != NULL) {
        a3float += startpoint;
        if (a4int != NULL)
            for (i = 0; i < truesize + 1; i++) a4int[i] = i + mmb_options.base;
        floatsort(a3float, size + 1, a4int, flags, startpoint);
    } else if (a3int != NULL) {
        a3int += startpoint;
        if (a4int != NULL)
            for (i = 0; i < truesize + 1; i++) a4int[i] = i + mmb_options.base;
        integersort(a3int, size + 1, a4int, flags, startpoint);
    } else if (a3str != NULL) {
        a3str += ((startpoint) * (maxsize + 1));
        if (a4int != NULL)
            for (i = 0; i < truesize + 1; i++) a4int[i] = i + mmb_options.base;
        stringsort(a3str, size + 1, maxsize + 1, a4int, flags, startpoint);
    }
}
