/*-*****************************************************************************

MMBasic for Linux (MMB4L)

utility.h

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
   be displayed on the console at startup (additional copyright messages may
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

#if !defined(MMBASIC_UTILITY_H)
#define MMBASIC_UTILITY_H

#include <stddef.h>

// To output the value of a macro during compilation do:
//   #pragma message(VAR_NAME_VALUE(macro))
#define stringify(a) #a
#define xstringify(a) stringify(a)
#define VAR_NAME_VALUE(var) #var "=" xstringify(var)

// =============================================================================
// SUPPRESS WARNINGS
// =============================================================================

/**
 * Macros to suppress specific warnings
 */
#if defined(_MSC_VER)

#define DIAGNOSTIC_IGNORE_ARRAY_BOUNDS
#define DIAGNOSTIC_IGNORE_CLOBBERED
#define DIAGNOSTIC_IGNORE_MAYBE_UNINITIALIZED
#define DIAGNOSTIC_IGNORE_UNUSED_VARIABLE
#define DIAGNOSTIC_RESTORE

#else

#define DIAGNOSTIC_IGNORE_ARRAY_BOUNDS \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Warray-bounds\"")

#define DIAGNOSTIC_IGNORE_UNUSED_VARIABLE \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wunused-variable\"")

#define DIAGNOSTIC_RESTORE \
    _Pragma("GCC diagnostic pop")

#if defined(__clang__)

#define DIAGNOSTIC_IGNORE_CLOBBERED \
    _Pragma("GCC diagnostic push")

#define DIAGNOSTIC_IGNORE_MAYBE_UNINITIALIZED  \
    _Pragma("GCC diagnostic push")

#else

#define DIAGNOSTIC_IGNORE_CLOBBERED \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wclobbered\"")

#define DIAGNOSTIC_IGNORE_MAYBE_UNINITIALIZED \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")

#endif // #if defined(__clang__)

#endif // #if defined(_MSC_VER)

// =============================================================================
// ALIGNMENT ATTRIBUTES
// =============================================================================

/**
 * ALIGNED_PREFIX(n) - Specify alignment for a type or variable (prefix)
 * ALIGNED_SUFFIX(n) - Specify alignment as suffix (for compatibility)
 *
 * Usage:
 *   ALIGNED_PREFIX(8) struct foo { ... };    // Align struct to 8 bytes
 *   ALIGNED_PREFIX(16) int x;                // Align variable to 16 bytes
 *
 *   union bar {
 *       int x;
 *   } ALIGNED_SUFFIX(8) my_union;            // Suffix form for union members
 */
#ifdef _MSC_VER
    #define ALIGNED_PREFIX(n) __declspec(align(n))
    #define ALIGNED_SUFFIX(n)
    #define ALIGNED_VAR(n, type) __declspec(align(n)) type
#elif defined(__GNUC__) || defined(__clang__)
    #define ALIGNED_PREFIX(n)
    #define ALIGNED_SUFFIX(n) __attribute__((aligned(n)))
    #define ALIGNED_VAR(n, type) type __attribute__((aligned(n)))
#else
    #define ALIGNED_PREFIX(n)
    #define ALIGNED_SUFFIX(n)
#endif // #if defined(_MSC_VER)

#if __GNUC__ >= 11
#define CASE_FALLTHROUGH  [[fallthrough]]
#elif defined(_MSC_VER)
#define CASE_FALLTHROUGH
#else
#define CASE_FALLTHROUGH  __attribute__ ((fallthrough))
#endif // #if __GNUC__ >= 11

#if !defined(__cplusplus)

#if defined(_MSC_VER)

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#else // Linux

#define max(a,b) \
    ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })

#define min(a,b) \
    ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a < _b ? _a : _b; })

#endif // #if defined(_MSC_VER)

#endif // #if !defined(__cplusplus)

#define snprintf_nowarn(...) (snprintf(__VA_ARGS__) < 0 ? abort() : (void)0)

#define FAILED(x) (x != 0)
#define SUCCEEDED(x) (x == 0)

#define SWAP(T, a, b) do { T tmp = a; a = b; b = tmp; } while (0)

#define RADCONV   57.2957795130823229
#define DEGREES_TO_RADIANS(angle)  (((MMFLOAT) angle) / RADCONV)

#define CHAR_IS_SIGNED ((char) -1 < 0)

/**
 * Dump contents of memory to stdout.
 *
 * @param  p          Pointer to start of memory.
 * @param  num_bytes  Number of bytes to dump.
 *                    If <= 0 then stop when encounter eight consecutive 0xFF.
 * @param  indent     Number of spaces of indent for each line of the dump.
 * @param  cols       Number of 8-byte columns.
 */
void utility_dump_memory(const char *p, int num_bytes, size_t indent, size_t cols);

/** perror() with formatted string support. */
void utility_perror_ext(const char *format, ...);

#endif // #if !defined(MMBASIC_UTILITY_H)
