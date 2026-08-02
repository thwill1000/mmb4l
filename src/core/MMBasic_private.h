/*-*****************************************************************************

MMBasic for Linux (MMB4L)

MMBasic_private.h

Copyright 2011-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#if !defined(MMBASIC_PRIVATE_H)
#define MMBASIC_PRIVATE_H

#include "MMBasic.h"

// Storage for a single resolved argument value, used by DefinedSubFunArgs
// below. Which member is valid is determined by the corresponding entry in
// DefinedSubFunArgs.type[].
union DefinedSubFunArgValue {
    MMFLOAT f;      // the value if it is a float
    MMINTEGER i;    // the value if it is an integer
    MMFLOAT *fa;    // pointer to the allocated memory if it is an array of floats
    MMINTEGER *ia;  // pointer to the allocated memory if it is an array of integers
    char *s;        // pointer to the allocated memory if it is a string
};

// How a single parameter in a sub/fun definition was declared to be passed.
enum ParamConvention { kParamConventionDefault, kParamConventionByVal, kParamConventionByRef };

// Working state shared by split_caller_and_definition_args(), resolve_caller_argument()
// and bind_parameter_to_local() while binding a call's arguments to a sub/fun's
// parameters. Allocated with GetTempMemory() by DefinedSubFun() because it is
// too large to comfortably keep on the stack.
struct DefinedSubFunArgs {
    union DefinedSubFunArgValue val[MAX_ARG_COUNT];
    int type[MAX_ARG_COUNT];
    int varIndex[MAX_ARG_COUNT];

    // Arguments provided by caller.
    char buf1[STRINGSIZE];
    char *v1[MAX_ARG_COUNT];
    int c1;

    // Parameters in sub/fun definition.
    char buf2[STRINGSIZE];
    char *v2[MAX_ARG_COUNT];
    enum ParamConvention convention[MAX_ARG_COUNT];
    int c2;
};

/**
 * Parses the sub/fun name (and, for subs, its optional type suffix) from the
 * start of its definition, copying it into fun_name_out.
 *
 * @param  definition_start  Pointer to the first character of the name in
 *                           the definition (already past the SUB/FUN keyword).
 * @param  isfun             True if parsing a FUNCTION definition.
 * @param  fun_name_out      Buffer to receive the NUL-terminated name; must
 *                           be at least MAXVARLEN + 2 bytes.
 * @return                   Pointer just past the name (and optional type
 *                           suffix), i.e. the start of the argument list.
 */
const char *parse_definition_name(const char *definition_start, bool isfun, char *fun_name_out);

/**
 * Finds the end of the caller's identifier (name plus optional type suffix)
 * and checks that its type suffix, if any, matches the one on the
 * definition.
 *
 * @param  cmd                 Pointer to the caller's identifier (as passed
 *                             into DefinedSubFun()).
 * @param  isfun               True if calling a FUNCTION.
 * @param  definition_name_end  Pointer just past the definition's name (and
 *                             optional type suffix), as returned by
 *                             parse_definition_name().
 * @return                    Pointer just past the caller's identifier, i.e.
 *                            the start of the caller's argument list.
 */
const char *validate_caller_type_suffix(const char *cmd, bool isfun,
                                        const char *definition_name_end);

/**
 * For a FUNCTION definition, determines the function's return type,
 * honouring an explicit "AS <type>" clause if present. No-op for a SUB.
 *
 * @param  definition_start  Pointer to the start of the name in the
 *                           definition (i.e. before parse_definition_name()
 *                           advanced past it) - skipvar() re-parses the name
 *                           and any bracketed argument list from here.
 * @param  isfun             True if parsing a FUNCTION definition.
 * @return                   T_NOTYPE for a sub; otherwise the function's
 *                           return type combined with V_FIND | V_DIM_VAR |
 *                           V_LOCAL | V_EMPTY_OK, ready to pass to findvar().
 */
int get_function_return_type(const char *definition_start, bool isfun);

/**
 * Handles the case where the routine being invoked is actually a CFUNCTION
 * or CSUB (implemented in machine code rather than MMBasic). If so, it is
 * called directly and, for a CFUNCTION, its result is stored in
 * \p fa, \p i64a or \p sa and its type in \p typ.
 *
 * @return  true if sub_line_ptr identified a CFUNCTION/CSUB and it was
 *          called (the caller should return immediately afterwards); false
 *          if this is an ordinary user-defined sub/fun and normal
 *          processing should continue.
 */
bool try_call_cfunction_or_csub(const char *sub_line_ptr, const char *caller_args,
                                const char *definition_args, MMFLOAT *fa, MMINTEGER *i64a,
                                char **sa, int *typ);

/**
 * Splits both the caller's argument list and the sub/fun definition's
 * parameter list into individual elements (via makeargs()), and determines
 * each definition parameter's BYREF/BYVAL calling convention, stripping any
 * such prefix from args->v2[] so it points directly at the parameter name.
 *
 * @param  args             Populated with c1/v1/buf1 (caller arguments),
 *                          c2/v2/buf2 (definition parameters), and
 *                          convention[] (one entry per parameter).
 * @param  caller_args      Caller's argument list; advanced past it by
 *                          makeargs().
 * @param  definition_args  Definition's parameter list; advanced past it by
 *                          makeargs().
 * @param  sub_line_ptr     Definition's start, for error reporting.
 * @param  caller_line_ptr  Caller's start, for error reporting.
 */
void split_caller_and_definition_args(struct DefinedSubFunArgs *args, const char **caller_args,
                                      const char **definition_args, const char *sub_line_ptr,
                                      const char *caller_line_ptr);

/**
 * Resolves a single caller-supplied argument (index i in the definition's
 * parameter list) to either a pointer to an existing variable or an
 * evaluated expression value, honouring the parameter's BYREF/BYVAL calling
 * convention. Populates args->val[i] / args->type[i] / args->varIndex[i].
 *
 * No-op if the caller did not supply a value for this parameter, leaving
 * args->type[i] as the zero it was initialised to by GetTempMemory().
 *
 * @param  args  Caller/definition arguments, as populated by
 *              split_caller_and_definition_args().
 * @param  i     Index (even) of the parameter within args->v2[].
 */
void resolve_caller_argument(struct DefinedSubFunArgs *args, int i);

/**
 * Declares (or resolves) the local variable for a single sub/fun parameter
 * (index i in the definition), and binds the caller-resolved argument
 * (args->val[i] / args->type[i], as populated by resolve_caller_argument())
 * into it - either as a pointer (for variable arguments) or as a value
 * copy, performing float/integer coercion as needed.
 *
 * @param  args             Caller/definition arguments, as populated by
 *                          split_caller_and_definition_args() and
 *                          resolve_caller_argument().
 * @param  i                Index (even) of the parameter within args->v2[].
 * @param  sub_line_ptr     Definition's start, for error reporting.
 * @param  caller_line_ptr  Caller's start, for error reporting.
 */
void bind_parameter_to_local(struct DefinedSubFunArgs *args, int i, const char *sub_line_ptr,
                             const char *caller_line_ptr);

/**
 * Executes a defined FUNCTION's body and returns its result to the caller.
 * Declares the local variable holding the function's return value, runs the
 * function body via a nested ExecuteProgram(), then copies the resulting
 * value into \p fa, \p i64a, or \p sa according to fun_type and records the type
 * in \p typ.
 *
 * @param  definition_args  Pointer to the (already consumed) argument list
 *                          in the definition; skipelement() advances past it
 *                          to the start of the body.
 * @param  fun_name         Name of the function, used to declare its
 *                          return-value local variable.
 * @param  fun_type         Function's return type combined with
 *                          V_FIND | V_DIM_VAR | V_LOCAL | V_EMPTY_OK, as
 *                          returned by get_function_return_type().
 */
void invoke_function_body(const char *definition_args, char *fun_name, int fun_type, MMFLOAT *fa,
                          MMINTEGER *i64a, char **sa, int *typ);

#endif  // #if !defined(MMBASIC_PRIVATE_H)
