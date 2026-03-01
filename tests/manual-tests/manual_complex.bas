' ComplexTest.bas
' Comprehensive test suite for MMBasic MATH(C_...) complex number functions
' Complex numbers are stored as 32-bit real + 32-bit imaginary in a 64-bit integer
'
' Tolerance set for 32-bit float precision (~7 significant digits)

Option Default Integer
Option Base 0

Dim pass% = 0, fail% = 0, total% = 0
Dim tol! = 1e-4  ' tolerance for float comparisons

Print "=============================================="
Print " MMBasic Complex Number Function Test Suite"
Print "=============================================="
Print

' ==============================================
' Section 1: C_CPLX - Construction & Extraction
' ==============================================

Print "--- Section 1: C_CPLX Construction & C_REAL / C_IMAG Extraction ---"

' Test 1.1: Basic construction and extraction

Dim z1% = Math(C_CPLX 3.0, 4.0)
TestFloat "C_CPLX/C_REAL(3+4i)", Math(C_REAL z1%), 3.0
TestFloat "C_CPLX/C_IMAG(3+4i)", Math(C_IMAG z1%), 4.0

' Test 1.2: Negative components
Dim z2% = Math(C_CPLX -2.5, -7.1)
TestFloat "C_CPLX/C_REAL(-2.5-7.1i)", Math(C_REAL z2%), -2.5
TestFloat "C_CPLX/C_IMAG(-2.5-7.1i)", Math(C_IMAG z2%), -7.1

' Test 1.3: Zero complex number
Dim z0% = Math(C_CPLX 0.0, 0.0)
TestFloat "C_CPLX/C_REAL(0+0i)", Math(C_REAL z0%), 0.0
TestFloat "C_CPLX/C_IMAG(0+0i)", Math(C_IMAG z0%), 0.0

' Test 1.4: Purely real number
Dim zr% = Math(C_CPLX 5.0, 0.0)
TestFloat "C_REAL(5+0i)", Math(C_REAL zr%), 5.0
TestFloat "C_IMAG(5+0i)", Math(C_IMAG zr%), 0.0

' Test 1.5: Purely imaginary number
Dim zi% = Math(C_CPLX 0.0, 3.0)
TestFloat "C_REAL(0+3i)", Math(C_REAL zi%), 0.0
TestFloat "C_IMAG(0+3i)", Math(C_IMAG zi%), 3.0

' Test 1.6: Large values
Dim zl% = Math(C_CPLX 1e6, -1e6)
TestFloat "C_REAL(1e6-1e6i)", Math(C_REAL zl%), 1e6
TestFloat "C_IMAG(1e6-1e6i)", Math(C_IMAG zl%), -1e6

' Test 1.7: Small values
Dim zs% = Math(C_CPLX 1e-6, 2e-6)
TestFloat "C_REAL(1e-6+2e-6i)", Math(C_REAL zs%), 1e-6
TestFloat "C_IMAG(1e-6+2e-6i)", Math(C_IMAG zs%), 2e-6

Print

' ==============================================
' Section 2: C_POLAR - Polar Construction
' ==============================================

Print "--- Section 2: C_POLAR Construction ---"

' Test 2.1: r=1, theta=0 -> 1+0i
Dim zp1% = Math(C_POLAR 1.0, 0.0)
TestFloat "C_POLAR(1,0) real", Math(C_REAL zp1%), 1.0
TestFloat "C_POLAR(1,0) imag", Math(C_IMAG zp1%), 0.0

' Test 2.2: r=1, theta=pi/2 -> 0+1i (radians by default)
'Dim pi = 3.14159265358979
Dim zp2% = Math(C_POLAR 1.0, pi/2)
TestFloat "C_POLAR(1,pi/2) real", Math(C_REAL zp2%), 0.0
TestFloat "C_POLAR(1,pi/2) imag", Math(C_IMAG zp2%), 1.0

' Test 2.3: r=2, theta=pi -> -2+0i
Dim zp3% = Math(C_POLAR 2.0, pi)
TestFloat "C_POLAR(2,pi) real", Math(C_REAL zp3%), -2.0
TestFloat "C_POLAR(2,pi) imag", Math(C_IMAG zp3%), 0.0

' Test 2.4: r=5, theta=pi/4 -> 5*cos(pi/4)+5*sin(pi/4)i
Dim zp4% = Math(C_POLAR 5.0, pi/4)
TestFloat "C_POLAR(5,pi/4) real", Math(C_REAL zp4%), 5.0*Cos(pi/4)
TestFloat "C_POLAR(5,pi/4) imag", Math(C_IMAG zp4%), 5.0*Sin(pi/4)

' Test 2.5: r=0, theta=anything -> 0+0i
Dim zp5% = Math(C_POLAR 0.0, 1.234)
TestFloat "C_POLAR(0,1.234) real", Math(C_REAL zp5%), 0.0
TestFloat "C_POLAR(0,1.234) imag", Math(C_IMAG zp5%), 0.0

Print

' ==============================================
' Section 3: C_MOD, C_PHASE, C_CARG
' ==============================================

Print "--- Section 3: C_MOD, C_PHASE, C_CARG ---"

' Test 3.1: MOD of 3+4i = 5
Dim zm1% = Math(C_CPLX 3.0, 4.0)
TestFloat "C_MOD(3+4i)", Math(C_MOD zm1%), 5.0

' Test 3.2: MOD of 0+0i = 0
TestFloat "C_MOD(0+0i)", Math(C_MOD z0%), 0.0

' Test 3.3: MOD of purely real = abs(real)
TestFloat "C_MOD(5+0i)", Math(C_MOD zr%), 5.0

' Test 3.4: MOD of purely imaginary = abs(imag)
TestFloat "C_MOD(0+3i)", Math(C_MOD zi%), 3.0

' Test 3.5: MOD of -3-4i = 5
Dim zm2% = Math(C_CPLX -3.0, -4.0)
TestFloat "C_MOD(-3-4i)", Math(C_MOD zm2%), 5.0

' Test 3.6: PHASE of 1+1i = pi/4
Dim zph1% = Math(C_CPLX 1.0, 1.0)
TestFloat "C_PHASE(1+1i)", Math(C_PHASE zph1%), pi/4

' Test 3.7: PHASE of 1+0i = 0
TestFloat "C_PHASE(1+0i)", Math(C_PHASE zr%), 0.0

' Test 3.8: PHASE of 0+1i = pi/2
TestFloat "C_PHASE(0+1i)", Math(C_PHASE zi%), pi/2

' Test 3.9: PHASE of -1+0i = pi
Dim zneg1% = Math(C_CPLX -1.0, 0.0)
TestFloat "C_PHASE(-1+0i)", Math(C_PHASE zneg1%), pi

' Test 3.10: CARG of 1+1i = pi/4
TestFloat "C_CARG(1+1i)", Math(C_CARG zph1%), pi/4

' Test 3.11: CARG of -1-1i = -3*pi/4
Dim zca1% = Math(C_CPLX -1.0, -1.0)
TestFloat "C_CARG(-1-1i)", Math(C_CARG zca1%), -3*pi/4

Print

' ==============================================
' Section 4: C_ADD - Complex Addition
' ==============================================

Print "--- Section 4: C_ADD ---"

' Test 4.1: (3+4i) + (1+2i) = 4+6i
Dim za1% = Math(C_CPLX 1.0, 2.0)
Dim zadd1% = Math(C_ADD z1%, za1%)
TestFloat "C_ADD real (3+4i)+(1+2i)", Math(C_REAL zadd1%), 4.0
TestFloat "C_ADD imag (3+4i)+(1+2i)", Math(C_IMAG zadd1%), 6.0

' Test 4.2: Add zero
Dim zadd2% = Math(C_ADD z1%, z0%)
TestFloat "C_ADD real z+0", Math(C_REAL zadd2%), 3.0
TestFloat "C_ADD imag z+0", Math(C_IMAG zadd2%), 4.0

' Test 4.3: Add negative
Dim zn1% = Math(C_CPLX -3.0, -4.0)
Dim zadd3% = Math(C_ADD z1%, zn1%)
TestFloat "C_ADD real z+(-z)", Math(C_REAL zadd3%), 0.0
TestFloat "C_ADD imag z+(-z)", Math(C_IMAG zadd3%), 0.0

Print

' ==============================================
' Section 5: C_SUB - Complex Subtraction
' ==============================================

Print "--- Section 5: C_SUB ---"

' Test 5.1: (3+4i) - (1+2i) = 2+2i
Dim zsub1% = Math(C_SUB z1%, za1%)
TestFloat "C_SUB real (3+4i)-(1+2i)", Math(C_REAL zsub1%), 2.0
TestFloat "C_SUB imag (3+4i)-(1+2i)", Math(C_IMAG zsub1%), 2.0

' Test 5.2: Subtract zero
Dim zsub2% = Math(C_SUB z1%, z0%)
TestFloat "C_SUB real z-0", Math(C_REAL zsub2%), 3.0
TestFloat "C_SUB imag z-0", Math(C_IMAG zsub2%), 4.0

' Test 5.3: Subtract self = 0
Dim zsub3% = Math(C_SUB z1%, z1%)
TestFloat "C_SUB real z-z", Math(C_REAL zsub3%), 0.0
TestFloat "C_SUB imag z-z", Math(C_IMAG zsub3%), 0.0

Print

' ==============================================
' Section 6: C_MUL - Complex Multiplication
' ==============================================

Print "--- Section 6: C_MUL ---"

' Test 6.1: (3+4i)*(1+2i) = (3-8)+(6+4)i = -5+10i
Dim zmul1% = Math(C_MUL z1%, za1%)
TestFloat "C_MUL real (3+4i)*(1+2i)", Math(C_REAL zmul1%), -5.0
TestFloat "C_MUL imag (3+4i)*(1+2i)", Math(C_IMAG zmul1%), 10.0

' Test 6.2: Multiply by zero = 0
Dim zmul2% = Math(C_MUL z1%, z0%)
TestFloat "C_MUL real z*0", Math(C_REAL zmul2%), 0.0
TestFloat "C_MUL imag z*0", Math(C_IMAG zmul2%), 0.0

' Test 6.3: Multiply by 1+0i = identity
Dim zone% = Math(C_CPLX 1.0, 0.0)
Dim zmul3% = Math(C_MUL z1%, zone%)
TestFloat "C_MUL real z*1", Math(C_REAL zmul3%), 3.0
TestFloat "C_MUL imag z*1", Math(C_IMAG zmul3%), 4.0

' Test 6.4: i*i = -1
Dim zii% = Math(C_CPLX 0.0, 1.0)
Dim zmul4% = Math(C_MUL zii%, zii%)
TestFloat "C_MUL real i*i=-1", Math(C_REAL zmul4%), -1.0
TestFloat "C_MUL imag i*i=0", Math(C_IMAG zmul4%), 0.0

Print

' ==============================================
' Section 7: C_DIV - Complex Division
' ==============================================

Print "--- Section 7: C_DIV ---"

' Test 7.1: (3+4i)/(1+2i) = (3+8+(-6+4)i)/(1+4) = (11+(-2)i)/5 = 2.2-0.4i
Dim zdiv1% = Math(C_DIV z1%, za1%)
TestFloat "C_DIV real (3+4i)/(1+2i)", Math(C_REAL zdiv1%), 2.2
TestFloat "C_DIV imag (3+4i)/(1+2i)", Math(C_IMAG zdiv1%), -0.4

' Test 7.2: Divide by 1+0i = identity
Dim zdiv2% = Math(C_DIV z1%, zone%)
TestFloat "C_DIV real z/1", Math(C_REAL zdiv2%), 3.0
TestFloat "C_DIV imag z/1", Math(C_IMAG zdiv2%), 4.0

' Test 7.3: z/z = 1
Dim zdiv3% = Math(C_DIV z1%, z1%)
TestFloat "C_DIV real z/z=1", Math(C_REAL zdiv3%), 1.0
TestFloat "C_DIV imag z/z=0", Math(C_IMAG zdiv3%), 0.0

Print

' ==============================================
' Section 8: C_POW - Complex Power
' ==============================================

Print "--- Section 8: C_POW ---"

' Test 8.1: (1+1i)^2 = 2i
Dim zpow_base% = Math(C_CPLX 1.0, 1.0)
Dim zpow_exp% = Math(C_CPLX 2.0, 0.0)
Dim zpow1% = Math(C_POW zpow_base%, zpow_exp%)
TestFloat "C_POW real (1+i)^2", Math(C_REAL zpow1%), 0.0
TestFloat "C_POW imag (1+i)^2", Math(C_IMAG zpow1%), 2.0

' Test 8.2: z^1 = z
Dim zpow2% = Math(C_POW z1%, zone%)
TestFloat "C_POW real z^1=z", Math(C_REAL zpow2%), 3.0
TestFloat "C_POW imag z^1=z", Math(C_IMAG zpow2%), 4.0

' Test 8.3: z^0 = 1
Dim zpow3% = Math(C_POW z1%, z0%)
TestFloat "C_POW real z^0=1", Math(C_REAL zpow3%), 1.0
TestFloat "C_POW imag z^0=0", Math(C_IMAG zpow3%), 0.0

Print

' ==============================================
' Section 9: C_CONJ - Complex Conjugate
' ==============================================

Print "--- Section 9: C_CONJ ---"

' Test 9.1: conj(3+4i) = 3-4i
Dim zconj1% = Math(C_CONJ z1%)
TestFloat "C_CONJ real (3+4i)", Math(C_REAL zconj1%), 3.0
TestFloat "C_CONJ imag (3+4i)", Math(C_IMAG zconj1%), -4.0

' Test 9.2: conj(0+0i) = 0+0i
Dim zconj2% = Math(C_CONJ z0%)
TestFloat "C_CONJ real (0+0i)", Math(C_REAL zconj2%), 0.0
TestFloat "C_CONJ imag (0+0i)", Math(C_IMAG zconj2%), 0.0

' Test 9.3: conj(conj(z)) = z
Dim ztmp9% = Math(C_CONJ z1%)
Dim zconj3% = Math(C_CONJ ztmp9%)
TestFloat "C_CONJ(C_CONJ(z)) real", Math(C_REAL zconj3%), 3.0
TestFloat "C_CONJ(C_CONJ(z)) imag", Math(C_IMAG zconj3%), 4.0

' Test 9.4: z * conj(z) = |z|^2 (real number)
Dim zcc% = Math(C_MUL z1%, zconj1%)
TestFloat "z*conj(z) real=25", Math(C_REAL zcc%), 25.0
TestFloat "z*conj(z) imag=0", Math(C_IMAG zcc%), 0.0

Print

' ==============================================
' Section 10: C_ABS - Complex Absolute Value
' ==============================================

Print "--- Section 10: C_ABS ---"

' Test 10.1: abs(3+4i) = 5+0i
Dim zabs1% = Math(C_ABS z1%)
TestFloat "C_ABS(3+4i) real", Math(C_REAL zabs1%), 5.0
TestFloat "C_ABS(3+4i) imag", Math(C_IMAG zabs1%), 0.0

' Test 10.2: abs(0+0i) = 0
Dim zabs2% = Math(C_ABS z0%)
TestFloat "C_ABS(0+0i) real", Math(C_REAL zabs2%), 0.0

Print

' ==============================================
' Section 11: C_SQRT - Complex Square Root
' ==============================================

Print "--- Section 11: C_SQRT ---"

' Test 11.1: sqrt(-1+0i) = 0+1i (i)
Dim zsqrt_in1% = Math(C_CPLX -1.0, 0.0)
Dim zsqrt1% = Math(C_SQRT zsqrt_in1%)
TestFloat "C_SQRT(-1) real", Math(C_REAL zsqrt1%), 0.0
TestFloat "C_SQRT(-1) imag", Math(C_IMAG zsqrt1%), 1.0

' Test 11.2: sqrt(0+2i) - verify by squaring result
Dim zsqrt_in2% = Math(C_CPLX 0.0, 2.0)
Dim zsqrt2% = Math(C_SQRT zsqrt_in2%)
Dim zsqrt2_sq% = Math(C_MUL zsqrt2%, zsqrt2%)
TestFloat "C_SQRT(2i)^2 real", Math(C_REAL zsqrt2_sq%), 0.0
TestFloat "C_SQRT(2i)^2 imag", Math(C_IMAG zsqrt2_sq%), 2.0

' Test 11.3: sqrt(3+4i) - verify by squaring
Dim zsqrt3% = Math(C_SQRT z1%)
Dim zsqrt3_sq% = Math(C_MUL zsqrt3%, zsqrt3%)
TestFloat "C_SQRT(3+4i)^2 real", Math(C_REAL zsqrt3_sq%), 3.0
TestFloat "C_SQRT(3+4i)^2 imag", Math(C_IMAG zsqrt3_sq%), 4.0

' Test 11.4: sqrt(4+0i) = 2+0i
Dim zsqrt_in4% = Math(C_CPLX 4.0, 0.0)
Dim zsqrt4% = Math(C_SQRT zsqrt_in4%)
TestFloat "C_SQRT(4) real", Math(C_REAL zsqrt4%), 2.0
TestFloat "C_SQRT(4) imag", Math(C_IMAG zsqrt4%), 0.0

Print

' ==============================================
' Section 12: C_EXP - Complex Exponential
' ==============================================

Print "--- Section 12: C_EXP ---"

' Test 12.1: exp(0+0i) = 1+0i
Dim zexp1% = Math(C_EXP z0%)
TestFloat "C_EXP(0) real", Math(C_REAL zexp1%), 1.0
TestFloat "C_EXP(0) imag", Math(C_IMAG zexp1%), 0.0

' Test 12.2: exp(1+0i) = e+0i
Dim zexp_in2% = Math(C_CPLX 1.0, 0.0)
Dim zexp2% = Math(C_EXP zexp_in2%)
TestFloat "C_EXP(1) real=e", Math(C_REAL zexp2%), Exp(1)
TestFloat "C_EXP(1) imag=0", Math(C_IMAG zexp2%), 0.0

' Test 12.3: exp(0+pi*i) = -1+0i (Euler's identity)
Dim zexp_in3% = Math(C_CPLX 0.0, pi)
Dim zexp3% = Math(C_EXP zexp_in3%)
TestFloat "Euler: exp(i*pi) real=-1", Math(C_REAL zexp3%), -1.0
TestFloat "Euler: exp(i*pi) imag=0", Math(C_IMAG zexp3%), 0.0

' Test 12.4: exp(0+pi/2*i) = 0+1i
Dim zexp_in4% = Math(C_CPLX 0.0, pi/2)
Dim zexp4% = Math(C_EXP zexp_in4%)
TestFloat "exp(i*pi/2) real=0", Math(C_REAL zexp4%), 0.0
TestFloat "exp(i*pi/2) imag=1", Math(C_IMAG zexp4%), 1.0

Print

' ==============================================
' Section 13: C_LOG - Complex Natural Logarithm
' ==============================================

Print "--- Section 13: C_LOG ---"

' Test 13.1: log(1+0i) = 0+0i
Dim zlog_in1% = Math(C_CPLX 1.0, 0.0)
Dim zlog1% = Math(C_LOG zlog_in1%)
TestFloat "C_LOG(1) real=0", Math(C_REAL zlog1%), 0.0
TestFloat "C_LOG(1) imag=0", Math(C_IMAG zlog1%), 0.0

' Test 13.2: log(e+0i) = 1+0i
Dim e! = Exp(1)
Dim zlog_in2% = Math(C_CPLX e!, 0.0)
Dim zlog2% = Math(C_LOG zlog_in2%)
TestFloat "C_LOG(e) real=1", Math(C_REAL zlog2%), 1.0
TestFloat "C_LOG(e) imag=0", Math(C_IMAG zlog2%), 0.0

' Test 13.3: log(-1+0i) = 0+pi*i
Dim zlog3% = Math(C_LOG zneg1%)
TestFloat "C_LOG(-1) real=0", Math(C_REAL zlog3%), 0.0
TestFloat "C_LOG(-1) imag=pi", Math(C_IMAG zlog3%), pi

' Test 13.4: exp(log(z)) = z (round-trip)
Dim zlog_tmp% = Math(C_LOG z1%)
Dim zlog_rt% = Math(C_EXP zlog_tmp%)
TestFloat "exp(log(3+4i)) real", Math(C_REAL zlog_rt%), 3.0
TestFloat "exp(log(3+4i)) imag", Math(C_IMAG zlog_rt%), 4.0

Print

' ==============================================
' Section 14: C_SIN, C_COS, C_TAN
' ==============================================

Print "--- Section 14: C_SIN, C_COS, C_TAN ---"

' Test 14.1: sin(0+0i) = 0+0i
Dim zsin1% = Math(C_SIN z0%)
TestFloat "C_SIN(0) real", Math(C_REAL zsin1%), 0.0
TestFloat "C_SIN(0) imag", Math(C_IMAG zsin1%), 0.0

' Test 14.2: cos(0+0i) = 1+0i
Dim zcos1% = Math(C_COS z0%)
TestFloat "C_COS(0) real", Math(C_REAL zcos1%), 1.0
TestFloat "C_COS(0) imag", Math(C_IMAG zcos1%), 0.0

' Test 14.3: sin(pi/2+0i) = 1+0i
Dim zsin_in2% = Math(C_CPLX pi/2, 0.0)
Dim zsin2% = Math(C_SIN zsin_in2%)
TestFloat "C_SIN(pi/2) real=1", Math(C_REAL zsin2%), 1.0
TestFloat "C_SIN(pi/2) imag=0", Math(C_IMAG zsin2%), 0.0

' Test 14.4: cos(pi+0i) = -1+0i
Dim zcos_in2% = Math(C_CPLX pi, 0.0)
Dim zcos2% = Math(C_COS zcos_in2%)
TestFloat "C_COS(pi) real=-1", Math(C_REAL zcos2%), -1.0
TestFloat "C_COS(pi) imag=0", Math(C_IMAG zcos2%), 0.0

' Test 14.5: tan(0+0i) = 0+0i
Dim ztan1% = Math(C_TAN z0%)
TestFloat "C_TAN(0) real=0", Math(C_REAL ztan1%), 0.0
TestFloat "C_TAN(0) imag=0", Math(C_IMAG ztan1%), 0.0

' Test 14.6: sin^2 + cos^2 = 1 for complex argument
' sin^2(1+2i) + cos^2(1+2i) = 1+0i
Dim zsc_in% = Math(C_CPLX 1.0, 2.0)
Dim zsin_sc% = Math(C_SIN zsc_in%)
Dim zcos_sc% = Math(C_COS zsc_in%)
Dim zsin2_sc% = Math(C_MUL zsin_sc%, zsin_sc%)
Dim zcos2_sc% = Math(C_MUL zcos_sc%, zcos_sc%)
Dim zident% = Math(C_ADD zsin2_sc%, zcos2_sc%)
TestFloat "sin^2+cos^2 real=1", Math(C_REAL zident%), 1.0
TestFloat "sin^2+cos^2 imag=0", Math(C_IMAG zident%), 0.0

' Test 14.7: sin(1+2i) known value
' sin(1+2i) = sin(1)*cosh(2) + i*cos(1)*sinh(2)
Dim expected_sr! = Sin(1.0)*((Exp(2)+Exp(-2))/2)
Dim expected_si! = Cos(1.0)*((Exp(2)-Exp(-2))/2)
Dim zsin_12% = Math(C_SIN zsc_in%)
TestFloat "C_SIN(1+2i) real", Math(C_REAL zsin_12%), expected_sr!
TestFloat "C_SIN(1+2i) imag", Math(C_IMAG zsin_12%), expected_si!

' Test 14.8: tan = sin/cos
Dim ztan_12% = Math(C_TAN zsc_in%)
Dim ztan_check% = Math(C_DIV zsin_12%, zcos_sc%)
TestFloat "tan=sin/cos real", Math(C_REAL ztan_12%), Math(C_REAL ztan_check%)

TestFloat "tan=sin/cos imag", Math(C_IMAG ztan_12%), Math(C_IMAG ztan_check%)

Print

' ==============================================
' Section 15: C_ASIN, C_ACOS, C_ATAN
' ==============================================

Print "--- Section 15: C_ASIN, C_ACOS, C_ATAN ---"

' Test 15.1: asin(sin(z)) = z for z=0.5+0.5i
Dim ztrig_in% = Math(C_CPLX 0.5, 0.5)
Dim ztmp15a% = Math(C_SIN ztrig_in%)
Dim zasin_rt% = Math(C_ASIN ztmp15a%)
TestFloat "asin(sin(z)) real", Math(C_REAL zasin_rt%), 0.5
TestFloat "asin(sin(z)) imag", Math(C_IMAG zasin_rt%), 0.5

' Test 15.2: acos(cos(z)) = z for z=0.5+0.5i
Dim ztmp15b% = Math(C_COS ztrig_in%)
Dim zacos_rt% = Math(C_ACOS ztmp15b%)
TestFloat "acos(cos(z)) real", Math(C_REAL zacos_rt%), 0.5
TestFloat "acos(cos(z)) imag", Math(C_IMAG zacos_rt%), 0.5

' Test 15.3: atan(tan(z)) = z for z=0.5+0.5i
Dim ztmp15c% = Math(C_TAN ztrig_in%)
Dim zatan_rt% = Math(C_ATAN ztmp15c%)
TestFloat "atan(tan(z)) real", Math(C_REAL zatan_rt%), 0.5
TestFloat "atan(tan(z)) imag", Math(C_IMAG zatan_rt%), 0.5

' Test 15.4: asin(0) = 0
Dim zasin0% = Math(C_ASIN z0%)
TestFloat "C_ASIN(0) real=0", Math(C_REAL zasin0%), 0.0
TestFloat "C_ASIN(0) imag=0", Math(C_IMAG zasin0%), 0.0

' Test 15.5: acos(1+0i) = 0+0i
Dim zacos1_in% = Math(C_CPLX 1.0, 0.0)
Dim zacos1% = Math(C_ACOS zacos1_in%)
TestFloat "C_ACOS(1) real=0", Math(C_REAL zacos1%), 0.0
TestFloat "C_ACOS(1) imag=0", Math(C_IMAG zacos1%), 0.0

' Test 15.6: atan(0) = 0
Dim zatan0% = Math(C_ATAN z0%)
TestFloat "C_ATAN(0) real=0", Math(C_REAL zatan0%), 0.0
TestFloat "C_ATAN(0) imag=0", Math(C_IMAG zatan0%), 0.0

Print

' ==============================================
' Section 16: C_SINH, C_COSH, C_TANH
' ==============================================

Print "--- Section 16: C_SINH, C_COSH, C_TANH ---"

' Test 16.1: sinh(0+0i) = 0+0i
Dim zsinh1% = Math(C_SINH z0%)
TestFloat "C_SINH(0) real=0", Math(C_REAL zsinh1%), 0.0
TestFloat "C_SINH(0) imag=0", Math(C_IMAG zsinh1%), 0.0

' Test 16.2: cosh(0+0i) = 1+0i
Dim zcosh1% = Math(C_COSH z0%)
TestFloat "C_COSH(0) real=1", Math(C_REAL zcosh1%), 1.0
TestFloat "C_COSH(0) imag=0", Math(C_IMAG zcosh1%), 0.0

' Test 16.3: tanh(0+0i) = 0+0i
Dim ztanh1% = Math(C_TANH z0%)
TestFloat "C_TANH(0) real=0", Math(C_REAL ztanh1%), 0.0
TestFloat "C_TANH(0) imag=0", Math(C_IMAG ztanh1%), 0.0

' Test 16.4: sinh(1+0i) = sinh(1)+0i
Dim zsinh_in2% = Math(C_CPLX 1.0, 0.0)
Dim zsinh2% = Math(C_SINH zsinh_in2%)
Dim expected_sinh1! = (Exp(1)-Exp(-1))/2
TestFloat "C_SINH(1) real", Math(C_REAL zsinh2%), expected_sinh1!
TestFloat "C_SINH(1) imag=0", Math(C_IMAG zsinh2%), 0.0

' Test 16.5: cosh(1+0i) = cosh(1)+0i
Dim zcosh2% = Math(C_COSH zsinh_in2%)
Dim expected_cosh1! = (Exp(1)+Exp(-1))/2
TestFloat "C_COSH(1) real", Math(C_REAL zcosh2%), expected_cosh1!
TestFloat "C_COSH(1) imag=0", Math(C_IMAG zcosh2%), 0.0

' Test 16.6: cosh^2 - sinh^2 = 1 for complex
Dim zhyp_in% = Math(C_CPLX 1.0, 2.0)
Dim zsinh_h% = Math(C_SINH zhyp_in%)
Dim zcosh_h% = Math(C_COSH zhyp_in%)
Dim zsinh2_h% = Math(C_MUL zsinh_h%, zsinh_h%)
Dim zcosh2_h% = Math(C_MUL zcosh_h%, zcosh_h%)
Dim zhident% = Math(C_SUB zcosh2_h%, zsinh2_h%)
TestFloat "cosh^2-sinh^2 real=1", Math(C_REAL zhident%), 1.0
TestFloat "cosh^2-sinh^2 imag=0", Math(C_IMAG zhident%), 0.0

' Test 16.7: tanh = sinh/cosh
Dim ztanh_h% = Math(C_TANH zhyp_in%)
Dim ztanh_check% = Math(C_DIV zsinh_h%, zcosh_h%)
TestFloat "tanh=sinh/cosh real", Math(C_REAL ztanh_h%), Math(C_REAL ztanh_check%)
TestFloat "tanh=sinh/cosh imag", Math(C_IMAG ztanh_h%), Math(C_IMAG ztanh_check%)

Print

' ==============================================
' Section 17: C_ASINH, C_ACOSH, C_ATANH
' ==============================================

Print "--- Section 17: C_ASINH, C_ACOSH, C_ATANH ---"

' Test 17.1: asinh(sinh(z)) = z for z=0.5+0.5i
Dim ztmp17a% = Math(C_SINH ztrig_in%)
Dim zasinh_rt% = Math(C_ASINH ztmp17a%)
TestFloat "asinh(sinh(z)) real", Math(C_REAL zasinh_rt%), 0.5
TestFloat "asinh(sinh(z)) imag", Math(C_IMAG zasinh_rt%), 0.5

' Test 17.2: acosh(cosh(z)) = z for z=0.5+0.5i
Dim ztmp17b% = Math(C_COSH ztrig_in%)
Dim zacosh_rt% = Math(C_ACOSH ztmp17b%)
TestFloat "acosh(cosh(z)) real", Math(C_REAL zacosh_rt%), 0.5
TestFloat "acosh(cosh(z)) imag", Math(C_IMAG zacosh_rt%), 0.5

' Test 17.3: atanh(tanh(z)) = z for z=0.5+0.5i
Dim ztmp17c% = Math(C_TANH ztrig_in%)
Dim zatanh_rt% = Math(C_ATANH ztmp17c%)
TestFloat "atanh(tanh(z)) real", Math(C_REAL zatanh_rt%), 0.5
TestFloat "atanh(tanh(z)) imag", Math(C_IMAG zatanh_rt%), 0.5

' Test 17.4: asinh(0) = 0
Dim zasinh0% = Math(C_ASINH z0%)
TestFloat "C_ASINH(0) real=0", Math(C_REAL zasinh0%), 0.0
TestFloat "C_ASINH(0) imag=0", Math(C_IMAG zasinh0%), 0.0

' Test 17.5: acosh(1) = 0
Dim zacosh1_in% = Math(C_CPLX 1.0, 0.0)
Dim zacosh1% = Math(C_ACOSH zacosh1_in%)
TestFloat "C_ACOSH(1) real=0", Math(C_REAL zacosh1%), 0.0
TestFloat "C_ACOSH(1) imag=0", Math(C_IMAG zacosh1%), 0.0

' Test 17.6: atanh(0) = 0
Dim zatanh0% = Math(C_ATANH z0%)
TestFloat "C_ATANH(0) real=0", Math(C_REAL zatanh0%), 0.0
TestFloat "C_ATANH(0) imag=0", Math(C_IMAG zatanh0%), 0.0

Print

' ==============================================
' Section 18: C_PROJ - Projection onto Riemann Sphere
' ==============================================

Print "--- Section 18: C_PROJ ---"

' Test 18.1: proj of finite number = itself
Dim zproj1% = Math(C_PROJ z1%)
TestFloat "C_PROJ(3+4i) real", Math(C_REAL zproj1%), 3.0
TestFloat "C_PROJ(3+4i) imag", Math(C_IMAG zproj1%), 4.0

' Test 18.2: proj(0+0i) = 0+0i
Dim zproj2% = Math(C_PROJ z0%)
TestFloat "C_PROJ(0+0i) real", Math(C_REAL zproj2%), 0.0
TestFloat "C_PROJ(0+0i) imag", Math(C_IMAG zproj2%), 0.0

Print

' ==============================================
' Section 19: Chained / Composite Operations
' ==============================================

Print "--- Section 19: Chained / Composite Operations ---"

' Test 19.1: (a+b) - b = a
Dim zch_a% = Math(C_CPLX 2.0, 3.0)
Dim zch_b% = Math(C_CPLX 5.0, -1.0)
Dim zch_sum% = Math(C_ADD zch_a%, zch_b%)
Dim zch_res% = Math(C_SUB zch_sum%, zch_b%)
TestFloat "chain (a+b)-b real", Math(C_REAL zch_res%), 2.0
TestFloat "chain (a+b)-b imag", Math(C_IMAG zch_res%), 3.0

' Test 19.2: (a*b)/b = a
Dim zch_prod% = Math(C_MUL zch_a%, zch_b%)
Dim zch_res2% = Math(C_DIV zch_prod%, zch_b%)
TestFloat "chain (a*b)/b real", Math(C_REAL zch_res2%), 2.0
TestFloat "chain (a*b)/b imag", Math(C_IMAG zch_res2%), 3.0

' Test 19.3: exp(log(z)) = z
Dim zch_in3% = Math(C_CPLX 2.0, -1.0)
Dim zch_tmp3% = Math(C_LOG zch_in3%)
Dim zch_res3% = Math(C_EXP zch_tmp3%)
TestFloat "exp(log(z)) real", Math(C_REAL zch_res3%), 2.0
TestFloat "exp(log(z)) imag", Math(C_IMAG zch_res3%), -1.0

' Test 19.4: sqrt(z)^2 = z
Dim zch_in4% = Math(C_CPLX -2.0, 3.0)
Dim zch_sq% = Math(C_SQRT zch_in4%)
Dim zch_res4% = Math(C_MUL zch_sq%, zch_sq%)
TestFloat "sqrt(z)^2 real", Math(C_REAL zch_res4%), -2.0
TestFloat "sqrt(z)^2 imag", Math(C_IMAG zch_res4%), 3.0

' Test 19.5: z + conj(z) = 2*Re(z) (purely real)
Dim zch_in5% = Math(C_CPLX 7.0, -3.0)
Dim zch_conj5% = Math(C_CONJ zch_in5%)
Dim zch_res5% = Math(C_ADD zch_in5%, zch_conj5%)
TestFloat "z+conj(z) real=14", Math(C_REAL zch_res5%), 14.0
TestFloat "z+conj(z) imag=0", Math(C_IMAG zch_res5%), 0.0

' Test 19.6: z - conj(z) = 2i*Im(z) (purely imaginary)
Dim zch_res6% = Math(C_SUB zch_in5%, zch_conj5%)
TestFloat "z-conj(z) real=0", Math(C_REAL zch_res6%), 0.0
TestFloat "z-conj(z) imag=-6", Math(C_IMAG zch_res6%), -6.0

Print

' ==============================================
' Section 20: Edge Cases & Special Values
' ==============================================

Print "--- Section 20: Edge Cases & Special Values ---"

' Test 20.1: Operations with purely real numbers
Dim zre_a% = Math(C_CPLX 3.0, 0.0)
Dim zre_b% = Math(C_CPLX 2.0, 0.0)
Dim zre_add% = Math(C_ADD zre_a%, zre_b%)
TestFloat "real+real", Math(C_REAL zre_add%), 5.0
TestFloat "real+real imag=0", Math(C_IMAG zre_add%), 0.0

' Test 20.2: Operations with purely imaginary numbers
Dim zim_a% = Math(C_CPLX 0.0, 3.0)
Dim zim_b% = Math(C_CPLX 0.0, 2.0)
Dim zim_mul% = Math(C_MUL zim_a%, zim_b%)
TestFloat "imag*imag real=-6", Math(C_REAL zim_mul%), -6.0
TestFloat "imag*imag imag=0", Math(C_IMAG zim_mul%), 0.0

' Test 20.3: Very small imaginary part
Dim zvs% = Math(C_CPLX 1.0, 1e-7)
Dim zvs_mod! = Math(C_MOD zvs%)
TestFloat "mod(1+tiny*i)~1", zvs_mod!, 1.0

' Test 20.4: Distributive property: a*(b+c) = a*b + a*c
Dim zd_a% = Math(C_CPLX 2.0, 1.0)
Dim zd_b% = Math(C_CPLX 3.0, -2.0)
Dim zd_c% = Math(C_CPLX -1.0, 4.0)
Dim zd_bc% = Math(C_ADD zd_b%, zd_c%)
Dim zd_lhs% = Math(C_MUL zd_a%, zd_bc%)
Dim zd_ab% = Math(C_MUL zd_a%, zd_b%)
Dim zd_ac% = Math(C_MUL zd_a%, zd_c%)
Dim zd_rhs% = Math(C_ADD zd_ab%, zd_ac%)
TestFloat "distrib real", Math(C_REAL zd_lhs%), Math(C_REAL zd_rhs%)
TestFloat "distrib imag", Math(C_IMAG zd_lhs%), Math(C_IMAG zd_rhs%)

' Test 20.5: Commutativity of addition: a+b = b+a
Dim zcom_ab% = Math(C_ADD zd_a%, zd_b%)
Dim zcom_ba% = Math(C_ADD zd_b%, zd_a%)
TestFloat "a+b=b+a real", Math(C_REAL zcom_ab%), Math(C_REAL zcom_ba%)
TestFloat "a+b=b+a imag", Math(C_IMAG zcom_ab%), Math(C_IMAG zcom_ba%)

' Test 20.6: Commutativity of multiplication: a*b = b*a
Dim zmcom_ab% = Math(C_MUL zd_a%, zd_b%)
Dim zmcom_ba% = Math(C_MUL zd_b%, zd_a%)
TestFloat "a*b=b*a real", Math(C_REAL zmcom_ab%), Math(C_REAL zmcom_ba%)
TestFloat "a*b=b*a imag", Math(C_IMAG zmcom_ab%), Math(C_IMAG zmcom_ba%)

Print

' ==============================================
' Final Summary
' ==============================================

Print "=============================================="
Print " TEST SUMMARY"
Print "=============================================="
Print " Total tests: "; total%
Print " Passed:      "; pass%
Print " Failed:      "; fail%
Print "=============================================="

If fail% = 0 Then
  Print " ALL TESTS PASSED"
Else
  Print " *** "; fail%; " TEST(S) FAILED ***"
EndIf

Print "=============================================="

End

' ==============================================
' Test Helper Subroutine
' ==============================================

Sub TestFloat(name$, got!, expected!)
Local diff!
total% = total% + 1
If expected! = 0 Then
  diff! = Abs(got!)
Else
  diff! = Abs((got! - expected!) / expected!)
EndIf
If diff! <= tol! Or (expected! = 0 And Abs(got!) < tol!) Then
  pass% = pass% + 1
  Print "  PASS: "; name$
Else
  fail% = fail% + 1
  Print "  FAIL: "; name$; "  expected="; expected!; "  got="; got; "  diff="; diff!
EndIf
End Sub
