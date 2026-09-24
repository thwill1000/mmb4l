/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_dim.c

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

#include <string.h>

#include "../common/cstring.h"
#include "../common/mmb4l.h"
#include "../common/utility.h"
#include "../core/tokentbl.h"

static const char *SetValue(const char *p, int t, void *v) {
    MMFLOAT f = 0.0;
    MMINTEGER i64 = 0;
    char *s = NULL;
    char TempCurrentSubFunName[MAXVARLEN + 2];                      // requires extra byte to store optional type suffix.

    strcpy(TempCurrentSubFunName, CurrentSubFunName);               // save the current sub/fun name
    if(t & T_STR) {
        p = evaluate(p, &f, &i64, &s, &t, true);
        Mstrcpy(v, s);
    }
    else if(t & T_NBR) {
        p = evaluate(p, &f, &i64, &s, &t, false);
        if(t & T_NBR)
            (*(MMFLOAT *)v) = f;
        else
            (*(MMFLOAT *)v) = (MMFLOAT)i64;
    } else {
        p = evaluate(p, &f, &i64, &s, &t, false);
        if(t & T_INT)
            (*(MMINTEGER *)v) = i64;
        else
            (*(MMINTEGER *)v) = FloatToInt64(f);
    }
    strcpy(CurrentSubFunName, TempCurrentSubFunName);               // restore the current sub/fun name
    return p;
}

// define a variable
// DIM [AS INTEGER|FLOAT|STRING] var[(d1 [,d2,...]] [AS INTEGER|FLOAT|STRING] [, ..., ...]
// LOCAL also uses this function the routines only differ in that LOCAL can only be used in a sub/fun
void cmd_dim(void) {
    int i, j, k, type, typeSave, ImpliedType = 0, VIndexSave, StaticVar = false;
    char chSave, *chPosit;
    char VarName[STRINGSIZE];
    void *v, *tv;

    if (tokentbl_peek(cmdline) == tokenAS) cmdline += tokensize(tokenAS); // this means that we can use DIM AS INTEGER a, b, etc
    const char *pconst = CheckIfTypeSpecified(cmdline, &type, true);  // check for DIM FLOAT A, B, ...
    ImpliedType = type;
    {
        getargs(&pconst, (MAX_ARG_COUNT * 2) - 1, DELIM_COMMA);
        if((argc & 0x01) == 0) ERROR_SYNTAX;

        // 'p' will be pointing into the items of argv[] which we know are not
        // constants so we can cast away const-ness as necessary to remove
        // warnings.
        char *p;
        for(i = 0; i < argc; i += 2) {
            p = (char *) skipvar(argv[i], false);                   // point to after the variable
            FunctionToken funtok = INVALID_TOKEN;
            while (*p != 0 && *p != '\'') {                         // skip over a LENGTH keyword if there and see if we can find "AS"
                funtok = tokentbl_peek(p);
                if (funtok == tokenAS || funtok == tokenEQUAL) break;
                p += tokensize(funtok);
            }
            chSave = *p; chPosit = p; *p = 0;                       // save the char then terminate the string so that LENGTH is evaluated correctly
            if (funtok == tokenAS) {                                // are we using Microsoft syntax (eg, AS INTEGER)?
                if(ImpliedType & T_IMPLIED) error_throw_legacy("Type specified twice");
                p += tokensize(tokenAS);                            // step over the AS token
                p = (char *) CheckIfTypeSpecified(p, &type, true);  // and get the type
                if(!(type & T_IMPLIED)) error_throw_legacy("Variable type");
            }

            if(cmdtoken == cmdLOCAL) {
                if(LocalIndex == 0) error_throw_legacy("Invalid here");
                type |= V_LOCAL;                                    // local if defined in a sub/fun
            }

            if (cmdtoken == cmdSTATIC) {
                if (LocalIndex == 0) error_throw_legacy("Invalid here");
                // Create a unique global name by prefixing variable name with sub/fun name.
                char function_name[MAXVARLEN + 2];
                ON_FAILURE_ERROR(get_current_function_name(function_name, sizeof(function_name)));
                if (FAILED(cstring_cpy(VarName, function_name, sizeof(function_name)))) ERROR_LINE_LENGTH;
                for (k = 1; k <= MAXVARLEN; k++) {
                    if (!isnamechar(VarName[k])) {
                        VarName[k] = 0;                             // terminate the string on a non valid char
                        break;
                    }
                }
                if (FAILED(cstring_cat(VarName, argv[i], sizeof(VarName)))) ERROR_LINE_LENGTH;
                StaticVar = true;
            } else {
                if (FAILED(cstring_cpy(VarName, argv[i], sizeof(VarName)))) ERROR_LINE_LENGTH;
            }

            v = findvar(VarName, type | V_NOFIND_NULL);             // check if the variable exists
            typeSave = type;
            VIndexSave = VarIndex;
            if(v == NULL) {                                         // if not found
                v = findvar(VarName, type | V_FIND | V_DIM_VAR);    // create the variable
                type = TypeMask(vartbl[VarIndex].type);
                VIndexSave = VarIndex;
                *chPosit = chSave;                                  // restore the char previously removed
                if(vartbl[VarIndex].dims[0] == -1) error_throw_legacy("Array dimensions");
                if(vartbl[VarIndex].dims[0] > 0) {
                    DimUsed = true;                                 // prevent OPTION BASE from being used
                    v = vartbl[VarIndex].val.s;
                }
                FunctionToken funtok = INVALID_TOKEN;
                while (*p != 0 && *p != '\'' && funtok != tokenEQUAL) {  // search through the line looking for the equals sign
                    funtok = tokentbl_read((const char **) &p);
                }
                if (funtok == tokenEQUAL) {
                    skipspace(p);
                    if(vartbl[VarIndex].dims[0] > 0 && *p == '(') {
                        // calculate the overall size of the array
                        for(j = 1, k = 0; k < MAXDIM && vartbl[VIndexSave].dims[k]; k++) {
                            j *= (vartbl[VIndexSave].dims[k] + 1 - mmb_options.base);
                        }
                        do {
                            p++;                                    // step over the opening bracket or terminating comma
                            p = (char *) SetValue(p, type, v);
                            if(type & T_STR) v = (char *)v + vartbl[VIndexSave].size + 1;
                            if(type & T_NBR) v = (char *)v + sizeof(MMFLOAT);
                            if(type & T_INT) v = (char *)v + sizeof(MMINTEGER);
                            skipspace(p); j--;
                        } while(j > 0 && *p == ',');
                        if(*p != ')') error_throw_legacy("Number of initialising values");
                        if(j != 0) error_throw_legacy("Number of initialising values");
                    } else
                        SetValue(p, type, v);
                }
                type = ImpliedType;
            } else {
                if(!StaticVar) error_throw_legacy("$ already declared", VarName);
            }

            // if it is a STATIC var create a local var pointing to the global var
            if(StaticVar) {
                tv = findvar(argv[i], typeSave | V_LOCAL | V_NOFIND_NULL);                        // check if the local variable exists
                if(tv != NULL) error_throw_legacy("$ already declared", argv[i]);
                tv = findvar(argv[i], typeSave | V_LOCAL | V_FIND | V_DIM_VAR);                   // create the variable
                if(vartbl[VIndexSave].dims[0] > 0 || (vartbl[VIndexSave].type & T_STR)) {
                    FreeMemory(tv);                                                               // we don't need the memory allocated to the local
                    vartbl[VarIndex].val.s = vartbl[VIndexSave].val.s;                            // point to the memory of the global variable
                } else
                    vartbl[VarIndex].val.ia = &(vartbl[VIndexSave].val.i);                        // point to the data of the variable
                vartbl[VarIndex].type = vartbl[VIndexSave].type | T_PTR;                          // set the type to a pointer
                vartbl[VarIndex].size = vartbl[VIndexSave].size;                                  // just in case it is a string copy the size
                for(j = 0; j < MAXDIM; j++) vartbl[VarIndex].dims[j] = vartbl[VIndexSave].dims[j];// just in case it is an array copy the dimensions
            }
        }
    }
}
