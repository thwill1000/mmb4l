/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_math.c

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

#include <complex.h>
#include <ctype.h>
#include <math.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/maths.h"

#define ERROR_ARG_NOT_2D_OR_MORE_NUMERICAL_ARRAY(i)    error_throw_ex(kError,  "Argument % must be a 2D or more numerical array", i)
#define ERROR_ARG_NOT_3_ELEMENT_FLOAT_ARRAY(i)         error_throw_ex(kError,  "Argument % must be a 3 element floating point array", i)
#define ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(i)         error_throw_ex(kError,  "Argument % must be a 5 element floating point array", i)
#define ERROR_ARRAY_DETERMINANT_ZERO                   error_throw_ex(kError,  "Determinant of array is zero")
#define ERROR_ARRAY_NOT_SQUARE                         error_throw_ex(kError,  "Array must be square")
#define ERROR_ARRAYS_MUST_BE_DIFFERENT                 error_throw_ex(kError,  "Arrays must be different")
#define ERROR_DST_SAME_AS_SRC_ARRAY                    error_throw_ex(kError,  "Destination array same as source")
#define ERROR_INPUT_ARRAY_SIZE_MISMATCH                error_throw_ex(kError,  "Input array size mismatch")
#define ERROR_ONLY_ONE_INDEX_CAN_BE_OMITTED            error_throw_ex(kSyntax, "Only one index can be omitted")
#define ERROR_OUTPUT_ARRAY_SIZE_MISMATCH               error_throw_ex(kError,  "Output array size mismatch")
#define ERROR_INPUT_SAME_AS_OUTPUT_ARRAY               error_throw_ex(kError,  "Same array specified for input and output")
#define ERROR_SIZE_MISMATCH_BETWEEN_SLICE_AND_TARGET   error_throw_ex(kError,  "Size mismatch between slice and target array")
#define ERROR_SIZE_MISMATCH_BETWEEN_INSERT_AND_TARGET  error_throw_ex(kError,  "Size mismatch between insert and target array")
#define ERROR_SRC_NOT_1D_NUMERICAL_ARRAY               error_throw_ex(kError,  "Source must be a 1D numerical array");
#define ERROR_TARGET_NOT_1D_NUMERICAL_ARRAY            error_throw_ex(kError,  "Target must be a 1D numerical array")

void cmd_math(void){
    const char *tp;
    int t = T_NBR;
    MMFLOAT f;
    MMINTEGER i64;
    char *s;

        skipspace(cmdline);
        if (toupper(*cmdline)=='S'){

                tp = checkstring(cmdline, "SET");
                if (tp) {
                        void *ptr1 = NULL;
                        int i,j,card1=1;
                        MMFLOAT *a1float=NULL;
                        int64_t *a1int=NULL;
                        getargs(&tp, 3,",");
                        if (!(argc == 3)) ERROR_ARGUMENT_COUNT;
                    evaluate(argv[0], &f, &i64, &s, &t, false);
                    if (t & T_STR) ERROR_SYNTAX;
                        ptr1 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card1 *= j;
                                }
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if (vartbl[VarIndex].type & T_INT) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card1 *= j;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_NUMBER(2);
                        if (a1float!=NULL){
                                for(i=0; i< card1;i++)*a1float++ = ((t & T_INT) ? (MMFLOAT)i64 : f);
                        } else {
                                for(i=0; i< card1;i++)*a1int++ = ((t & T_INT) ? i64 : FloatToInt64(f));
                        }
                        return;
                }

                tp = checkstring(cmdline, "SCALE");
                if (tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        int i,j,card1=1, card2=1;
                        MMFLOAT *a1float=NULL,*a2float=NULL, scale;
                        int64_t *a1int=NULL, *a2int=NULL;
                        getargs(&tp, 5,",");
                        if (!(argc == 5)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card1 *= j;
                                }
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if (vartbl[VarIndex].type & T_INT) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card1 *= j;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_NUMBER(1);
                    evaluate(argv[2], &f, &i64, &s, &t, false);
                    if (t & T_STR) ERROR_SYNTAX;
                    scale=getnumber(argv[2]);
                        ptr2 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card2 *= j;
                                }
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) a2float != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if (vartbl[VarIndex].type & T_INT) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card2 *= j;
                                }
                                a2int = (int64_t *)ptr2;
                                if ((char *) a2int != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_NUMBER(3);
                        if (card1 != card2) ERROR_SIZE_MISMATCH;
                        if (scale!=1.0){
                                if (a2float!=NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)*a2float++ = ((t & T_INT) ? (MMFLOAT)i64 : f) * (*a1float++);
                                } else if (a2float!=NULL && a1float==NULL){
                                        for(i=0; i< card1;i++)(*a2float++) = ((t & T_INT) ? (MMFLOAT)i64 : f) * ((MMFLOAT)*a1int++);
                                } else if (a2float==NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(((t & T_INT) ? i64 : FloatToInt64(f)) * (*a1float++));
                                } else {
                                        for(i=0; i< card1;i++)(*a2int++) = ((t & T_INT) ? i64 : FloatToInt64(f)) * (*a1int++);
                                }
                        } else {
                                if (a2float!=NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)*a2float++ = *a1float++;
                                } else if (a2float!=NULL && a1float==NULL){
                                        for(i=0; i< card1;i++)(*a2float++) = ((MMFLOAT)*a1int++);
                                } else if (a2float==NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(*a1float++);
                                } else {
                                        for(i=0; i< card1;i++)*a2int++ = *a1int++;
                                }
                        }
                        return;
                }

                tp = checkstring(cmdline, "SLICE");
                if (tp) {
                        int i, j, start, increment, dim[MAXDIM], pos[MAXDIM],off[MAXDIM], dimcount=0, target=-1, toarray=0;
                        int64_t *a1int=NULL,*a2int=NULL;
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        getargs(&tp, 13,",");
                        if (argc<7) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & (T_NBR | T_INT)) {
                                if (vartbl[VarIndex].dims[1] <= 0) ERROR_ARG_NOT_2D_OR_MORE_NUMERICAL_ARRAY(1);
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_2D_OR_MORE_NUMERICAL_ARRAY(1);
                                for(i=0;i<MAXDIM;i++){
                                        if (vartbl[VarIndex].dims[i]-mmb_options.base>0){
                                                dimcount++;
                                                dim[i]=vartbl[VarIndex].dims[i]-mmb_options.base;
                                        } else dim[i]=0;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_2D_OR_MORE_NUMERICAL_ARRAY(1);
                        if (((argc-1)/2-1)!=dimcount) ERROR_ARGUMENT_COUNT;
                        for(i=0; i<dimcount;i++ ){
                                if (*argv[i*2 +2]) pos[i]=getint(argv[i*2 +2],mmb_options.base,dim[i]+mmb_options.base)-mmb_options.base;
                                else {
                                        if (target != -1) ERROR_ONLY_ONE_INDEX_CAN_BE_OMITTED;
                                        target=i;
                                        pos[i]=1;
                                }
                        }
                        ptr2 = findvar(argv[i*2 +2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type &  (T_NBR | T_INT)) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_TARGET_NOT_1D_NUMERICAL_ARRAY;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_TARGET_NOT_1D_NUMERICAL_ARRAY;
                                toarray=vartbl[VarIndex].dims[0]-mmb_options.base;
                                a2int = (int64_t *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_TARGET_NOT_1D_NUMERICAL_ARRAY;
                        if (dim[target] != toarray) ERROR_SIZE_MISMATCH_BETWEEN_SLICE_AND_TARGET;
                        i=dimcount-1;
// Suppress (spurious?) warnings with GCC 7.5 Release build on x86_64.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
                        while(i>=0){
                                off[i]=1;
                                for(j=0; j<i; j++)off[i]*=(dim[j]+1);
                                i--;
                        }
                        start=1;
                        for(i=0;i<dimcount;i++){
                                start+= (pos[i]*off[i]);
                        }
#pragma GCC diagnostic pop
                        start--;
                        increment=off[target];
                        start-=increment;
                        for(i=0;i<=dim[target];i++)*a2int++ = a1int[start+i*increment];
                        return;
                }
                // tp = checkstring(cmdline, "SENSORFUSION");
                // if (tp) {
                //         cmd_SensorFusion(tp);
                //         return;
                // }
        } else if (toupper(*cmdline)=='V') {
                tp = checkstring(cmdline, "V_MULT");
                if (tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        void *ptr3 = NULL;
                        int i,j, numcols=0, numrows=0;
                        MMFLOAT *a1float=NULL,*a2float=NULL,*a2sfloat=NULL,*a3float=NULL;
                        getargs(&tp, 5,",");
                        if (!(argc == 5)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[2] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[1] <= 0) ERROR_ARG_NOT_2D_FLOAT_ARRAY(1);
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_2D_FLOAT_ARRAY(1);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                numrows=vartbl[VarIndex].dims[1] - mmb_options.base;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_2D_FLOAT_ARRAY(1);
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_FLOAT_ARRAY(2);
                                if ((vartbl[VarIndex].dims[0] - mmb_options.base) != numcols) ERROR_ARRAY_SIZE_MISMATCH;
                                a2float = a2sfloat = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_FLOAT_ARRAY(2);
                        ptr3 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_FLOAT_ARRAY(3);
                                if ((vartbl[VarIndex].dims[0] - mmb_options.base) != numrows) ERROR_ARRAY_SIZE_MISMATCH;
                                a3float = (MMFLOAT *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_FLOAT_ARRAY(3);
                        if (ptr3 == ptr1 || ptr3 == ptr2) ERROR_DST_SAME_AS_SRC_ARRAY;
                        numcols++;
                        numrows++;
                        for(i=0;i<numrows;i++){
                                a2float=a2sfloat;
                                *a3float=0.0;
                                for(j=0;j<numcols;j++){
                                        *a3float= *a3float + ((*a1float++) * (*a2float++));
                                }
                                a3float++;
                        }
                        return;
                }

                tp = checkstring(cmdline, "V_NORMALISE");
                if (tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        int j, numrows=0;
                        MMFLOAT *a1float=NULL,*a1sfloat=NULL,*a2float=NULL,mag=0.0;
                        getargs(&tp, 3,",");
                        if (!(argc == 3)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_FLOAT_ARRAY(1);
                                numrows=vartbl[VarIndex].dims[0] - mmb_options.base;
                                a1float = a1sfloat = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_FLOAT_ARRAY(1);
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_FLOAT_ARRAY(2);
                                if ((vartbl[VarIndex].dims[0] - mmb_options.base) != numrows) ERROR_ARRAY_SIZE_MISMATCH;
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_FLOAT_ARRAY(2);
                        numrows++;
                        for(j=0;j<numrows;j++){
                                mag+= (*a1sfloat) * (*a1sfloat);
                                a1sfloat++;
                        }
                        mag= sqrt(mag);
                        for(j=0;j<numrows;j++){
                                *a2float++ = (*a1float++)/mag;
                        }
                        return;
                }

                tp = checkstring(cmdline, "V_CROSS");
                if (tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        void *ptr3 = NULL;
                        int j, numcols=0;
                        MMFLOAT *a1float=NULL,*a2float=NULL,*a3float=NULL;
                        MMFLOAT a[3],b[3];
                        getargs(&tp, 5,",");
                        if (!(argc == 5)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_3_ELEMENT_FLOAT_ARRAY(1);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                if (numcols != 2) ERROR_ARG_NOT_3_ELEMENT_FLOAT_ARRAY(1);
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_3_ELEMENT_FLOAT_ARRAY(1);
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_FLOAT_ARRAY(2);
                                if ((vartbl[VarIndex].dims[0] - mmb_options.base) != numcols) ERROR_ARRAY_SIZE_MISMATCH;
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_FLOAT_ARRAY(2);
                        ptr3 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_FLOAT_ARRAY(3);
                                if ((vartbl[VarIndex].dims[0] - mmb_options.base) != numcols) ERROR_ARRAY_SIZE_MISMATCH;
                                a3float = (MMFLOAT *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_FLOAT_ARRAY(3);
                        numcols++;
                        for(j=0;j<numcols;j++){
                                a[j]=*a1float++;
                                b[j]=*a2float++;
                        }
                *a3float++ = a[1]*b[2] - a[2]*b[1];
                *a3float++ = a[2]*b[0] - a[0]*b[2];
                *a3float = a[0]*b[1] - a[1]*b[0];
                return;
                }
                tp = checkstring(cmdline, "V_PRINT");
                if (tp) {
                        void *ptr1 = NULL;
                        int j, numcols=0;
                        MMFLOAT *a1float=NULL;
                        int64_t *a1int=NULL;
                        getargs(&tp, 1,",");
                        if (!(argc == 1)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_NUMBER_ARRAY(1);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if (ptr1 && vartbl[VarIndex].type & T_INT) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_NUMBER_ARRAY(1);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_NUMBER_ARRAY(1);
                        numcols++;
                        if (a1float!=NULL){
                                PFlt(*a1float++);
                                for(j=1;j<numcols;j++)PFltComma(*a1float++);
                                PRet();
                        } else {
                                PInt(*a1int++);
                                for(j=1;j<numcols;j++)PIntComma(*a1int++);
                                PRet();
                        }
                        return;
                }


        } else if (toupper(*cmdline)=='M') {
                tp = checkstring(cmdline, "M_INVERSE");
                if (tp){
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        int i, j, n, numcols=0, numrows=0;
                        MMFLOAT *a1float=NULL, *a2float=NULL,det;
                        getargs(&tp, 3,",");
                        if (!(argc == 3)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[2] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[1] <= 0) ERROR_ARG_NOT_2D_NUMBER_ARRAY(1);
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_2D_NUMBER_ARRAY(1);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                numrows=vartbl[VarIndex].dims[1] - mmb_options.base;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_NUMBER_ARRAY(2);
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[2] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[1] <= 0) ERROR_ARG_NOT_2D_NUMBER_ARRAY(2);
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_2D_NUMBER_ARRAY(2);
                                if (numcols!=vartbl[VarIndex].dims[0] - mmb_options.base) ERROR_ARRAY_SIZE_MISMATCH;
                                if (numrows!=vartbl[VarIndex].dims[1] - mmb_options.base) ERROR_ARRAY_SIZE_MISMATCH;
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_2D_NUMBER_ARRAY(2);
                        if (numcols != numrows) ERROR_ARRAY_NOT_SQUARE;
                        if (ptr1 == ptr2) ERROR_INPUT_SAME_AS_OUTPUT_ARRAY;
                        n=numrows+1;
                        MMFLOAT **matrix=alloc2df(n,n);
                        for(i=0;i<n;i++){ //load the matrix
                                for(j=0;j<n;j++){
                                        matrix[j][i]=*a1float++;
                                }
                        }
                        det=determinant(matrix,n);
                        if (det==0.0){
                                dealloc2df(matrix,numcols,numrows);
                                ERROR_ARRAY_DETERMINANT_ZERO;
                        }
                        MMFLOAT **matrix1=alloc2df(n,n);
                        cofactor(matrix, matrix1, n);
                        for(i=0;i<n;i++){ //load the matrix
                                for(j=0;j<n;j++){
                                        *a2float++=matrix1[j][i];
                                }
                        }
                        dealloc2df(matrix,numcols,numrows);
                        dealloc2df(matrix1,numcols,numrows);

                        return;
                }

                tp = checkstring(cmdline, "M_TRANSPOSE");
                if (tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        int i,j, numcols1=0, numrows1=0, numcols2=0, numrows2=0;
                        MMFLOAT *a1float=NULL,*a2float=NULL;
                        getargs(&tp, 3,",");
                        if (!(argc == 3)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[2] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[1] <= 0) ERROR_ARG_NOT_2D_FLOAT_ARRAY(1);
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_2D_FLOAT_ARRAY(1);
                                numcols1=numrows2=vartbl[VarIndex].dims[0] - mmb_options.base;
                                numrows1=numcols2=vartbl[VarIndex].dims[1] - mmb_options.base;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_2D_FLOAT_ARRAY(1);
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[2] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[1] <= 0) ERROR_ARG_NOT_2D_FLOAT_ARRAY(2);
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_2D_FLOAT_ARRAY(2);
                                if (numcols2 !=(vartbl[VarIndex].dims[0] - mmb_options.base)) ERROR_ARRAY_SIZE_MISMATCH;
                                if (numrows2 !=(vartbl[VarIndex].dims[1] - mmb_options.base)) ERROR_ARRAY_SIZE_MISMATCH;
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_2D_FLOAT_ARRAY(2);
                        numcols1++;
                        numrows1++;
                        numcols2++;
                        numrows2++;
                        MMFLOAT **matrix1=alloc2df(numcols1,numrows1);
                        MMFLOAT **matrix2=alloc2df(numrows2,numcols2);
                        for(i=0;i<numrows1;i++){
                                for(j=0;j<numcols1;j++){
                                        matrix1[j][i]=*a1float++;
                                }
                        }
                        for(i=0;i<numrows1;i++){
                                for(j=0;j<numcols1;j++){
                                        matrix2[i][j]=matrix1[j][i];
                                }
                        }
                        for(i=0;i<numrows2;i++){
                                for(j=0;j<numcols2;j++){
                                        *a2float++=matrix2[j][i];
                                }
                        }
                        dealloc2df(matrix1,numcols1,numrows1);
                        dealloc2df(matrix2,numcols2,numrows2);
                        return;
                }

                tp = checkstring(cmdline, "M_MULT");
                if (tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        void *ptr3 = NULL;
                        int i,j, k, numcols1=0, numrows1=0, numcols2=0, numrows2=0, numcols3=0, numrows3=0;
                        MMFLOAT *a1float=NULL,*a2float=NULL,*a3float=NULL;
                        getargs(&tp, 5,",");
                        if (!(argc == 5)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[2] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[1] <= 0) ERROR_ARG_NOT_2D_FLOAT_ARRAY(1);
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_2D_FLOAT_ARRAY(1);
                                numcols1=numrows2=vartbl[VarIndex].dims[0] - mmb_options.base + 1;
                                numrows1=vartbl[VarIndex].dims[1] - mmb_options.base + 1;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_2D_FLOAT_ARRAY(1);
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[2] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[1] <= 0) ERROR_ARG_NOT_2D_FLOAT_ARRAY(2);
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_2D_FLOAT_ARRAY(2);
                                numcols2=vartbl[VarIndex].dims[0] - mmb_options.base + 1;
                                numrows2=vartbl[VarIndex].dims[1] - mmb_options.base + 1;
                                if (numrows2 != numcols1) ERROR_INPUT_ARRAY_SIZE_MISMATCH;
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_2D_FLOAT_ARRAY(2);
                        ptr3 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[2] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[1] <= 0) ERROR_ARG_NOT_2D_FLOAT_ARRAY(3);
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_2D_FLOAT_ARRAY(3);
                                numcols3=vartbl[VarIndex].dims[0] - mmb_options.base + 1;
                                numrows3=vartbl[VarIndex].dims[1] - mmb_options.base + 1;
                                if (numcols3 != numcols2 || numrows3 !=numrows1) ERROR_OUTPUT_ARRAY_SIZE_MISMATCH;
                                a3float = (MMFLOAT *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_2D_FLOAT_ARRAY(3);
                        if (ptr3 == ptr1 || ptr3 == ptr2) ERROR_DST_SAME_AS_SRC_ARRAY;
                        MMFLOAT **matrix1=alloc2df(numcols1,numrows1);
                        MMFLOAT **matrix2=alloc2df(numrows2,numcols2);
                        MMFLOAT **matrix3=alloc2df(numrows3,numcols3);
                        for(i=0;i<numrows1;i++){ //load the first matrix
                                for(j=0;j<numcols1;j++){
                                        matrix1[j][i]=*a1float++;
                                }
                        }
                        for(i=0;i<numrows2;i++){ //load the second matrix
                                for(j=0;j<numcols2;j++){
                                        matrix2[j][i]=*a2float++;
                                }
                        }
        // Now calculate the dot products
                        for(i=0;i<numrows3;i++){
                                for(j=0;j<numcols3;j++){
                                        matrix3[j][i]=0.0;
                                        for(k=0;k<numcols1;k++){
                                                matrix3[j][i] += matrix1[k][i] * matrix2[j][k];
                                        }
                                }
                        }

                        for(i=0;i<numrows3;i++){ //store the answer
                                for(j=0;j<numcols3;j++){
                                        *a3float++=matrix3[j][i];
                                }
                        }
                        dealloc2df(matrix1,numcols1,numrows1);
                        dealloc2df(matrix2,numcols2,numrows2);
                        dealloc2df(matrix3,numcols3,numrows3);
                        return;
                }
                tp = checkstring(cmdline, "M_PRINT");
                if (tp) {
                        void *ptr1 = NULL;
                        int i,j, numcols=0, numrows=0;
                        MMFLOAT *a1float=NULL;
                        int64_t *a1int=NULL;
                        // need three arrays with same cardinality, second array must be 2 dimensional
                        getargs(&tp, 1,",");
                        if (!(argc == 1)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[2] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[1] <= 0) ERROR_ARG_NOT_2D_NUMBER_ARRAY(1);
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_2D_NUMBER_ARRAY(1);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                numrows=vartbl[VarIndex].dims[1] - mmb_options.base;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if (ptr1 && vartbl[VarIndex].type & T_INT) {
                                if (vartbl[VarIndex].dims[2] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[1] <= 0) ERROR_ARG_NOT_2D_NUMBER_ARRAY(1);
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_2D_NUMBER_ARRAY(1);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                numrows=vartbl[VarIndex].dims[1] - mmb_options.base;
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_2D_NUMBER_ARRAY(1);
                        numcols++;
                        numrows++;
                        MMFLOAT **matrix=alloc2df(numcols,numrows);
                        int64_t **imatrix= (int64_t **)matrix;
                        if (a1float!=NULL){
                                for(i=0;i<numrows;i++){
                                        for(j=0;j<numcols;j++){
                                                matrix[j][i]=*a1float++;
                                        }
                                }
                                for(i=0;i<numrows;i++){
                                        PFlt(matrix[0][i]);
                                        for(j=1;j<numcols;j++){
                                                PFltComma(matrix[j][i]);
                                        }
                                        PRet();
                                }
                        } else {
                                for(i=0;i<numrows;i++){
                                        for(j=0;j<numcols;j++){
                                                imatrix[j][i]=*a1int++;
                                        }
                                }
                                for(i=0;i<numrows;i++){
                                        PInt(imatrix[0][i]);
                                        for(j=1;j<numcols;j++){
                                                PIntComma(imatrix[j][i]);
                                        }
                                        PRet();
                                }
                        }
                        dealloc2df(matrix,numcols,numrows);
                        return;
                }
        } else if (toupper(*cmdline)=='Q') {

                tp = checkstring(cmdline, "Q_INVERT");
                if (tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        int numcols=0;
                        MMFLOAT *q=NULL,*n=NULL;
                        getargs(&tp, 3,",");
                        if (!(argc == 3)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(1);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                if (numcols != 4) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(1);
                                q = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(1);
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(2);
                                if ((vartbl[VarIndex].dims[0] - mmb_options.base) != numcols) ERROR_ARRAY_SIZE_MISMATCH;
                                n = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(2);
                        Q_Invert(q, n);
                        return;
                }

                tp = checkstring(cmdline, "Q_VECTOR");
                if (tp) {
                        void *ptr1 = NULL;
                        int numcols=0;
                        MMFLOAT *q=NULL;
                        MMFLOAT mag=0.0;
                        getargs(&tp, 7,",");
                        if (!(argc == 7)) ERROR_ARGUMENT_COUNT;
                        MMFLOAT x=getnumber(argv[0]);
                        MMFLOAT y=getnumber(argv[2]);
                        MMFLOAT z=getnumber(argv[4]);
                        ptr1 = findvar(argv[6], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(4);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                if (numcols != 4) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(4);
                                q = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(4);
                        mag=sqrt(x*x + y*y + z*z) ;//calculate the magnitude
                        q[0]=0.0; //create a normalised vector
                        q[1]=x/mag;
                        q[2]=y/mag;
                        q[3]=z/mag;
                        q[4]=mag;
                        return;
                }

                tp = checkstring(cmdline, "Q_EULER");
                if (tp) {
                        void *ptr1 = NULL;
                        int numcols=0;
                        MMFLOAT *q=NULL;
                        getargs(&tp, 7,",");
                        if (!(argc == 7)) ERROR_ARGUMENT_COUNT;
                        MMFLOAT yaw=-getnumber(argv[0])/optionangle;
                        MMFLOAT pitch=getnumber(argv[2])/optionangle;
                        MMFLOAT roll=getnumber(argv[4])/optionangle;
                        ptr1 = findvar(argv[6], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(4);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                if (numcols != 4) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(4);
                                q = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(4);
                        MMFLOAT s1=sin(pitch/2);
                        MMFLOAT c1=cos(pitch/2);
                        MMFLOAT s2=sin(yaw/2);
                        MMFLOAT c2=cos(yaw/2);
                        MMFLOAT s3=sin(roll/2);
                        MMFLOAT c3=cos(roll/2);
                        q[1] = s1 * c2 * c3 - c1 * s2 * s3;
                        q[2] = c1 * s2 * c3 + s1 * c2 * s3;
                        q[3] = c1 * c2 * s3 - s1 * s2 * c3;
                        q[0] = c1 * c2 * c3 + s1 * s2 * s3;
                        q[4]=1.0;
                        return;
                }

                tp = checkstring(cmdline, "Q_CREATE");
                if (tp) {
                        void *ptr1 = NULL;
                        int numcols=0;
                        MMFLOAT *q=NULL;
                        MMFLOAT mag=0.0;
                        getargs(&tp, 9,",");
                        if (!(argc == 9)) ERROR_ARGUMENT_COUNT;
                        MMFLOAT theta=getnumber(argv[0]);
                        MMFLOAT x=getnumber(argv[2]);
                        MMFLOAT y=getnumber(argv[4]);
                        MMFLOAT z=getnumber(argv[6]);
                        ptr1 = findvar(argv[8], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(1);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                if (numcols != 4) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(1);
                                q = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(1);
                        MMFLOAT sineterm= sin(theta/2.0/optionangle);
                        q[0]=cos(theta/2.0);
                        q[1]=x* sineterm;
                        q[2]=y* sineterm;
                        q[3]=z* sineterm;
                        mag=sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]) ;//calculate the magnitude
                        q[0]=q[0]/mag; //create a normalised quaternion
                        q[1]=q[1]/mag;
                        q[2]=q[2]/mag;
                        q[3]=q[3]/mag;
                        q[4]=1.0;
                        return;
                }

                tp = checkstring(cmdline, "Q_MULT");
                if (tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        void *ptr3 = NULL;
                        int numcols=0;
                        MMFLOAT *q1=NULL,*q2=NULL,*n=NULL;
                        getargs(&tp, 5,",");
                        if (!(argc == 5)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(1);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                if (numcols != 4) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(1);
                                q1 = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(1);
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(2);
                                if ((vartbl[VarIndex].dims[0] - mmb_options.base) != numcols) ERROR_ARRAY_SIZE_MISMATCH;
                                q2 = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(2);
                        ptr3 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(3);
                                if ((vartbl[VarIndex].dims[0] - mmb_options.base) != numcols) ERROR_ARRAY_SIZE_MISMATCH;
                                n = (MMFLOAT *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(3);
                        numcols++;
                        Q_Mult(q1, q2, n);
                        return;
                }

                tp = checkstring(cmdline, "Q_ROTATE");
                if (tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        void *ptr3 = NULL;
                        int numcols=0;
                        MMFLOAT *q1=NULL,*v1=NULL,*n=NULL;
                        MMFLOAT temp[5], qtemp[5];
                        getargs(&tp, 5,",");
                        if (!(argc == 5)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(1);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                if (numcols != 4) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(1);
                                q1 = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(1);
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(2);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                if (numcols != 4) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(2);
                                v1 = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(2);
                        ptr3 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(3);
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                if (numcols != 4) ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(3);
                                n = (MMFLOAT *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_5_ELEMENT_FLOAT_ARRAY(3);
                        numcols++;
                        Q_Mult(q1, v1, temp);
                        Q_Invert(q1, qtemp);
                        Q_Mult(temp, qtemp, n);
                        return;
                }
        } else {
                tp = checkstring(cmdline, "ADD");
                if (tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        int i,j,card1=1, card2=1;
                        MMFLOAT *a1float=NULL,*a2float=NULL, scale;
                        int64_t *a1int=NULL, *a2int=NULL;
                        getargs(&tp, 5,",");
                        if (!(argc == 5)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card1 *= j;
                                }
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if (vartbl[VarIndex].type & T_INT) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card1 *= j;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_NUMBER(1);
                    evaluate(argv[2], &f, &i64, &s, &t, false);
                    if (t & T_STR) ERROR_SYNTAX;
                    scale=getnumber(argv[2]);
                        ptr2 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card2 *= j;
                                }
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if (vartbl[VarIndex].type & T_INT) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card2 *= j;
                                }
                                a2int = (int64_t *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_NUMBER(3);
                        if (card1 != card2) ERROR_SIZE_MISMATCH;
                        if (scale!=0.0){
                                if (a2float!=NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)*a2float++ = ((t & T_INT) ? (MMFLOAT)i64 : f) + (*a1float++);
                                } else if (a2float!=NULL && a1float==NULL){
                                        for(i=0; i< card1;i++)(*a2float++) = ((t & T_INT) ? (MMFLOAT)i64 : f) + ((MMFLOAT)*a1int++);
                                } else if (a2float==NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(((t & T_INT) ? i64 : FloatToInt64(f)) + (*a1float++));
                                } else {
                                        for(i=0; i< card1;i++)(*a2int++) = ((t & T_INT) ? i64 : FloatToInt64(f)) + (*a1int++);
                                }
                        } else {
                                if (a2float!=NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)*a2float++ = *a1float++;
                                } else if (a2float!=NULL && a1float==NULL){
                                        for(i=0; i< card1;i++)(*a2float++) = ((MMFLOAT)*a1int++);
                                } else if (a2float==NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(*a1float++);
                                } else {
                                        for(i=0; i< card1;i++)*a2int++ = *a1int++;
                                }
                        }
                        return;
                }
                tp = checkstring(cmdline, "INTERPOLATE");
                if (tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        void *ptr3 = NULL;
                        int i,j,card1=1, card2=1, card3=1;
                        MMFLOAT *a1float=NULL,*a2float=NULL, *a3float=NULL, scale, tmp1, tmp2, tmp3;
                        int64_t *a1int=NULL, *a2int=NULL, *a3int=NULL;
                        getargs(&tp, 7,",");
                        if (!(argc == 7)) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card1 *= j;
                                }
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if (vartbl[VarIndex].type & T_INT) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card1 *= j;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_NUMBER(1);
                    evaluate(argv[4], &f, &i64, &s, &t, false);
                    if (t & T_STR) ERROR_SYNTAX;
                    scale=getnumber(argv[4]);
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card2 *= j;
                                }
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if (vartbl[VarIndex].type & T_INT) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card2 *= j;
                                }
                                a2int = (int64_t *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_NUMBER(2);
                        ptr3 = findvar(argv[6], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & T_NBR) {
                                card3=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card3 *= j;
                                }
                                a3float = (MMFLOAT *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if (vartbl[VarIndex].type & T_INT) {
                                card3=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if (j)card3 *= j;
                                }
                                a3int = (int64_t *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_ARG_NOT_NUMBER(3);
                        if ((card1 != card2) || (card1 !=card3)) ERROR_SIZE_MISMATCH;
                        if ((ptr1==ptr2) || (ptr1==ptr3)) ERROR_ARRAYS_MUST_BE_DIFFERENT;
                        if (a3int!=NULL){
                                for(i=0; i< card1;i++){
                                        if (a1int!=NULL)tmp1=(MMFLOAT)*a1int++;
                                        else tmp1=*a1float++;
                                        if (a2int!=NULL)tmp2=(MMFLOAT)*a2int++;
                                        else tmp2=*a2float++;
                                        tmp3=(tmp2-tmp1)*scale + tmp1;
                                        *a3int++=FloatToInt64(tmp3);
                                }
                        } else {
                                for(i=0; i< card1;i++){
                                        if (a1int!=NULL)tmp1=(MMFLOAT)*a1int++;
                                        else tmp1=*a1float++;
                                        if (a2int!=NULL)tmp2=(MMFLOAT)*a2int++;
                                        else tmp2=*a2float++;
                                        tmp3=(tmp2-tmp1)*scale + tmp1;
                                        *a3float++=tmp3;
                                }
                        }
                        return;
                }
                tp = checkstring(cmdline, "INSERT");
                if (tp) {
                        int i, j, start, increment, dim[MAXDIM], pos[MAXDIM],off[MAXDIM], dimcount=0, target=-1, toarray=0;
                        int64_t *a1int=NULL,*a2int=NULL;
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        getargs(&tp, 13,",");
                        if (argc<7) ERROR_ARGUMENT_COUNT;
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type & (T_NBR | T_INT)) {
                                if (vartbl[VarIndex].dims[1] <= 0) ERROR_ARG_NOT_2D_OR_MORE_NUMERICAL_ARRAY(1);
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_2D_OR_MORE_NUMERICAL_ARRAY(1);
                                for(i=0;i<MAXDIM;i++){
                                        if (vartbl[VarIndex].dims[i]-mmb_options.base>0){
                                                dimcount++;
                                                dim[i]=vartbl[VarIndex].dims[i]-mmb_options.base;
                                        } else dim[i]=0;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else  ERROR_ARG_NOT_2D_OR_MORE_NUMERICAL_ARRAY(1);
                        if (((argc-1)/2-1)!=dimcount) ERROR_ARGUMENT_COUNT;
                        for(i=0; i<dimcount;i++ ){
                                if (*argv[i*2 +2]) pos[i]=getint(argv[i*2 +2],mmb_options.base,dim[i]+mmb_options.base)-mmb_options.base;
                                else {
                                        if (target != -1) ERROR_ONLY_ONE_INDEX_CAN_BE_OMITTED;
                                        target=i;
                                        pos[i]=1;
                                }
                        }
                        ptr2 = findvar(argv[i*2 +2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if (vartbl[VarIndex].type &  (T_NBR | T_INT)) {
                                if (vartbl[VarIndex].dims[1] != 0) ERROR_SRC_NOT_1D_NUMERICAL_ARRAY;
                                if (vartbl[VarIndex].dims[0] <= 0) ERROR_SRC_NOT_1D_NUMERICAL_ARRAY;
                                toarray=vartbl[VarIndex].dims[0]-mmb_options.base;
                                a2int = (int64_t *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else ERROR_SRC_NOT_1D_NUMERICAL_ARRAY;
                        if (dim[target] != toarray) ERROR_SIZE_MISMATCH_BETWEEN_INSERT_AND_TARGET;
                        i=dimcount-1;
// Suppress (spurious?) warnings with GCC 7.5 Release build on x86_64.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
                        while(i>=0){
                                off[i]=1;
                                for(j=0; j<i; j++)off[i]*=(dim[j]+1);
                                i--;
                        }
                        start=1;
                        for(i=0;i<dimcount;i++){
                                start+= (pos[i]*off[i]);
                        }
#pragma GCC diagnostic pop
                        start--;
                        increment=off[target];
                        start-=increment;
                        for(i=0;i<=dim[target];i++) a1int[start+i*increment]=*a2int++;
                        return;
                }
                tp = checkstring(cmdline, "FFT");
                if (tp) {
                        maths_fft(tp);
                        return;
                }
        }

        ERROR_SYNTAX;
}
