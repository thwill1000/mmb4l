#include <complex.h>
#include <ctype.h>
#include <math.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/maths.h"

void fun_math(void){
        char *tp, *tp1;
        skipspace(ep);
        if(toupper(*ep)=='A'){
                tp = checkstring(ep, "ATAN3");
                if(tp) {
                        MMFLOAT y,x,z;
                        getargs(&tp, 3,",");
                        if(argc != 3) ERROR_SYNTAX;
                        y=getnumber(argv[0]);
                        x=getnumber(argv[2]);
                        z=atan2(y,x);
                        if (z < 0.0) z = z + 2.0 * PI_VALUE;
                        fret=z*optionangle;
                        targ = T_NBR;
                        return;
                }
        } else if(toupper(*ep)=='C') {

                tp = checkstring(ep, "COSH");
                if(tp) {
                        getargs(&tp, 1,",");
                        if(!(argc == 1)) error("Argument count");
                        fret=cosh(getnumber(argv[0]));
                        targ=T_NBR;
                        return;
                }
                tp = checkstring(ep, "CORREL");
                if(tp) {
                    void *ptr1 = NULL;
                    void *ptr2 = NULL;
                    int i,j,card1=1, card2=1;
                    MMFLOAT *a1float=NULL, *a2float=NULL, mean1=0, mean2=0;
                    MMFLOAT *a3float=NULL, *a4float=NULL;
                    MMFLOAT axb=0, a2=0, b2=0;
                    int64_t *a1int=NULL, *a2int=NULL;
                    getargs(&tp, 3,",");
                    if(!(argc == 3)) error("Argument count");
                    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                    j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                    if(j)card1 *= j;
                                }
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                    } else if(vartbl[VarIndex].type & T_INT) {
                            card1=1;
                            for(i=0;i<MAXDIM;i++){
                                    j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                    if(j)card1 *= j;
                            }
                        a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                    } else error("Argument 1 must be numerical");
                    ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                    j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                    if(j)card2 *= j;
                                }
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                    } else if(vartbl[VarIndex].type & T_INT) {
                            card2=1;
                            for(i=0;i<MAXDIM;i++){
                                    j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                    if(j)card2 *= j;
                            }
                        a2int = (int64_t *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                    } else error("Argument 2 must be numerical");
                        if(card1!=card2)error("Array size mismatch");
                        a3float=GetTempMemory(card1*sizeof(MMFLOAT));
                        a4float=GetTempMemory(card1*sizeof(MMFLOAT));
                        if(a1float!=NULL){
                                for(i=0; i< card1;i++)a3float[i] = (*a1float++);
                        } else {
                                for(i=0; i< card1;i++)a3float[i] = (MMFLOAT)(*a1int++);
                        }
                        if(a2float!=NULL){
                                for(i=0; i< card1;i++)a4float[i] = (*a2float++);
                        } else {
                                for(i=0; i< card1;i++)a4float[i] = (MMFLOAT)(*a2int++);
                        }
                        for(i=0;i<card1;i++){
                                mean1+=a3float[i];
                                mean2+=a4float[i];
                        }
                        mean1/=card1;
                        mean2/=card1;
                        for(i=0;i<card1;i++){
                                a3float[i]-=mean1;
                                a2+=(a3float[i]*a3float[i]);
                                a4float[i]-=mean2;
                                b2+=(a4float[i]*a4float[i]);
                                axb+=(a3float[i]*a4float[i]);
                        }
                        targ=T_NBR;
                        fret=axb/sqrt(a2*b2);
                        return;
                }
        tp = (checkstring(ep, "CHI_P"));
        tp1 = (checkstring(ep, "CHI"));
        if(tp || tp1) {
                        void *ptr1 = NULL;
                        int chi_p=1;
                        if(tp1){
                                tp=tp1;
                                chi_p=0;
                        }
                        int i,j, df, numcols=0, numrows=0;
                        MMFLOAT *a1float=NULL,*rows=NULL, *cols=NULL, chi=0, prob, chi_prob;
                        MMFLOAT total=0.0;
                        int64_t *a1int=NULL;
                        {
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
                                        numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                        numrows=vartbl[VarIndex].dims[1] - mmb_options.base;
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
                                        numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                        numrows=vartbl[VarIndex].dims[1] - mmb_options.base;
                                        a1int = (int64_t *)ptr1;
                                        if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                                } else error("Argument 1 must be a numerical 2D array");
                                df=numcols*numrows;
                                numcols++;
                                numrows++;
                                MMFLOAT **observed=alloc2df(numcols,numrows);
                                MMFLOAT **expected=alloc2df(numcols,numrows);
                                rows=alloc1df(numrows);
                                cols=alloc1df(numcols);
                                if(a1float!=NULL){
                                        for(i=0;i<numrows;i++){
                                                for(j=0;j<numcols;j++){
                                                        observed[j][i]=*a1float++;
                                                        total+=observed[j][i];
                                                        rows[i]+=observed[j][i];
                                                }
                                        }
                                } else {
                                        for(i=0;i<numrows;i++){
                                                for(j=0;j<numcols;j++){
                                                        observed[j][i]=(MMFLOAT)(*a1int++);
                                                        total+=observed[j][i];
                                                        rows[i]+=observed[j][i];
                                                }
                                        }
                                }
                                for(j=0;j<numcols;j++){
                                        for(i=0;i<numrows;i++){
                                                cols[j]+=observed[j][i];
                                        }
                                }
                                for(i=0;i<numrows;i++){
                                        for(j=0;j<numcols;j++){
                                                expected[j][i]=cols[j]*rows[i]/total;
                                                expected[j][i]=((observed[j][i]-expected[j][i]) * (observed[j][i]-expected[j][i]) / expected[j][i]);
                                                chi+=expected[j][i];
                                        }
                                }
                                prob=chitable[df][7];
                                if(chi>prob){
                                        i=7;
                                        while(i<15 && chi>=chitable[df][i])i++;
                                        chi_prob=chitable[0][i-1];
                                } else {
                                        i=7;
                                        while(i>=0 && chi<=chitable[df][i])i--;
                                        chi_prob=chitable[0][i+1];
                                }
                                dealloc2df(observed,numcols,numrows);
                                dealloc2df(expected,numcols,numrows);
                                FreeMemory(rows);
                                FreeMemory(cols);
                                // FreeMemorySafe((void *)&rows);
                                // FreeMemorySafe((void *)&cols);
                                targ=T_NBR;
                                fret=(chi_p ? chi_prob*100 : chi);
                                return;
                        }
                }

        } else if(toupper(*ep)=='D') {

                tp = checkstring(ep, "DOTPRODUCT");
                if(tp) {
                        int i;
                        void *ptr1 = NULL,*ptr2 = NULL;
                        int numcols=0;
                        MMFLOAT *a1float=NULL, *a2float=NULL;
                        // need two arrays with same cardinality
                        getargs(&tp, 3,",");
                        if(!(argc == 3)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a floating point array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a floating point array");
                        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 2 must be a floating point array");
                                }
                                if((vartbl[VarIndex].dims[0] - mmb_options.base) != numcols)error("Array size mismatch");
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 2 must be a floating point array");
                        numcols++;
                        fret=0;
                        for(i=0;i<numcols;i++){
                                fret = fret + ((*a1float++) * (*a2float++));
                        }
                        targ = T_NBR;
                        return;
                }
        } else if(toupper(*ep)=='L') {
                tp = checkstring(ep, "LOG10");
                if(tp) {
                        getargs(&tp, 1,",");
                        if(!(argc == 1)) error("Argument count");
                        fret=log10(getnumber(argv[0]));
                        targ=T_NBR;
                        return;
                }

        } else if(toupper(*ep)=='M') {
                tp = checkstring(ep, "M_DETERMINANT");
                if(tp){
                        void *ptr1 = NULL;
                        int i, j, n, numcols=0, numrows=0;
                        MMFLOAT *a1float=NULL;
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
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                numrows=vartbl[VarIndex].dims[1] - mmb_options.base;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else        error("Argument 1 must be a numerical 2D array");
                        if(numcols!=numrows)error("Array must be square");
                        n=numrows+1;
                        MMFLOAT **matrix=alloc2df(n,n);
                        for(i=0;i<n;i++){ //load the matrix
                                for(j=0;j<n;j++){
                                        matrix[j][i]=*a1float++;
                                }
                        }
                        fret=determinant(matrix,n);
                        dealloc2df(matrix,numcols,numrows);
                        targ=T_NBR;

                        return;
                }

                tp = checkstring(ep, "MAX");
                if(tp) {
                        void *ptr1 = NULL;
                        int i,j,card1=1;
                        MMFLOAT *a1float=NULL, max=-3.0e+38;
                        int64_t *a1int=NULL;
                        getargs(&tp, 1,",");
                        if(!(argc == 1)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if(j)card1 *= j;
                                }
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if(j)card1 *= j;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be numerical");
                        if(a1float!=NULL){
                                for(i=0; i< card1;i++){
                                        if((*a1float)>max)max=(*a1float);
                                        a1float++;
                                }
                        } else {
                                for(i=0; i< card1;i++){
                                        if(((MMFLOAT)(*a1int))>max)max=(MMFLOAT)(*a1int);
                                        a1int++;
                                }
                        }
                        targ=T_NBR;
                        fret=max;
                        return;
                }
                tp = checkstring(ep, "MIN");
                if(tp) {
                        void *ptr1 = NULL;
                        int i,j,card1=1;
                        MMFLOAT *a1float=NULL, min=3.0e+38;
                        int64_t *a1int=NULL;
                        getargs(&tp, 1,",");
                        if(!(argc == 1)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if(j)card1 *= j;
                                }
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if(j)card1 *= j;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be numerical");
                        if(a1float!=NULL){
                                for(i=0; i< card1;i++){
                                        if((*a1float)<min)min=(*a1float);
                                        a1float++;
                                }
                        } else {
                                for(i=0; i< card1;i++){
                                        if(((MMFLOAT)(*a1int))<min)min=(MMFLOAT)(*a1int);
                                        a1int++;
                                }
                        }
                        targ=T_NBR;
                        fret=min;
                        return;
                }
                tp = checkstring(ep, "MAGNITUDE");
                if(tp) {
                        int i;
                        void *ptr1 = NULL;
                        int numcols=0;
                        MMFLOAT *a1float=NULL;
                        MMFLOAT mag=0.0;
                        getargs(&tp, 1,",");
                        if(!(argc == 1)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                                if(vartbl[VarIndex].dims[0] <= 0) {                // Not an array
                                        error("Argument 1 must be a floating point array");
                                }
                                numcols=vartbl[VarIndex].dims[0] - mmb_options.base;
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be a floating point array");
                        numcols++;
                        for(i=0;i<numcols;i++){
                                mag = mag + ((*a1float) * (*a1float));
                                a1float++;
                        }
                        fret=sqrt(mag);
                        targ = T_NBR;
                        return;
                }

                tp = checkstring(ep, "MEAN");
                if(tp) {
                        void *ptr1 = NULL;
                        int i,j,card1=1;
                        MMFLOAT *a1float=NULL, mean=0;
                        int64_t *a1int=NULL;
                        getargs(&tp, 1,",");
                        if(!(argc == 1)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if(j)card1 *= j;
                                }
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if(j)card1 *= j;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be numerical");
                        if(a1float!=NULL){
                                for(i=0; i< card1;i++)mean+= (*a1float++);
                        } else {
                                for(i=0; i< card1;i++)mean+= (MMFLOAT)(*a1int++);
                        }
                        targ=T_NBR;
                        fret=mean/(MMFLOAT)card1;
                        return;
                }

                tp = checkstring(ep, "MEDIAN");
                if(tp) {
                        void *ptr2 = NULL;
                        int i,j,card1, card2=1;
                        MMFLOAT *a1float=NULL, *a2float=NULL;
                        int64_t *a2int=NULL;
                        getargs(&tp, 1,",");
                        if(!(argc == 1)) error("Argument count");
                        ptr2 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if(j)card2 *= j;
                                }
                                a2float = (MMFLOAT *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card2=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if(j)card2 *= j;
                                }
                                a2int = (int64_t *)ptr2;
                                if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be numerical");
                        card1=card2;
                        card2=(card2-1)/2;
                        a1float=GetTempMemory(card1*sizeof(MMFLOAT));
                        if(a2float!=NULL){
                                for(i=0; i< card1;i++)a1float[i] = (*a2float++);
                        } else {
                                for(i=0; i< card1;i++)a1float[i] = (MMFLOAT)(*a2int++);
                        }
                        floatshellsort(a1float,  card1);
                        targ=T_NBR;
                        if(card1 & 1)fret=a1float[card2];
                        else fret=(a1float[card2]+a1float[card2+1])/2.0;
                        return;
                }
        } else if(toupper(*ep)=='S') {

                tp = checkstring(ep, "SINH");
                if(tp) {
                        getargs(&tp, 1,",");
                        if(!(argc == 1)) error("Argument count");
                        fret=sinh(getnumber(argv[0]));
                        targ=T_NBR;
                        return;
                }

                tp = checkstring(ep, "SD");
                if(tp) {
                        void *ptr1 = NULL;
                        int i,j,card1=1;
                        MMFLOAT *a2float=NULL, *a1float=NULL, mean=0, var=0, deviation;
                        int64_t *a2int=NULL, *a1int=NULL;
                        getargs(&tp, 1,",");
                        if(!(argc == 1)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if(j)card1 *= j;
                                }
                                a1float = a2float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if(j)card1 *= j;
                                }
                                a1int = a2int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be numerical");
                        if(a2float!=NULL){
                                for(i=0; i< card1;i++)mean+= (*a2float++);
                        } else {
                                for(i=0; i< card1;i++)mean+= (MMFLOAT)(*a2int++);
                        }
                        mean=mean/(MMFLOAT)card1;
                        if(a1float!=NULL){
                                for(i=0; i< card1;i++){
                                        deviation = (*a1float++) - mean;
                                        var += deviation * deviation;
                                }
                        } else {
                                for(i=0; i< card1;i++){
                                        deviation = (MMFLOAT)(*a1int++) - mean;
                                        var += deviation * deviation;
                                }
                        }
                        targ=T_NBR;
                        fret=sqrt(var/card1);
                        return;
                }

                tp = checkstring(ep, "SUM");
                if(tp) {
                        void *ptr1 = NULL;
                        int i,j,card1=1;
                        MMFLOAT *a1float=NULL, sum=0;
                        int64_t *a1int=NULL;
                        getargs(&tp, 1,",");
                        if(!(argc == 1)) error("Argument count");
                        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
                        if(vartbl[VarIndex].type & T_NBR) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if(j)card1 *= j;
                                }
                                a1float = (MMFLOAT *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else if(vartbl[VarIndex].type & T_INT) {
                                card1=1;
                                for(i=0;i<MAXDIM;i++){
                                        j=(vartbl[VarIndex].dims[i] - mmb_options.base+1);
                                        if(j)card1 *= j;
                                }
                                a1int = (int64_t *)ptr1;
                                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
                        } else error("Argument 1 must be numerical");
                        if(a1float!=NULL){
                                for(i=0; i< card1;i++)sum+= (*a1float++);
                        } else {
                                for(i=0; i< card1;i++)sum+= (MMFLOAT)(*a1int++);
                        }
                        targ=T_NBR;
                        fret=sum;
                        return;
                }
        } else if(toupper(*ep)=='T') {

                tp = checkstring(ep, "TANH");
                if(tp) {
                        getargs(&tp, 1,",");
                        if(!(argc == 1)) error("Argument count");
                        fret=tanh(getnumber(argv[0]));
                        targ=T_NBR;
                        return;
                }
        }
        error("Syntax");
}
