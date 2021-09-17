#include <complex.h>
#include <stdint.h>

#include "../common/error.h"
#include "../common/maths.h"
#include "../common/version.h"

void cmd_math(void){
        char *tp;
    int t = T_NBR;
    MMFLOAT f;
    long long int i64;
    char *s;

        skipspace(cmdline);
        if(toupper(*cmdline)=='S'){

                tp = checkstring(cmdline, "SET");
                if(tp) {
                        void *ptr1 = NULL;
                        int i,j,card1=1;
                        MMFLOAT *a1float=NULL;
                        int64_t *a1int=NULL;
                        getargs(&tp, 3,",");
                        if(!(argc == 3)) error("Argument count");
                    evaluate(argv[0], &f, &i64, &s, &t, false);
                    if(t & T_STR) error("Syntax");
                        ptr1 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card1 *= j;
                                }
                                a1float = (MMFLOAT *)ptr1;
                                if((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card1 *= j;
                                }
                                a1int = (int64_t *)ptr1;
                                if((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 2 must be numerical");
                        if(a1float!=NULL){
                                for(i=0; i< card1;i++)*a1float++ = ((t & T_INT) ? (MMFLOAT)i64 : f);
                        } else {
                                for(i=0; i< card1;i++)*a1int++ = ((t & T_INT) ? i64 : FloatToInt64(f));
                        }
                        return;
                }

                tp = checkstring(cmdline, "SCALE");
                if(tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        int i,j,card1=1, card2=1;
                        MMFLOAT *a1float=NULL,*a2float=NULL, scale;
                        int64_t *a1int=NULL, *a2int=NULL;
                        getargs(&tp, 5,",");
                        if(!(argc == 5)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card1 *= j;
                                }
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card1 *= j;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be numerical");
                    evaluate(argv[2], &f, &i64, &s, &t, false);
                    if(t & T_STR) error("Syntax");
                    scale=getnumber(argv[2]);
                        ptr2 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card2 *= j;
                                }
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) a2float != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card2 *= j;
                                }
                                a2int = (int64_t *)ptr2;
                                if ((char *) a2int != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 3 must be numerical");
                        if(card1 != card2)error("Size mismatch");
                        if(scale!=1.0){
                                if(a2float!=NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)*a2float++ = ((t & T_INT) ? (MMFLOAT)i64 : f) * (*a1float++);
                                } else if(a2float!=NULL && a1float==NULL){
                                        for(i=0; i< card1;i++)(*a2float++) = ((t & T_INT) ? (MMFLOAT)i64 : f) * ((MMFLOAT)*a1int++);
                                } else if(a2float==NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(((t & T_INT) ? i64 : FloatToInt64(f)) * (*a1float++));
                                } else {
                                        for(i=0; i< card1;i++)(*a2int++) = ((t & T_INT) ? i64 : FloatToInt64(f)) * (*a1int++);
                                }
                        } else {
                                if(a2float!=NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)*a2float++ = *a1float++;
                                } else if(a2float!=NULL && a1float==NULL){
                                        for(i=0; i< card1;i++)(*a2float++) = ((MMFLOAT)*a1int++);
                                } else if(a2float==NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(*a1float++);
                                } else {
                                        for(i=0; i< card1;i++)*a2int++ = *a1int++;
                                }
                        }
                        return;
                }

                tp = checkstring(cmdline, "SLICE");
                if(tp) {
                        int i, j, start, increment, dim[MAXDIM], pos[MAXDIM],off[MAXDIM], dimcount=0, target=-1, toarray=0;
                        int64_t *a1int=NULL,*a2int=NULL;
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        getargs(&tp, 13,",");
                        if(argc<7)error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & (T_NBR | T_INT)) {
                                if(vartbl[VarIndex].dims[1] <= 0) {                // Not an array
                                        error("Argument 1 must be a 2D or more numerical array");
                                }
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a 2D or more numerical array");
                                }
                                for(i=0;i<MAXDIM;i++){
                                        if(vartbl[VarIndex].dims[i]-OptionBase>0){
                                                dimcount++;
                                                dim[i]=vartbl[VarIndex].dims[i]-OptionBase;
                                        } else dim[i]=0;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a 2D or more numerical array");
                        if(((argc-1)/2-1)!=dimcount)error("Argument count");
                        for(i=0; i<dimcount;i++ ){
                                if(*argv[i*2 +2]) pos[i]=getint(argv[i*2 +2],OptionBase,dim[i]+OptionBase)-OptionBase;
                                else {
                                        if(target!=-1)error("Only one index can be omitted");
                                        target=i;
                                        pos[i]=1;
                                }
                        }
                        ptr2 = findvar(argv[i*2 +2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type &  (T_NBR | T_INT)) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Target must be 1D a numerical point array");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Target must be 1D a numerical point array");
                                }
                                toarray=vartbl[VarIndex].dims[0]-OptionBase;
                                a2int = (int64_t *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Target must be 1D a numerical point array");
                        if(dim[target]!=toarray)error("Size mismatch between slice and target array");
                        i=dimcount-1;
                        while(i>=0){
                                off[i]=1;
                                for(j=0; j<i; j++)off[i]*=(dim[j]+1);
                                i--;
                        }
                        start=1;
                        for(i=0;i<dimcount;i++){
                                start+= (pos[i]*off[i]);
                        }
                        start--;
                        increment=off[target];
                        start-=increment;
                        for(i=0;i<=dim[target];i++)*a2int++ = a1int[start+i*increment];
                        return;
                }
                // tp = checkstring(cmdline, "SENSORFUSION");
                // if(tp) {
                //         cmd_SensorFusion(tp);
                //         return;
                // }
        } else if(toupper(*cmdline)=='V') {
                tp = checkstring(cmdline, "V_MULT");
                if(tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        void *ptr3 = NULL;
                        int i,j, numcols=0, numrows=0;
                        MMFLOAT *a1float=NULL,*a2float=NULL,*a2sfloat=NULL,*a3float=NULL;
                        getargs(&tp, 5,",");
                        if(!(argc == 5)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[2] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[1] <= 0) {                // Not an array
                                        error("Argument 1 must be a 2D floating point array");
                                }
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a 2D floating point array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                numrows=vartbl[VarIndex].dims[1] - OptionBase;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a 2D floating point array");
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 2 must be a floating point array");
                                }
                                if((vartbl[VarIndex].dims[0] - OptionBase) != numcols)error("Array size mismatch");
                                a2float = a2sfloat = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 2 must be a floating point array");
                        ptr3 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 3 must be a floating point array");
                                }
                                if((vartbl[VarIndex].dims[0] - OptionBase) != numrows)error("Array size mismatch");
                                a3float = (MMFLOAT *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 3 must be a floating point array");
                        if(ptr3==ptr1 || ptr3==ptr2)error("Destination array same as source");
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
                if(tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        int j, numrows=0;
                        MMFLOAT *a1float=NULL,*a1sfloat=NULL,*a2float=NULL,mag=0.0;
                        getargs(&tp, 3,",");
                        if(!(argc == 3)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a floating point array");
                                }
                                numrows=vartbl[VarIndex].dims[0] - OptionBase;
                                a1float = a1sfloat = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a floating point array");
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 2 must be a floating point array");
                                }
                                if((vartbl[VarIndex].dims[0] - OptionBase) != numrows)error("Array size mismatch");
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 2 must be a floating point array");
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
                if(tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        void *ptr3 = NULL;
                        int j, numcols=0;
                        MMFLOAT *a1float=NULL,*a2float=NULL,*a3float=NULL;
                        MMFLOAT a[3],b[3];
                        getargs(&tp, 5,",");
                        if(!(argc == 5)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a 3 element floating point array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                if(numcols!=2)error("Argument 1 must be a 3 element floating point array");
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a 3 element floating point array");
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 2 must be a floating point array");
                                }
                                if((vartbl[VarIndex].dims[0] - OptionBase) != numcols)error("Array size mismatch");
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 2 must be a floating point array");
                        ptr3 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 3 must be a floating point array");
                                }
                                if((vartbl[VarIndex].dims[0] - OptionBase) != numcols)error("Array size mismatch");
                                a3float = (MMFLOAT *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 3 must be a floating point array");
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
                if(tp) {
                        void *ptr1 = NULL;
                        int j, numcols=0;
                        MMFLOAT *a1float=NULL;
                        int64_t *a1int=NULL;
                        getargs(&tp, 1,",");
                        if(!(argc == 1)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a numerical array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(ptr1 && vartbl[VarIndex].type & T_INT) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a numerical array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a numerical array");
                        numcols++;
                        if(a1float!=NULL){
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


        } else if(toupper(*cmdline)=='M') {
                tp = checkstring(cmdline, "M_INVERSE");
                if(tp){
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        int i, j, n, numcols=0, numrows=0;
                        MMFLOAT *a1float=NULL, *a2float=NULL,det;
                        getargs(&tp, 3,",");
                        if(!(argc == 3)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[2] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[1] <= 0) {                // Not an array
                                        error("Argument 1 must be a numerical 2D array");
                                }
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a numerical 2D array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                numrows=vartbl[VarIndex].dims[1] - OptionBase;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else        error("Argument 1 must be a numerical 2D array");
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[2] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[1] <= 0) {                // Not an array
                                        error("Argument 2 must be a numerical 2D array");
                                }
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 2 must be a numerical 2D array");
                                }
                                if(numcols!=vartbl[VarIndex].dims[0] - OptionBase)error("array size mismatch");
                                if(numrows!=vartbl[VarIndex].dims[1] - OptionBase)error("array size mismatch");
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else        error("Argument 2 must be a numerical 2D array");
                        if(numcols!=numrows)error("Array must be square");
                        if(ptr1==ptr2)error("Same array specified for input and output");
                        n=numrows+1;
                        MMFLOAT **matrix=alloc2df(n,n);
                        for(i=0;i<n;i++){ //load the matrix
                                for(j=0;j<n;j++){
                                        matrix[j][i]=*a1float++;
                                }
                        }
                        det=determinant(matrix,n);
                        if(det==0.0){
                                dealloc2df(matrix,numcols,numrows);
                                error("Determinant of array is zero");
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
                if(tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        int i,j, numcols1=0, numrows1=0, numcols2=0, numrows2=0;
                        MMFLOAT *a1float=NULL,*a2float=NULL;
                        getargs(&tp, 3,",");
                        if(!(argc == 3)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[2] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[1] <= 0) {                // Not an array
                                        error("Argument 1 must be a 2D floating point array");
                                }
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a 2D floating point array");
                                }
                                numcols1=numrows2=vartbl[VarIndex].dims[0] - OptionBase;
                                numrows1=numcols2=vartbl[VarIndex].dims[1] - OptionBase;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a 2D floating point array");
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[2] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[1] <= 0) {                // Not an array
                                        error("Argument 2 must be 2D floating point array");
                                }
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 2 must be 2D floating point array");
                                }
                                if(numcols2 !=(vartbl[VarIndex].dims[0] - OptionBase))error("Array size mismatch");
                                if(numrows2 !=(vartbl[VarIndex].dims[1] - OptionBase))error("Array size mismatch");
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 2 must be a 2D floating point array");
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
                if(tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        void *ptr3 = NULL;
                        int i,j, k, numcols1=0, numrows1=0, numcols2=0, numrows2=0, numcols3=0, numrows3=0;
                        MMFLOAT *a1float=NULL,*a2float=NULL,*a3float=NULL;
                        getargs(&tp, 5,",");
                        if(!(argc == 5)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[2] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[1] <= 0) {                // Not an array
                                        error("Argument 1 must be a 2D floating point array");
                                }
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a 2D floating point array");
                                }
                                numcols1=numrows2=vartbl[VarIndex].dims[0] - OptionBase + 1;
                                numrows1=vartbl[VarIndex].dims[1] - OptionBase + 1;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a 2D floating point array");
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[2] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[1] <= 0) {                // Not an array
                                        error("Argument 2 must be a 2D floating point array");
                                }
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 2 must be a 2D floating point array");
                                }
                                numcols2=vartbl[VarIndex].dims[0] - OptionBase + 1;
                                numrows2=vartbl[VarIndex].dims[1] - OptionBase + 1;
                                if(numrows2 !=numcols1)error("Input array size mismatch");
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 2 must be a 2D floating point array");
                        ptr3 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[2] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[1] <= 0) {                // Not an array
                                        error("Argument 3 must be a 2D floating point array");
                                }
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 3 must be a 2D floating point array");
                                }
                                numcols3=vartbl[VarIndex].dims[0] - OptionBase + 1;
                                numrows3=vartbl[VarIndex].dims[1] - OptionBase + 1;
                                if(numcols3 !=numcols2 || numrows3 !=numrows1)error("Output array size mismatch");
                                a3float = (MMFLOAT *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 3 must be a 2D floating point array");
                        if(ptr3==ptr1 || ptr3==ptr2)error("Destination array same as source");
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
                if(tp) {
                        void *ptr1 = NULL;
                        int i,j, numcols=0, numrows=0;
                        MMFLOAT *a1float=NULL;
                        int64_t *a1int=NULL;
                        // need three arrays with same cardinality, second array must be 2 dimensional
                        getargs(&tp, 1,",");
                        if(!(argc == 1)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[2] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[1] <= 0) {                // Not an array
                                        error("Argument 1 must be a numerical 2D array");
                                }
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a numerical 2D array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                numrows=vartbl[VarIndex].dims[1] - OptionBase;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(ptr1 && vartbl[VarIndex].type & T_INT) {
                                if(vartbl[VarIndex].dims[2] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[1] <= 0) {                // Not an array
                                        error("Argument 1 must be a numerical 2D array");
                                }
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a numerical 2D array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                numrows=vartbl[VarIndex].dims[1] - OptionBase;
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a numerical 2D array");
                        numcols++;
                        numrows++;
                        MMFLOAT **matrix=alloc2df(numcols,numrows);
                        int64_t **imatrix= (int64_t **)matrix;
                        if(a1float!=NULL){
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
        } else if(toupper(*cmdline)=='Q') {

                tp = checkstring(cmdline, "Q_INVERT");
                if(tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        int numcols=0;
                        MMFLOAT *q=NULL,*n=NULL;
                        getargs(&tp, 3,",");
                        if(!(argc == 3)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a 5 element floating point array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                if(numcols!=4)error("Argument 1 must be a 5 element floating point array");
                                q = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a 5 element floating point array");
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 2 must be a 5 element floating point array");
                                }
                                if((vartbl[VarIndex].dims[0] - OptionBase) != numcols)error("Array size mismatch");
                                n = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 2 must be a 5 element floating point array");
                        Q_Invert(q, n);
                        return;
                }

                tp = checkstring(cmdline, "Q_VECTOR");
                if(tp) {
                        void *ptr1 = NULL;
                        int numcols=0;
                        MMFLOAT *q=NULL;
                        MMFLOAT mag=0.0;
                        getargs(&tp, 7,",");
                        if(!(argc == 7)) error("Argument count");
                        MMFLOAT x=getnumber(argv[0]);
                        MMFLOAT y=getnumber(argv[2]);
                        MMFLOAT z=getnumber(argv[4]);
                        ptr1 = findvar(argv[6], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 4 must be a 5 element floating point array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                if(numcols!=4)error("Argument 4 must be a 5 element floating point array");
                                q = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 4 must be a 5 element floating point array");
                        mag=sqrt(x*x + y*y + z*z) ;//calculate the magnitude
                        q[0]=0.0; //create a normalised vector
                        q[1]=x/mag;
                        q[2]=y/mag;
                        q[3]=z/mag;
                        q[4]=mag;
                        return;
                }

                tp = checkstring(cmdline, "Q_EULER");
                if(tp) {
                        void *ptr1 = NULL;
                        int numcols=0;
                        MMFLOAT *q=NULL;
                        getargs(&tp, 7,",");
                        if(!(argc == 7)) error("Argument count");
                        MMFLOAT yaw=-getnumber(argv[0])/optionangle;
                        MMFLOAT pitch=getnumber(argv[2])/optionangle;
                        MMFLOAT roll=getnumber(argv[4])/optionangle;
                        ptr1 = findvar(argv[6], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 4 must be a 5 element floating point array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                if(numcols!=4)error("Argument 4 must be a 5 element floating point array");
                                q = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 4 must be a 5 element floating point array");
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
                if(tp) {
                        void *ptr1 = NULL;
                        int numcols=0;
                        MMFLOAT *q=NULL;
                        MMFLOAT mag=0.0;
                        getargs(&tp, 9,",");
                        if(!(argc == 9)) error("Argument count");
                        MMFLOAT theta=getnumber(argv[0]);
                        MMFLOAT x=getnumber(argv[2]);
                        MMFLOAT y=getnumber(argv[4]);
                        MMFLOAT z=getnumber(argv[6]);
                        ptr1 = findvar(argv[8], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a 5 element floating point array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                if(numcols!=4)error("Argument 1 must be a 5 element floating point array");
                                q = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a 5 element floating point array");
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
                if(tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        void *ptr3 = NULL;
                        int numcols=0;
                        MMFLOAT *q1=NULL,*q2=NULL,*n=NULL;
                        getargs(&tp, 5,",");
                        if(!(argc == 5)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a 5 element floating point array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                if(numcols!=4)error("Argument 1 must be a 5 element floating point array");
                                q1 = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a 5 element floating point array");
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 2 must be a 5 element floating point array");
                                }
                                if((vartbl[VarIndex].dims[0] - OptionBase) != numcols)error("Array size mismatch");
                                q2 = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 2 must be a 5 element floating point array");
                        ptr3 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 3 must be a 5 element floating point array");
                                }
                                if((vartbl[VarIndex].dims[0] - OptionBase) != numcols)error("Array size mismatch");
                                n = (MMFLOAT *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 3 must be a 5 element floating point array");
                        numcols++;
                        Q_Mult(q1, q2, n);
                        return;
                }

                tp = checkstring(cmdline, "Q_ROTATE");
                if(tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        void *ptr3 = NULL;
                        int numcols=0;
                        MMFLOAT *q1=NULL,*v1=NULL,*n=NULL;
                        MMFLOAT temp[5], qtemp[5];
                        getargs(&tp, 5,",");
                        if(!(argc == 5)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a 5 element floating point array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                if(numcols!=4)error("Argument 1 must be a 5 element floating point array");
                                q1 = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a 5 element floating point array");
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 2 must be a 5 element floating point array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                if(numcols!=4)error("Argument 2 must be a 5 element floating point array");
                                v1 = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 2 must be a 5 element floating point array");
                        ptr3 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 3 must be a 5 element floating point array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - OptionBase;
                                if(numcols!=4)error("Argument 3 must be a 5 element floating point array");
                                n = (MMFLOAT *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 3 must be a 5 element floating point array");
                        numcols++;
                        Q_Mult(q1, v1, temp);
                        Q_Invert(q1, qtemp);
                        Q_Mult(temp, qtemp, n);
                        return;
                }
        } else {
                tp = checkstring(cmdline, "ADD");
                if(tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        int i,j,card1=1, card2=1;
                        MMFLOAT *a1float=NULL,*a2float=NULL, scale;
                        int64_t *a1int=NULL, *a2int=NULL;
                        getargs(&tp, 5,",");
                        if(!(argc == 5)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card1 *= j;
                                }
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card1 *= j;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be numerical");
                    evaluate(argv[2], &f, &i64, &s, &t, false);
                    if(t & T_STR) error("Syntax");
                    scale=getnumber(argv[2]);
                        ptr2 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card2 *= j;
                                }
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card2 *= j;
                                }
                                a2int = (int64_t *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 3 must be numerical");
                        if(card1 != card2)error("Size mismatch");
                        if(scale!=0.0){
                                if(a2float!=NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)*a2float++ = ((t & T_INT) ? (MMFLOAT)i64 : f) + (*a1float++);
                                } else if(a2float!=NULL && a1float==NULL){
                                        for(i=0; i< card1;i++)(*a2float++) = ((t & T_INT) ? (MMFLOAT)i64 : f) + ((MMFLOAT)*a1int++);
                                } else if(a2float==NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(((t & T_INT) ? i64 : FloatToInt64(f)) + (*a1float++));
                                } else {
                                        for(i=0; i< card1;i++)(*a2int++) = ((t & T_INT) ? i64 : FloatToInt64(f)) + (*a1int++);
                                }
                        } else {
                                if(a2float!=NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)*a2float++ = *a1float++;
                                } else if(a2float!=NULL && a1float==NULL){
                                        for(i=0; i< card1;i++)(*a2float++) = ((MMFLOAT)*a1int++);
                                } else if(a2float==NULL && a1float!=NULL){
                                        for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(*a1float++);
                                } else {
                                        for(i=0; i< card1;i++)*a2int++ = *a1int++;
                                }
                        }
                        return;
                }
                tp = checkstring(cmdline, "INTERPOLATE");
                if(tp) {
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        void *ptr3 = NULL;
                        int i,j,card1=1, card2=1, card3=1;
                        MMFLOAT *a1float=NULL,*a2float=NULL, *a3float=NULL, scale, tmp1, tmp2, tmp3;
                        int64_t *a1int=NULL, *a2int=NULL, *a3int=NULL;
                        getargs(&tp, 7,",");
                        if(!(argc == 7)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card1 *= j;
                                }
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card1 *= j;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be numerical");
                    evaluate(argv[4], &f, &i64, &s, &t, false);
                    if(t & T_STR) error("Syntax");
                    scale=getnumber(argv[4]);
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card2 *= j;
                                }
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card2 *= j;
                                }
                                a2int = (int64_t *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 2 must be numerical");
                        ptr3 = findvar(argv[6], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card3=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card3 *= j;
                                }
                                a3float = (MMFLOAT *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card3=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - OptionBase+1);
                                        if(j)card3 *= j;
                                }
                                a3int = (int64_t *)ptr3;
                                if ((char *) ptr3 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 2 must be numerical");
                        if((card1 != card2) || (card1 !=card3))error("Size mismatch");
                        if((ptr1==ptr2) || (ptr1==ptr3))error("Arrays must be different");
                        if(a3int!=NULL){
                                for(i=0; i< card1;i++){
                                        if(a1int!=NULL)tmp1=(MMFLOAT)*a1int++;
                                        else tmp1=*a1float++;
                                        if(a2int!=NULL)tmp2=(MMFLOAT)*a2int++;
                                        else tmp2=*a2float++;
                                        tmp3=(tmp2-tmp1)*scale + tmp1;
                                        *a3int++=FloatToInt64(tmp3);
                                }
                        } else {
                                for(i=0; i< card1;i++){
                                        if(a1int!=NULL)tmp1=(MMFLOAT)*a1int++;
                                        else tmp1=*a1float++;
                                        if(a2int!=NULL)tmp2=(MMFLOAT)*a2int++;
                                        else tmp2=*a2float++;
                                        tmp3=(tmp2-tmp1)*scale + tmp1;
                                        *a3float++=tmp3;
                                }
                        }
                        return;
                }
                tp = checkstring(cmdline, "INSERT");
                if(tp) {
                        int i, j, start, increment, dim[MAXDIM], pos[MAXDIM],off[MAXDIM], dimcount=0, target=-1, toarray=0;
                        int64_t *a1int=NULL,*a2int=NULL;
                        void *ptr1 = NULL;
                        void *ptr2 = NULL;
                        getargs(&tp, 13,",");
                        if(argc<7)error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & (T_NBR | T_INT)) {
                                if(vartbl[VarIndex].dims[1] <= 0) {                // Not an array
                                        error("Argument 1 must be a 2D or more numerical array");
                                }
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a 2D or more numerical array");
                                }
                                for(i=0;i<MAXDIM;i++){
                                        if(vartbl[VarIndex].dims[i]-OptionBase>0){
                                                dimcount++;
                                                dim[i]=vartbl[VarIndex].dims[i]-OptionBase;
                                        } else dim[i]=0;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a 2D or more numerical array");
                        if(((argc-1)/2-1)!=dimcount)error("Argument count");
                        for(i=0; i<dimcount;i++ ){
                                if(*argv[i*2 +2]) pos[i]=getint(argv[i*2 +2],OptionBase,dim[i]+OptionBase)-OptionBase;
                                else {
                                        if(target!=-1)error("Only one index can be omitted");
                                        target=i;
                                        pos[i]=1;
                                }
                        }
                        ptr2 = findvar(argv[i*2 +2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type &  (T_NBR | T_INT)) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Source must be 1D a numerical point array");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Source must be 1D a numerical point array");
                                }
                                toarray=vartbl[VarIndex].dims[0]-OptionBase;
                                a2int = (int64_t *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Source must be 1D a numerical point array");
                        if(dim[target]!=toarray)error("Size mismatch between insert and target array");
                        i=dimcount-1;
                        while(i>=0){
                                off[i]=1;
                                for(j=0; j<i; j++)off[i]*=(dim[j]+1);
                                i--;
                        }
                        start=1;
                        for(i=0;i<dimcount;i++){
                                start+= (pos[i]*off[i]);
                        }
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

        error("Syntax");
}
