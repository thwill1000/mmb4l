/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_json.c

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

#include "../common/mmb4l.h"
#include "../common/cJSON.h"

#include <stdlib.h>
#include <string.h>

#define ERROR_NOT_AN_ITEM  error_throw_ex(kError, "Not an item")

#define REPORT_NULL_FLAG  0x01
#define REPORT_MISSING_FLAG  0x02

static void fun_json_internal(void *varptr, char *key, int64_t flags) {

    int64_t *dest = (int64_t *) varptr;
    char *json_string = (char *) &dest[1];
    cJSON *parse = cJSON_Parse(json_string);
    if (!parse) ERROR_INVALID("JSON data");

    const cJSON *root = parse;
    int len = strlen(key);
    char field[32] = { 0 };
    char num[6] = { 0 };
    int i = 0, j = 0, k = 0, mode = 0;
    while (i < len) {
        if (key[i] == '[') {  // start of index
            mode = 1;
            field[j] = 0;
            root = cJSON_GetObjectItemCaseSensitive(root, field);
            memset(field, 0, 32);
            j = 0;
        }
        if (key[i] == ']') {
            num[k] = 0;
            int index = atoi(num);
            root = cJSON_GetArrayItem(root, index);
            memset(num, 0, 6);
            k = 0;
        }
        if (key[i] == '.') {  // new field separator
            if (mode == 0) {
                field[j] = 0;
                root = cJSON_GetObjectItemCaseSensitive(root, field);
                memset(field, 0, 32);
                j = 0;
            } else {  // step past the dot after a close bracket
                mode = 0;
            }
        } else {
            if (mode == 0)
                field[j++] = key[i];
            else if (key[i] != '[')
                num[k++] = key[i];
        }
        i++;
    }

    root = cJSON_GetObjectItem(root, field);
    targ = T_STR;
    sret = GetTempStrMemory();

    if (cJSON_IsObject(root) || cJSON_IsInvalid(root)) {
        cJSON_Delete(parse);
        ERROR_NOT_AN_ITEM;
    } else if (cJSON_IsNull(root)) {
        strcpy(sret, flags & REPORT_NULL_FLAG ? "<null>" : "");
        cJSON_Delete(parse);
    } else if (cJSON_IsNumber(root)) {
        MMFLOAT tempd = root->valuedouble;
        if ((MMFLOAT) ((int64_t) tempd) == tempd) {
            IntToStr(sret, (int64_t) tempd, 10);
        } else {
            FloatToStr(sret, tempd, 0, STR_AUTO_PRECISION, ' ');
        }
        cJSON_Delete(parse);
    } else if (cJSON_IsBool(root)) {
        strcpy(sret, root->valueint ? "true" : "false");
        cJSON_Delete(parse);
    } else if (cJSON_IsString(root)) {
        strcpy(sret, root->valuestring);
        cJSON_Delete(parse);
    } else {
        // Key not found.
        strcpy(sret, flags & REPORT_MISSING_FLAG ? "<missing>" : "");
        cJSON_Delete(parse);
    }

    targ = T_STR;
    CtoM(sret);
}

void fun_json(void) {
    getargs(&ep, 5, ",");

    if (argc != 3 && argc != 5) ERROR_SYNTAX;

    // First argument should be a LONGSTRING, aka. a 1D integer array.
    void *varptr = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (!(vartbl[VarIndex].type & T_INT)) ERROR_ARG_NOT_INTEGER_ARRAY(1);
    if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
    if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);

    // Second argument is the key to lookup in the JSON.
    char *key = getCstring(argv[2]);

    // Third (optional) argument is additional flags.
    int64_t flags = 0x0;
    if (argc == 5) {
        flags = getint(argv[4], 0, 3);
    }

    fun_json_internal(varptr, key, flags);
}
