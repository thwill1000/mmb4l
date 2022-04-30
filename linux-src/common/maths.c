/***********************************************************************************************************************
MMBasic

MATHS.c

Copyright 2016 - 2021 Peter Mather.  All Rights Reserved.

This file and modified versions of this file are supplied to specific individuals or organisations under the following
provisions:

- This file, or any files that comprise the MMBasic source (modified or not), may not be distributed or copied to any other
  person or organisation without written permission.

- Object files (.o and .hex files) generated using this file (modified or not) may not be distributed or copied to any other
  person or organisation without written permission.

- This file is provided in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

************************************************************************************************************************/

#include <math.h>
#include <complex.h>
#include <stdlib.h>
#include <string.h>

#include "mmb4l.h"
#include "error.h"
#include "maths.h"

#define ERROR_FAILED_TO_ALLOCATE_FLOAT_ARRAY(i)  error_throw_ex(kError, "Failed to allocate memory for %D float array!")
#define ERROR_ARRAY_SIZE_MUST_BE_POWER_OF_2      error_throw_ex(kError, "Array size must be power of 2")

//void cmd_SensorFusion(char *passcmdline);
void MahonyQuaternionUpdate(MMFLOAT ax, MMFLOAT ay, MMFLOAT az, MMFLOAT gx, MMFLOAT gy, MMFLOAT gz, MMFLOAT mx, MMFLOAT my, MMFLOAT mz, MMFLOAT Ki, MMFLOAT Kp, MMFLOAT deltat, MMFLOAT *yaw, MMFLOAT *pitch, MMFLOAT *roll);
void MadgwickQuaternionUpdate(MMFLOAT ax, MMFLOAT ay, MMFLOAT az, MMFLOAT gx, MMFLOAT gy, MMFLOAT gz, MMFLOAT mx, MMFLOAT my, MMFLOAT mz, MMFLOAT beta, MMFLOAT deltat, MMFLOAT *pitch, MMFLOAT *yaw, MMFLOAT *roll);

extern volatile unsigned int AHRSTimer;

MMFLOAT optionangle = 1.0;
MMFLOAT PI;
typedef MMFLOAT complex cplx;
void cmd_FFT(char *pp);
const double chitable[51][15]={
                {0.995,0.99,0.975,0.95,0.9,0.5,0.2,0.1,0.05,0.025,0.02,0.01,0.005,0.002,0.001},
                {0.0000397,0.000157,0.000982,0.00393,0.0158,0.455,1.642,2.706,3.841,5.024,5.412,6.635,7.879,9.550,10.828},
                {0.0100,0.020,0.051,0.103,0.211,1.386,3.219,4.605,5.991,7.378,7.824,9.210,10.597,12.429,13.816},
                {0.072,0.115,0.216,0.352,0.584,2.366,4.642,6.251,7.815,9.348,9.837,11.345,12.838,14.796,16.266},
                {0.207,0.297,0.484,0.711,1.064,3.357,5.989,7.779,9.488,11.143,11.668,13.277,14.860,16.924,18.467},
                {0.412,0.554,0.831,1.145,1.610,4.351,7.289,9.236,11.070,12.833,13.388,15.086,16.750,18.907,20.515},
                {0.676,0.872,1.237,1.635,2.204,5.348,8.558,10.645,12.592,14.449,15.033,16.812,18.548,20.791,22.458},
                {0.989,1.239,1.690,2.167,2.833,6.346,9.803,12.017,14.067,16.013,16.622,18.475,20.278,22.601,24.322},
                {1.344,1.646,2.180,2.733,3.490,7.344,11.030,13.362,15.507,17.535,18.168,20.090,21.955,24.352,26.124},
                {1.735,2.088,2.700,3.325,4.168,8.343,12.242,14.684,16.919,19.023,19.679,21.666,23.589,26.056,27.877},
                {2.156,2.558,3.247,3.940,4.865,9.342,13.442,15.987,18.307,20.483,21.161,23.209,25.188,27.722,29.588},
                {2.603,3.053,3.816,4.575,5.578,10.341,14.631,17.275,19.675,21.920,22.618,24.725,26.757,29.354,31.264},
                {3.074,3.571,4.404,5.226,6.304,11.340,15.812,18.549,21.026,23.337,24.054,26.217,28.300,30.957,32.909},
                {3.565,4.107,5.009,5.892,7.042,12.340,16.985,19.812,22.362,24.736,25.472,27.688,29.819,32.535,34.528},
                {4.075,4.660,5.629,6.571,7.790,13.339,18.151,21.064,23.685,26.119,26.873,29.141,31.319,34.091,36.123},
                {4.601,5.229,6.262,7.261,8.547,14.339,19.311,22.307,24.996,27.488,28.259,30.578,32.801,35.628,37.697},
                {5.142,5.812,6.908,7.962,9.312,15.338,20.465,23.542,26.296,28.845,29.633,32.000,34.267,37.146,39.252},
                {5.697,6.408,7.564,8.672,10.085,16.338,21.615,24.769,27.587,30.191,30.995,33.409,35.718,38.648,40.790},
                {6.265,7.015,8.231,9.390,10.865,17.338,22.760,25.989,28.869,31.526,32.346,34.805,37.156,40.136,42.312},
                {6.844,7.633,8.907,10.117,11.651,18.338,23.900,27.204,30.144,32.852,33.687,36.191,38.582,41.610,43.820},
                {7.434,8.260,9.591,10.851,12.443,19.337,25.038,28.412,31.410,34.170,35.020,37.566,39.997,43.072,45.315},
                {8.034,8.897,10.283,11.591,13.240,20.337,26.171,29.615,32.671,35.479,36.343,38.932,41.401,44.522,46.797},
                {8.643,9.542,10.982,12.338,14.041,21.337,27.301,30.813,33.924,36.781,37.659,40.289,42.796,45.962,48.268},
                {9.260,10.196,11.689,13.091,14.848,22.337,28.429,32.007,35.172,38.076,38.968,41.638,44.181,47.391,49.728},
                {9.886,10.856,12.401,13.848,15.659,23.337,29.553,33.196,36.415,39.364,40.270,42.980,45.559,48.812,51.179},
                {10.520,11.524,13.120,14.611,16.473,24.337,30.675,34.382,37.652,40.646,41.566,44.314,46.928,50.223,52.620},
                {11.160,12.198,13.844,15.379,17.292,25.336,31.795,35.563,38.885,41.923,42.856,45.642,48.290,51.627,54.052},
                {11.808,12.879,14.573,16.151,18.114,26.336,32.912,36.741,40.113,43.195,44.140,46.963,49.645,53.023,55.476},
                {12.461,13.565,15.308,16.928,18.939,27.336,34.027,37.916,41.337,44.461,45.419,48.278,50.993,54.411,56.892},
                {13.121,14.256,16.047,17.708,19.768,28.336,35.139,39.087,42.557,45.722,46.693,49.588,52.336,55.792,58.301},
                {13.787,14.953,16.791,18.493,20.599,29.336,36.250,40.256,43.773,46.979,47.962,50.892,53.672,57.167,59.703},
                {14.458,15.655,17.539,19.281,21.434,30.336,37.359,41.422,44.985,48.232,49.226,52.191,55.003,58.536,61.098},
                {15.134,16.362,18.291,20.072,22.271,31.336,38.466,42.585,46.194,49.480,50.487,53.486,56.328,59.899,62.487},
                {15.815,17.074,19.047,20.867,23.110,32.336,39.572,43.745,47.400,50.725,51.743,54.776,57.648,61.256,63.870},
                {16.501,17.789,19.806,21.664,23.952,33.336,40.676,44.903,48.602,51.966,52.995,56.061,58.964,62.608,65.247},
                {17.192,18.509,20.569,22.465,24.797,34.336,41.778,46.059,49.802,53.203,54.244,57.342,60.275,63.955,66.619},
                {17.887,19.233,21.336,23.269,25.643,35.336,42.879,47.212,50.998,54.437,55.489,58.619,61.581,65.296,67.985},
                {18.586,19.960,22.106,24.075,26.492,36.336,43.978,48.363,52.192,55.668,56.730,59.892,62.883,66.633,69.346},
                {19.289,20.691,22.878,24.884,27.343,37.335,45.076,49.513,53.384,56.896,57.969,61.162,64.181,67.966,70.703},
                {19.996,21.426,23.654,25.695,28.196,38.335,46.173,50.660,54.572,58.120,59.204,62.428,65.476,69.294,72.055},
                {20.707,22.164,24.433,26.509,29.051,39.335,47.269,51.805,55.758,59.342,60.436,63.691,66.766,70.618,73.402},
                {21.421,22.906,25.215,27.326,29.907,40.335,48.363,52.949,56.942,60.561,61.665,64.950,68.053,71.938,74.745},
                {22.138,23.650,25.999,28.144,30.765,41.335,49.456,54.090,58.124,61.777,62.892,66.206,69.336,73.254,76.084},
                {22.859,24.398,26.785,28.965,31.625,42.335,50.548,55.230,59.304,62.990,64.116,67.459,70.616,74.566,77.419},
                {23.584,25.148,27.575,29.787,32.487,43.335,51.639,56.369,60.481,64.201,65.337,68.710,71.893,75.874,78.750},
                {24.311,25.901,28.366,30.612,33.350,44.335,52.729,57.505,61.656,65.410,66.555,69.957,73.166,77.179,80.077},
                {25.041,26.657,29.160,31.439,34.215,45.335,53.818,58.641,62.830,66.617,67.771,71.201,74.437,78.481,81.400},
                {25.775,27.416,29.956,32.268,35.081,46.335,54.906,59.774,64.001,67.821,68.985,72.443,75.704,79.780,82.720},
                {26.511,28.177,30.755,33.098,35.949,47.335,55.993,60.907,65.171,69.023,70.197,73.683,76.969,81.075,84.037},
                {27.249,28.941,31.555,33.930,36.818,48.335,57.079,62.038,66.339,70.222,71.406,74.919,78.231,82.367,85.351},
                {27.991,29.707,32.357,34.764,37.689,49.335,58.164,63.167,67.505,71.420,72.613,76.154,79.490,83.657,86.661}
};
MMFLOAT q[4]={1,0,0,0};
MMFLOAT eInt[3]={0,0,0};
void transpose(MMFLOAT **matrix,MMFLOAT **matrix_cofactor,MMFLOAT **newmatrix, int size);

void PRet(void){
    MMPrintString("\r\n");
}

void PFlt(MMFLOAT flt){
    char s[20];
    FloatToStr(s, flt, 4, 4, ' ');
    MMPrintString(s);
}

void PFltComma(MMFLOAT n) {
    MMPrintString(", "); PFlt(n);
}

void PInt(int64_t n) {
    char s[20];
    IntToStr(s, (int64_t)n, 10);
    MMPrintString(s);
}

void PIntComma(int64_t n) {
    MMPrintString(", "); PInt(n);
}

void floatshellsort(MMFLOAT a[], int n) {
    long h, l, j;
    MMFLOAT k;
    for (h = n; h /= 2;) {
        for (l = h; l < n; l++) {
            k = a[l];
            for (j = l; j >= h &&  k < a[j - h]; j -= h) {
                a[j] = a[j - h];
            }
            a[j] = k;
        }
    }
}

MMFLOAT *alloc1df(int n)
{
//    int i;
    MMFLOAT* array;
    if ((array = (MMFLOAT*) GetMemory(n * sizeof(MMFLOAT))) == NULL) {
        ERROR_FAILED_TO_ALLOCATE_FLOAT_ARRAY(1);
        exit(0);
    }

//    for (i = 0; i < n; i++) {
//        array[i] = 0.0;
//    }

    return array;
}

MMFLOAT **alloc2df(int m, int n)
{
    int i;
    MMFLOAT** array;
    if ((array = (MMFLOAT **) GetMemory(m * sizeof(MMFLOAT*))) == NULL) {
        ERROR_FAILED_TO_ALLOCATE_FLOAT_ARRAY(2);
        exit(0);
    }

    for (i = 0; i < m; i++) {
        array[i] = alloc1df(n);
    }

    return array;
}

void dealloc2df(MMFLOAT **array, int m, int n)
{
    int i;
    for (i = 0; i < m; i++) {
        // FreeMemorySafe((void *)&array[i]);
                FreeMemory(array[i]);
    }

    // FreeMemorySafe((void *)&array);
        FreeMemory(array);
}

void Q_Mult(MMFLOAT *q1, MMFLOAT *q2, MMFLOAT *n){
    MMFLOAT a1=q1[0],a2=q2[0],b1=q1[1],b2=q2[1],c1=q1[2],c2=q2[2],d1=q1[3],d2=q2[3];
    n[0]=a1*a2-b1*b2-c1*c2-d1*d2;
    n[1]=a1*b2+b1*a2+c1*d2-d1*c2;
    n[2]=a1*c2-b1*d2+c1*a2+d1*b2;
    n[3]=a1*d2+b1*c2-c1*b2+d1*a2;
    n[4]=q1[4]*q2[4];
}

void Q_Invert(MMFLOAT *q, MMFLOAT *n){
    n[0]=q[0];
    n[1]=-q[1];
    n[2]=-q[2];
    n[3]=-q[3];
    n[4]=q[4];
}

/*void fft(MMFLOAT* data, unsigned long nn)
{
    unsigned long n, mmax, m, j, istep, i;
    MMFLOAT wtemp, wr, wpr, wpi, wi, theta;
    MMFLOAT tempr, tempi;

    // reverse-binary reindexing
    n = nn<<1;
    j=1;
    for (i=1; i<n; i+=2) {
        if (j>i) {
            swap(data[j-1], data[i-1]);
            swap(data[j], data[i]);
        }
        m = nn;
        while (m>=2 && j>m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    };

    // here begins the Danielson-Lanczos section
    mmax=2;
    while (n>mmax) {
        istep = mmax<<1;
        theta = -(2*M_PI/mmax);
        wtemp = sin(0.5*theta);
        wpr = -2.0*wtemp*wtemp;
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;
        for (m=1; m < mmax; m += 2) {
            for (i=m; i <= n; i += istep) {
                j=i+mmax;
                tempr = wr*data[j-1] - wi*data[j];
                tempi = wr * data[j] + wi*data[j-1];

                data[j-1] = data[i-1] - tempr;
                data[j] = data[i] - tempi;
                data[i-1] += tempr;
                data[i] += tempi;
            }
            wtemp=wr;
            wr += wr*wpr - wi*wpi;
            wi += wi*wpr + wtemp*wpi;
        }
        mmax=istep;
    }
}*/
static size_t reverse_bits(size_t val, int width) {
        size_t result = 0;
        for (int i = 0; i < width; i++, val >>= 1)
                result = (result << 1) | (val & 1U);
        return result;
}

bool Fft_transformRadix2(double complex vec[], size_t n, bool inverse) {
        // Length variables
        int levels = 0;  // Compute levels = floor(log2(n))
        for (size_t temp = n; temp > 1U; temp >>= 1)
                levels++;
        if ((size_t)1U << levels != n)
                return false;  // n is not a power of 2

        // Trigonometric tables
        if (SIZE_MAX / sizeof(double complex) < n / 2)
                return false;
        double complex *exptable = GetMemory((n / 2) * sizeof(double complex));
        if (exptable == NULL)
                return false;
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


void maths_fft(char *pp) {
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    char *tp;
        PI = atan2(1, 1) * 4;
    cplx *a1cplx=NULL, *a2cplx=NULL;
    MMFLOAT *a3float=NULL, *a4float=NULL, *a5float;
    int i, size, powerof2=0;
        tp = checkstring(pp, "MAGNITUDE");
        if(tp) {
                getargs(&tp,3,",");
            ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
            if (vartbl[VarIndex].type & T_NBR) {
                if(vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                if(vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_FLOAT_ARRAY(1);
                a3float = (MMFLOAT *)ptr1;
                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
            } else ERROR_ARG_NOT_FLOAT_ARRAY(1);
            size=(vartbl[VarIndex].dims[0] - mmb_options.base);
            ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
            if(vartbl[VarIndex].type & T_NBR) {
                 if(vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                 if(vartbl[VarIndex].dims[0] <= 0 ) ERROR_ARG_NOT_FLOAT_ARRAY(2);
                 a4float = (MMFLOAT *)ptr2;
                 if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
            } else ERROR_ARG_NOT_FLOAT_ARRAY(2);
            if((vartbl[VarIndex].dims[0] - mmb_options.base) !=size) ERROR_ARRAY_SIZE_MISMATCH;
            for(i=1;i<65536;i*=2){
                    if(size==i-1)powerof2=1;
            }
            if (!powerof2) ERROR_ARRAY_SIZE_MUST_BE_POWER_OF_2;
        a1cplx=(cplx *)GetTempMemory((size+1)*16);
        a5float=(MMFLOAT *)a1cplx;
        for(i=0;i<=size;i++){a5float[i*2]=a3float[i];a5float[i*2+1]=0;}
        Fft_transformRadix2(a1cplx, size+1, 0);
//            fft((MMFLOAT *)a1cplx,size+1);
            for(i=0;i<=size;i++)a4float[i]=cabs(a1cplx[i]);
                return;
        }
        tp = checkstring(pp, "PHASE");
        if(tp) {
                getargs(&tp,3,",");
            ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
            if(vartbl[VarIndex].type & T_NBR) {
                if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_FLOAT_ARRAY(1);
                a3float = (MMFLOAT *)ptr1;
                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
            } else ERROR_ARG_NOT_FLOAT_ARRAY(1);
            size=(vartbl[VarIndex].dims[0] - mmb_options.base);
            ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
            if(vartbl[VarIndex].type & T_NBR) {
                 if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                 if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_FLOAT_ARRAY(2);
                 a4float = (MMFLOAT *)ptr2;
                 if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
            } else ERROR_ARG_NOT_FLOAT_ARRAY(2);
            if((vartbl[VarIndex].dims[0] - mmb_options.base) !=size) ERROR_ARRAY_SIZE_MISMATCH;
            for(i=1;i<65536;i*=2){
                    if(size==i-1)powerof2=1;
            }
            if (!powerof2) ERROR_ARRAY_SIZE_MUST_BE_POWER_OF_2;
        a1cplx=(cplx *)GetTempMemory((size+1)*16);
        a5float=(MMFLOAT *)a1cplx;
        for(i=0;i<=size;i++){a5float[i*2]=a3float[i];a5float[i*2+1]=0;}
        Fft_transformRadix2(a1cplx, size+1, 0);
//            fft((MMFLOAT *)a1cplx,size+1);
            for(i=0;i<=size;i++)a4float[i]=carg(a1cplx[i]);
                return;
        }
        tp = checkstring(pp, "INVERSE");
        if(tp) {
                getargs(&tp,3,",");
            ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
            if(vartbl[VarIndex].type & T_NBR) {
                if (vartbl[VarIndex].dims[1] <= 0) ERROR_INVALID_VARIABLE;
                if (vartbl[VarIndex].dims[2] != 0) ERROR_INVALID_VARIABLE;
                if (vartbl[VarIndex].dims[0] - mmb_options.base != 1) ERROR_ARG_NOT_2D_FLOAT_ARRAY(1);
                a1cplx = (cplx *)ptr1;
                if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
            } else ERROR_ARG_NOT_2D_FLOAT_ARRAY(1);
            size=(vartbl[VarIndex].dims[1] - mmb_options.base);
            ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
            if(vartbl[VarIndex].type & T_NBR) {
                 if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
                 if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_FLOAT_ARRAY(2);
                 a3float = (MMFLOAT *)ptr2;
                 if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
            } else ERROR_ARG_NOT_FLOAT_ARRAY(2);
            if((vartbl[VarIndex].dims[0] - mmb_options.base) != size) ERROR_ARRAY_SIZE_MISMATCH;
            for(i=1;i<65536;i*=2){
                    if(size==i-1)powerof2=1;
            }
            if (!powerof2) ERROR_ARRAY_SIZE_MUST_BE_POWER_OF_2;
        a2cplx=(cplx *)GetTempMemory((size+1)*16);
            memcpy(a2cplx,a1cplx,(size+1)*16);
            for(i=0;i<=size;i++)a2cplx[i]=conj(a2cplx[i]);
        Fft_transformRadix2(a2cplx, size+1, 0);
//            fft((MMFLOAT *)a2cplx,size+1);
            for(i=0;i<=size;i++)a2cplx[i]=conj(a2cplx[i])/(cplx)(size+1);
            for(i=0;i<=size;i++)a3float[i]=creal(a2cplx[i]);
            return;
        }
        getargs(&pp,3,",");
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if(vartbl[VarIndex].type & T_NBR) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 1) ERROR_ARG_NOT_FLOAT_ARRAY(1);
        a3float = (MMFLOAT *)ptr1;
        if ((char *) ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
    } else ERROR_ARG_NOT_FLOAT_ARRAY(1);
    size=(vartbl[VarIndex].dims[0] - mmb_options.base);
    ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if(vartbl[VarIndex].type & T_NBR) {
         if (vartbl[VarIndex].dims[1] <= 0) ERROR_INVALID_VARIABLE;
         if (vartbl[VarIndex].dims[2] != 0) ERROR_INVALID_VARIABLE;
         if (vartbl[VarIndex].dims[0] - mmb_options.base != 1) ERROR_ARG_NOT_2D_FLOAT_ARRAY(2);
         a2cplx = (cplx *)ptr2;
         if ((char *) ptr2 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
    } else ERROR_ARG_NOT_2D_FLOAT_ARRAY(2);
    if((vartbl[VarIndex].dims[1] - mmb_options.base) !=size) ERROR_ARRAY_SIZE_MISMATCH;
    for(i=1;i<65536;i*=2){
            if(size==i-1)powerof2=1;
    }
    if (!powerof2) ERROR_ARRAY_SIZE_MUST_BE_POWER_OF_2;
    a4float=(MMFLOAT *)a2cplx;
    for(i=0;i<=size;i++){a4float[i*2]=a3float[i];a4float[i*2+1]=0;}
//    fft((MMFLOAT *)a2cplx,size+1);
    Fft_transformRadix2(a2cplx, size+1, 0);
}

// void cmd_SensorFusion(char *passcmdline){
//     char *p;
//     if((p = checkstring(passcmdline, "MADGWICK")) != NULL) {
//     getargs(&p, 25,",");
//     if(argc < 23) ERROR_ARGUMENT_COUNT;
//         MMFLOAT t;
//         MMFLOAT *pitch, *yaw, *roll;
//         MMFLOAT ax; MMFLOAT ay; MMFLOAT az; MMFLOAT gx; MMFLOAT gy; MMFLOAT gz; MMFLOAT mx; MMFLOAT my; MMFLOAT mz; MMFLOAT beta;
//         ax=getnumber(argv[0]);
//         ay=getnumber(argv[2]);
//         az=getnumber(argv[4]);
//         gx=getnumber(argv[6]);
//         gy=getnumber(argv[8]);
//         gz=getnumber(argv[10]);
//         mx=getnumber(argv[12]);
//         my=getnumber(argv[14]);
//         mz=getnumber(argv[16]);
//         pitch = findvar(argv[18], V_FIND);
//         if(!(vartbl[VarIndex].type & T_NBR)) ERROR_INVALID_VARIABLE;
//         roll = findvar(argv[20], V_FIND);
//         if(!(vartbl[VarIndex].type & T_NBR)) ERROR_INVALID_VARIABLE;
//         yaw = findvar(argv[22], V_FIND);
//         if(!(vartbl[VarIndex].type & T_NBR)) ERROR_INVALID_VARIABLE;
//         beta = 0.5;
//         if(argc == 25) beta=getnumber(argv[24]);
//         t=(MMFLOAT)AHRSTimer/1000.0;
//         if(t>1.0)t=1.0;
//         AHRSTimer=0;
//         MadgwickQuaternionUpdate(ax, ay, az, gx, gy, gz, mx, my, mz, beta, t, pitch, yaw, roll);
//         return;
//     }
//     if((p = checkstring(passcmdline, "MAHONY")) != NULL) {
//     getargs(&p, 27,",");
//     if(argc < 23) ERROR_INVALID_ARGUMENT_COUNT;
//         MMFLOAT t;
//         MMFLOAT *pitch, *yaw, *roll;
//         MMFLOAT Kp, Ki;
//         MMFLOAT ax; MMFLOAT ay; MMFLOAT az; MMFLOAT gx; MMFLOAT gy; MMFLOAT gz; MMFLOAT mx; MMFLOAT my; MMFLOAT mz;
//         ax=getnumber(argv[0]);
//         ay=getnumber(argv[2]);
//         az=getnumber(argv[4]);
//         gx=getnumber(argv[6]);
//         gy=getnumber(argv[8]);
//         gz=getnumber(argv[10]);
//         mx=getnumber(argv[12]);
//         my=getnumber(argv[14]);
//         mz=getnumber(argv[16]);
//         pitch = findvar(argv[18], V_FIND);
//         if(!(vartbl[VarIndex].type & T_NBR)) ERROR_INVALID_VARIABLE;
//         roll = findvar(argv[20], V_FIND);
//         if(!(vartbl[VarIndex].type & T_NBR)) ERROR_INVALID_VARIABLE;
//         yaw = findvar(argv[22], V_FIND);
//         if(!(vartbl[VarIndex].type & T_NBR)) ERROR_INVALID_VARIABLE;
//         Kp=10.0 ; Ki=0.0;
//         if(argc >= 25)Kp=getnumber(argv[24]);
//         if(argc == 27)Ki=getnumber(argv[26]);
//         t=(MMFLOAT)AHRSTimer/1000.0;
//         if(t>1.0)t=1.0;
//         AHRSTimer=0;
//         MahonyQuaternionUpdate(ax, ay, az, gx, gy, gz, mx, my, mz, Ki, Kp, t, yaw, pitch, roll) ;
//         return;
//     }
//     ERROR_UNKNOWN_SUBCOMMAND;
// }

void MadgwickQuaternionUpdate(MMFLOAT ax, MMFLOAT ay, MMFLOAT az, MMFLOAT gx, MMFLOAT gy, MMFLOAT gz, MMFLOAT mx, MMFLOAT my, MMFLOAT mz, MMFLOAT beta, MMFLOAT deltat, MMFLOAT *pitch, MMFLOAT *yaw, MMFLOAT *roll)
        {
            MMFLOAT q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];   // short name local variable for readability
            MMFLOAT norm;
            MMFLOAT hx, hy, _2bx, _2bz;
            MMFLOAT s1, s2, s3, s4;
            MMFLOAT qDot1, qDot2, qDot3, qDot4;

            // Auxiliary variables to avoid repeated arithmetic
            MMFLOAT _2q1mx;
            MMFLOAT _2q1my;
            MMFLOAT _2q1mz;
            MMFLOAT _2q2mx;
            MMFLOAT _4bx;
            MMFLOAT _4bz;
            MMFLOAT _2q1 = 2.0 * q1;
            MMFLOAT _2q2 = 2.0 * q2;
            MMFLOAT _2q3 = 2.0 * q3;
            MMFLOAT _2q4 = 2.0 * q4;
            MMFLOAT _2q1q3 = 2.0 * q1 * q3;
            MMFLOAT _2q3q4 = 2.0 * q3 * q4;
            MMFLOAT q1q1 = q1 * q1;
            MMFLOAT q1q2 = q1 * q2;
            MMFLOAT q1q3 = q1 * q3;
            MMFLOAT q1q4 = q1 * q4;
            MMFLOAT q2q2 = q2 * q2;
            MMFLOAT q2q3 = q2 * q3;
            MMFLOAT q2q4 = q2 * q4;
            MMFLOAT q3q3 = q3 * q3;
            MMFLOAT q3q4 = q3 * q4;
            MMFLOAT q4q4 = q4 * q4;

            // Normalise accelerometer measurement
            norm = sqrt(ax * ax + ay * ay + az * az);
            if (norm == 0.0) return; // handle NaN
            norm = 1.0/norm;
            ax *= norm;
            ay *= norm;
            az *= norm;

            // Normalise magnetometer measurement
            norm = sqrt(mx * mx + my * my + mz * mz);
            if (norm == 0.0) return; // handle NaN
            norm = 1.0/norm;
            mx *= norm;
            my *= norm;
            mz *= norm;

            // Reference direction of Earth's magnetic field
            _2q1mx = 2.0 * q1 * mx;
            _2q1my = 2.0 * q1 * my;
            _2q1mz = 2.0 * q1 * mz;
            _2q2mx = 2.0 * q2 * mx;
            hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
            hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
            _2bx = sqrt(hx * hx + hy * hy);
            _2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
            _4bx = 2.0 * _2bx;
            _4bz = 2.0 * _2bz;

            // Gradient decent algorithm corrective step
            s1 = -_2q3 * (2.0 * q2q4 - _2q1q3 - ax) + _2q2 * (2.0 * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
            s2 = _2q4 * (2.0 * q2q4 - _2q1q3 - ax) + _2q1 * (2.0 * q1q2 + _2q3q4 - ay) - 4.0 * q2 * (1.0 - 2.0 * q2q2 - 2.0 * q3q3 - az) + _2bz * q4 * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
            s3 = -_2q1 * (2.0 * q2q4 - _2q1q3 - ax) + _2q4 * (2.0 * q1q2 + _2q3q4 - ay) - 4.0 * q3 * (1.0 - 2.0 * q2q2 - 2.0 * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
            s4 = _2q2 * (2.0 * q2q4 - _2q1q3 - ax) + _2q3 * (2.0 * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
            norm = sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);    // normalise step magnitude
            norm = 1.0/norm;
            s1 *= norm;
            s2 *= norm;
            s3 *= norm;
            s4 *= norm;

            // Compute rate of change of quaternion
            qDot1 = 0.5 * (-q2 * gx - q3 * gy - q4 * gz) - beta * s1;
            qDot2 = 0.5 * (q1 * gx + q3 * gz - q4 * gy) - beta * s2;
            qDot3 = 0.5 * (q1 * gy - q2 * gz + q4 * gx) - beta * s3;
            qDot4 = 0.5 * (q1 * gz + q2 * gy - q3 * gx) - beta * s4;

            // Integrate to yield quaternion
            q1 += qDot1 * deltat;
            q2 += qDot2 * deltat;
            q3 += qDot3 * deltat;
            q4 += qDot4 * deltat;
            norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);    // normalise quaternion
            norm = 1.0/norm;
            q[0] = q1 * norm;
            q[1] = q2 * norm;
            q[2] = q3 * norm;
            q[3] = q4 * norm;

            MMFLOAT ysqr = q3 * q3;


            // roll (x-axis rotation)
            MMFLOAT t0 = +2.0 * (q1 * q2 + q3 * q4);
            MMFLOAT t1 = +1.0 - 2.0 * (q2 * q2 + ysqr);
            *roll = atan2(t0, t1);

            // pitch (y-axis rotation)
            MMFLOAT t2 = +2.0 * (q1 * q3 - q4 * q2);
            t2 = t2 > 1.0 ? 1.0 : t2;
            t2 = t2 < -1.0 ? -1.0 : t2;
            *pitch = asin(t2);

            // yaw (z-axis rotation)
            MMFLOAT t3 = +2.0 * (q1 * q4 + q2 *q3);
            MMFLOAT t4 = +1.0 - 2.0 * (ysqr + q4 * q4);
            *yaw = atan2(t3, t4);

}
void MahonyQuaternionUpdate(MMFLOAT ax, MMFLOAT ay, MMFLOAT az, MMFLOAT gx, MMFLOAT gy, MMFLOAT gz, MMFLOAT mx, MMFLOAT my, MMFLOAT mz, MMFLOAT Ki, MMFLOAT Kp, MMFLOAT deltat, MMFLOAT *yaw, MMFLOAT *pitch, MMFLOAT *roll)        {
            MMFLOAT q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];   // short name local variable for readability
            MMFLOAT norm;
            MMFLOAT hx, hy, bx, bz;
            MMFLOAT vx, vy, vz, wx, wy, wz;
            MMFLOAT ex, ey, ez;
            MMFLOAT pa, pb, pc;

            // Auxiliary variables to avoid repeated arithmetic
            MMFLOAT q1q1 = q1 * q1;
            MMFLOAT q1q2 = q1 * q2;
            MMFLOAT q1q3 = q1 * q3;
            MMFLOAT q1q4 = q1 * q4;
            MMFLOAT q2q2 = q2 * q2;
            MMFLOAT q2q3 = q2 * q3;
            MMFLOAT q2q4 = q2 * q4;
            MMFLOAT q3q3 = q3 * q3;
            MMFLOAT q3q4 = q3 * q4;
            MMFLOAT q4q4 = q4 * q4;

            // Normalise accelerometer measurement
            norm = sqrt(ax * ax + ay * ay + az * az);
            if (norm == 0.0) return; // handle NaN
            norm = 1.0 / norm;        // use reciprocal for division
            ax *= norm;
            ay *= norm;
            az *= norm;

            // Normalise magnetometer measurement
            norm = sqrt(mx * mx + my * my + mz * mz);
            if (norm == 0.0) return; // handle NaN
            norm = 1.0 / norm;        // use reciprocal for division
            mx *= norm;
            my *= norm;
            mz *= norm;

            // Reference direction of Earth's magnetic field
            hx = 2.0 * mx * (0.5 - q3q3 - q4q4) + 2.0 * my * (q2q3 - q1q4) + 2.0 * mz * (q2q4 + q1q3);
            hy = 2.0 * mx * (q2q3 + q1q4) + 2.0 * my * (0.5 - q2q2 - q4q4) + 2.0 * mz * (q3q4 - q1q2);
            bx = sqrt((hx * hx) + (hy * hy));
            bz = 2.0 * mx * (q2q4 - q1q3) + 2.0 * my * (q3q4 + q1q2) + 2.0 * mz * (0.5 - q2q2 - q3q3);

            // Estimated direction of gravity and magnetic field
            vx = 2.0 * (q2q4 - q1q3);
            vy = 2.0 * (q1q2 + q3q4);
            vz = q1q1 - q2q2 - q3q3 + q4q4;
            wx = 2.0 * bx * (0.5 - q3q3 - q4q4) + 2.0 * bz * (q2q4 - q1q3);
            wy = 2.0 * bx * (q2q3 - q1q4) + 2.0 * bz * (q1q2 + q3q4);
            wz = 2.0 * bx * (q1q3 + q2q4) + 2.0 * bz * (0.5 - q2q2 - q3q3);

            // Error is cross product between estimated direction and measured direction of gravity
            ex = (ay * vz - az * vy) + (my * wz - mz * wy);
            ey = (az * vx - ax * vz) + (mz * wx - mx * wz);
            ez = (ax * vy - ay * vx) + (mx * wy - my * wx);
            if (Ki > 0.0)
            {
                eInt[0] += ex;      // accumulate integral error
                eInt[1] += ey;
                eInt[2] += ez;
            }
            else
            {
                eInt[0] = 0.0;     // prevent integral wind up
                eInt[1] = 0.0;
                eInt[2] = 0.0;
            }

            // Apply feedback terms
            gx = gx + Kp * ex + Ki * eInt[0];
            gy = gy + Kp * ey + Ki * eInt[1];
            gz = gz + Kp * ez + Ki * eInt[2];

            // Integrate rate of change of quaternion
            pa = q2;
            pb = q3;
            pc = q4;
            q1 = q1 + (-q2 * gx - q3 * gy - q4 * gz) * (0.5 * deltat);
            q2 = pa + (q1 * gx + pb * gz - pc * gy) * (0.5 * deltat);
            q3 = pb + (q1 * gy - pa * gz + pc * gx) * (0.5 * deltat);
            q4 = pc + (q1 * gz + pa * gy - pb * gx) * (0.5 * deltat);

            // Normalise quaternion
            norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);
            norm = 1.0 / norm;
            q[0] = q1 * norm;
            q[1] = q2 * norm;
            q[2] = q3 * norm;
            q[3] = q4 * norm;
            MMFLOAT ysqr = q3 * q3;


            // roll (x-axis rotation)
            MMFLOAT t0 = +2.0 * (q1 * q2 + q3 * q4);
            MMFLOAT t1 = +1.0 - 2.0 * (q2 * q2 + ysqr);
            *roll = atan2(t0, t1);

            // pitch (y-axis rotation)
            MMFLOAT t2 = +2.0 * (q1 * q3 - q4 * q2);
            t2 = t2 > 1.0 ? 1.0 : t2;
            t2 = t2 < -1.0 ? -1.0 : t2;
            *pitch = asin(t2);

            // yaw (z-axis rotation)
            MMFLOAT t3 = +2.0 * (q1 * q4 + q2 *q3);
            MMFLOAT t4 = +1.0 - 2.0 * (ysqr + q4 * q4);
            *yaw = atan2(t3, t4);
        }

/*Finding transpose of cofactor of matrix*/
void transpose(MMFLOAT **matrix,MMFLOAT **matrix_cofactor,MMFLOAT **newmatrix, int size)
{
    int i,j;
    MMFLOAT d;
        MMFLOAT **m_transpose=alloc2df(size,size);
        MMFLOAT **m_inverse=alloc2df(size,size);

    for (i=0;i<size;i++)
    {
        for (j=0;j<size;j++)
        {
            m_transpose[i][j]=matrix_cofactor[j][i];
        }
    }
    d=determinant(matrix,size);
    for (i=0;i<size;i++)
    {
        for (j=0;j<size;j++)
        {
            m_inverse[i][j]=m_transpose[i][j] / d;
        }
    }

    for (i=0;i<size;i++)
    {
        for (j=0;j<size;j++)
        {
            newmatrix[i][j]=m_inverse[i][j];
        }
    }
        dealloc2df(m_transpose,size,size);
        dealloc2df(m_inverse,size,size);
}
/*calculate cofactor of matrix*/
void cofactor(MMFLOAT **matrix,MMFLOAT **newmatrix,int size)
{
        MMFLOAT **m_cofactor=alloc2df(size,size);
        MMFLOAT **matrix_cofactor=alloc2df(size,size);
    int p,q,m,n,i,j;
    for (q=0;q<size;q++)
    {
        for (p=0;p<size;p++)
        {
            m=0;
            n=0;
            for (i=0;i<size;i++)
            {
                for (j=0;j<size;j++)
                {
                    if (i != q && j != p)
                    {
                       m_cofactor[m][n]=matrix[i][j];
                       if (n<(size-2))
                          n++;
                       else
                       {
                           n=0;
                           m++;
                       }
                    }
                }
            }
            matrix_cofactor[q][p]=pow(-1,q + p) * determinant(m_cofactor,size-1);
        }
    }
    transpose(matrix, matrix_cofactor, newmatrix, size);
        dealloc2df(m_cofactor,size,size);
        dealloc2df(matrix_cofactor,size,size);

}
/*For calculating Determinant of the Matrix . this function is recursive*/
MMFLOAT determinant(MMFLOAT **matrix,int size)
{
   MMFLOAT s=1,det=0;
   MMFLOAT **m_minor=alloc2df(size,size);
   int i,j,m,n,c;
   if (size==1)
   {
       return (matrix[0][0]);
   }
   else
   {
       det=0;
       for (c=0;c<size;c++)
       {
           m=0;
           n=0;
           for (i=0;i<size;i++)
           {
               for (j=0;j<size;j++)
               {
                   m_minor[i][j]=0;
                   if (i != 0 && j != c)
                   {
                      m_minor[m][n]=matrix[i][j];
                      if (n<(size-2))
                         n++;
                      else
                      {
                          n=0;
                          m++;
                      }
                   }
               }
           }
           det=det + s * (matrix[0][c] * determinant(m_minor,size-1));
           s=-1 * s;
       }
   }
   dealloc2df(m_minor,size,size);
   return (det);
}
