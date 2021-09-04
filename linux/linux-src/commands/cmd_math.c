#include <complex.h>
#include <stdint.h>

#include "../common/version.h"

typedef MMFLOAT complex cplx;

MMFLOAT PI;

static size_t reverse_bits(size_t val, int width) {
    size_t result = 0;
    for (int i = 0; i < width; i++, val >>= 1)
        result = (result << 1) | (val & 1U);
    return result;
}

static int Fft_transformRadix2(double complex vec[], size_t n, int inverse) {
    // Length variables
    int levels = 0;  // Compute levels = floor(log2(n))
    for (size_t temp = n; temp > 1U; temp >>= 1) levels++;
    if ((size_t)1U << levels != n) return false;  // n is not a power of 2

    // Trigonometric tables
    if (SIZE_MAX / sizeof(double complex) < n / 2) return false;
    double complex *exptable = GetMemory((n / 2) * sizeof(double complex));
    if (exptable == NULL) return false;
    for (size_t i = 0; i < n / 2; i++)
        exptable[i] = cexp((inverse ? 2 : -2) * M_PI * i / n * I);

    // Bit-reversed addressing permutation
    for (size_t i = 0; i < n; i++) {
        size_t j = reverse_bits(i, levels);
        if (j > i) {
            double complex temp = vec[i];
            vec[i] = vec[j];
            vec[j] = temp;
        }
    }

    // Cooley-Tukey decimation-in-time radix-2 FFT
    for (size_t size = 2; size <= n; size *= 2) {
        size_t halfsize = size / 2;
        size_t tablestep = n / size;
        for (size_t i = 0; i < n; i += size) {
            for (size_t j = i, k = 0; j < i + halfsize; j++, k += tablestep) {
                size_t l = j + halfsize;
                double complex temp = vec[l] * exptable[k];
                vec[l] = vec[j] - temp;
                vec[j] += temp;
            }
        }
        if (size == n)  // Prevent overflow in 'size *= 2'
            break;
    }

    FreeMemory(exptable);
    return true;
}

static void math_fft(char *p) {
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    char *tp;
    PI = atan2(1, 1) * 4;
    cplx *a1cplx = NULL, *a2cplx = NULL;
    MMFLOAT *a3float = NULL, *a4float = NULL, *a5float;
    int i, size, powerof2 = 0;
    tp = checkstring(p, "MAGNITUDE");
    if (tp) {
        getargs(&tp, 3, ",");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if (vartbl[VarIndex].type & T_NBR) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be a floating point array");
            }
            a3float = (MMFLOAT *)ptr1;
            if (ptr1 != vartbl[VarIndex].val.s)
                error("Syntax");
        } else
            error("Argument 1 must be a floating point array");
        size = (vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if (vartbl[VarIndex].type & T_NBR) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 2 must be a floating point array");
            }
            a4float = (MMFLOAT *)ptr2;
            if (ptr2 != vartbl[VarIndex].val.s)
                error("Syntax");
        } else
            error("Argument 2 must be a floating point array");
        if ((vartbl[VarIndex].dims[0] - OptionBase) != size)
            error("Array size mismatch");
        for (i = 1; i < 65536; i *= 2) {
            if (size == i - 1) powerof2 = 1;
        }
        if (!powerof2) error("array size must be a power of 2");
        a1cplx = (cplx *)GetTempMemory((size + 1) * 16);
        a5float = (MMFLOAT *)a1cplx;
        for (i = 0; i <= size; i++) {
            a5float[i * 2] = a3float[i];
            a5float[i * 2 + 1] = 0;
        }
        Fft_transformRadix2(a1cplx, size + 1, 0);
        //            fft((MMFLOAT *)a1cplx,size+1);
        for (i = 0; i <= size; i++) a4float[i] = cabs(a1cplx[i]);
        return;
    }
    tp = checkstring(p, "PHASE");
    if (tp) {
        getargs(&tp, 3, ",");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if (vartbl[VarIndex].type & T_NBR) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be a floating point array");
            }
            a3float = (MMFLOAT *)ptr1;
            if (ptr1 != vartbl[VarIndex].val.s)
                error("Syntax");
        } else
            error("Argument 1 must be a floating point array");
        size = (vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if (vartbl[VarIndex].type & T_NBR) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 2 must be a floating point array");
            }
            a4float = (MMFLOAT *)ptr2;
            if (ptr2 != vartbl[VarIndex].val.s)
                error("Syntax");
        } else
            error("Argument 2 must be a floating point array");
        if ((vartbl[VarIndex].dims[0] - OptionBase) != size)
            error("Array size mismatch");
        for (i = 1; i < 65536; i *= 2) {
            if (size == i - 1) powerof2 = 1;
        }
        if (!powerof2) error("array size must be a power of 2");
        a1cplx = (cplx *)GetTempMemory((size + 1) * 16);
        a5float = (MMFLOAT *)a1cplx;
        for (i = 0; i <= size; i++) {
            a5float[i * 2] = a3float[i];
            a5float[i * 2 + 1] = 0;
        }
        Fft_transformRadix2(a1cplx, size + 1, 0);
        //            fft((MMFLOAT *)a1cplx,size+1);
        for (i = 0; i <= size; i++) a4float[i] = carg(a1cplx[i]);
        return;
    }
    tp = checkstring(p, "INVERSE");
    if (tp) {
        getargs(&tp, 3, ",");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if (vartbl[VarIndex].type & T_NBR) {
            if (vartbl[VarIndex].dims[1] <= 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[2] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] - OptionBase != 1) {  // Not an array
                error("Argument 1 must be a 2D floating point array");
            }
            a1cplx = (cplx *)ptr1;
            if (ptr1 != vartbl[VarIndex].val.s)
                error("Syntax");
        } else
            error("Argument 1 must be a 2D floating point array");
        size = (vartbl[VarIndex].dims[1] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if (vartbl[VarIndex].type & T_NBR) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 2 must be a floating point array");
            }
            a3float = (MMFLOAT *)ptr2;
            if (ptr2 != vartbl[VarIndex].val.s)
                error("Syntax");
        } else
            error("Argument 2 must be a floating point array");
        if ((vartbl[VarIndex].dims[0] - OptionBase) != size)
            error("Array size mismatch");
        for (i = 1; i < 65536; i *= 2) {
            if (size == i - 1) powerof2 = 1;
        }
        if (!powerof2) error("array size must be a power of 2");
        a2cplx = (cplx *)GetTempMemory((size + 1) * 16);
        memcpy(a2cplx, a1cplx, (size + 1) * 16);
        for (i = 0; i <= size; i++) a2cplx[i] = conj(a2cplx[i]);
        Fft_transformRadix2(a2cplx, size + 1, 0);
        //            fft((MMFLOAT *)a2cplx,size+1);
        for (i = 0; i <= size; i++)
            a2cplx[i] = conj(a2cplx[i]) / (cplx)(size + 1);
        for (i = 0; i <= size; i++) a3float[i] = creal(a2cplx[i]);
        return;
    }
    getargs(&p, 3, ",");
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if (vartbl[VarIndex].type & T_NBR) {
        if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[0] <= 1) {  // Not an array
            error("Argument 1 must be a floating point array");
        }
        a3float = (MMFLOAT *)ptr1;
        if (ptr1 != vartbl[VarIndex].val.s) error("Syntax");
    } else
        error("Argument 1 must be a floating point array");
    size = (vartbl[VarIndex].dims[0] - OptionBase);
    ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if (vartbl[VarIndex].type & T_NBR) {
        if (vartbl[VarIndex].dims[1] <= 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[2] != 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[0] - OptionBase != 1) {  // Not an array
            error("Argument 2 must be a 2D floating point array");
        }
        a2cplx = (cplx *)ptr2;
        if (ptr2 != vartbl[VarIndex].val.s) error("Syntax");
    } else
        error("Argument 2 must be a 2D floating point array");
    if ((vartbl[VarIndex].dims[1] - OptionBase) != size)
        error("Array size mismatch");
    for (i = 1; i < 65536; i *= 2) {
        if (size == i - 1) powerof2 = 1;
    }
    if (!powerof2) error("array size must be a power of 2");
    a4float = (MMFLOAT *)a2cplx;
    for (i = 0; i <= size; i++) {
        a4float[i * 2] = a3float[i];
        a4float[i * 2 + 1] = 0;
    }
    //    fft((MMFLOAT *)a2cplx,size+1);
    Fft_transformRadix2(a2cplx, size + 1, 0);
}

static void math_set(char *p) {
    int t = T_NBR;
    MMFLOAT f;
    long long int i64;
    char *s;
    void *ptr1 = NULL;
    int i, j, card1 = 1;
    MMFLOAT *a1float = NULL;
    int64_t *a1int = NULL;
    getargs(&p, 3, ",");
    if (!(argc == 3)) error("Argument count");
    evaluate(argv[0], &f, &i64, &s, &t, false);
    if (t & T_STR) error("Syntax");
    ptr1 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if (vartbl[VarIndex].type & T_NBR) {
        card1 = 1;
        for (i = 0; i < MAXDIM; i++) {
            j = (vartbl[VarIndex].dims[i] - OptionBase + 1);
            if (j) card1 *= j;
        }
        a1float = (MMFLOAT *)ptr1;
        if (ptr1 != vartbl[VarIndex].val.s) error("Syntax");
    } else if (vartbl[VarIndex].type & T_INT) {
        card1 = 1;
        for (i = 0; i < MAXDIM; i++) {
            j = (vartbl[VarIndex].dims[i] - OptionBase + 1);
            if (j) card1 *= j;
        }
        a1int = (int64_t *)ptr1;
        if (ptr1 != vartbl[VarIndex].val.s) error("Syntax");
    } else
        error("Argument 2 must be numerical");
    if (a1float != NULL) {
        for (i = 0; i < card1; i++)
            *a1float++ = ((t & T_INT) ? (MMFLOAT)i64 : f);
    } else {
        for (i = 0; i < card1; i++)
            *a1int++ = ((t & T_INT) ? i64 : FloatToInt64(f));
    }
}

void cmd_math(void) {
    char *p;
    if (p = checkstring(cmdline, "FFT")) {
        math_fft(p);
    } else if (p = checkstring(cmdline, "SET")) {
        math_set(p);
    } else {
        error("Syntax");
    }
}
