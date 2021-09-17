#if !defined(MMB4L_MATHS_H)
#define MMB4L_MATHS_H

#include <stdint.h>

#include "../Configuration.h"

extern MMFLOAT optionangle;
extern const double chitable[51][15];

MMFLOAT *alloc1df(int n);
MMFLOAT **alloc2df(int m, int n);
void cofactor(MMFLOAT **matrix, MMFLOAT **newmatrix, int size);
void dealloc2df(MMFLOAT **array, int m, int n);
MMFLOAT determinant(MMFLOAT **matrix, int size);
void floatshellsort(MMFLOAT a[], int n);
void PRet(void);
void PFlt(MMFLOAT flt);
void PFltComma(MMFLOAT n);
void PInt(int64_t n);
void PIntComma(int64_t n);
void Q_Invert(MMFLOAT *q, MMFLOAT *n);
void Q_Mult(MMFLOAT *q1, MMFLOAT *q2, MMFLOAT *n);

void maths_fft(char *pp);

#endif
