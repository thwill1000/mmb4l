/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_longstring.c

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"

static void longstring_append(const char *tp) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    char *p = NULL;
    char *q = NULL;
    int i, j, nbr;
    getargs(&tp, 3, ",");
    if (argc != 3) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *)ptr1;
        q = (char *)&dest[1];
        q += dest[0];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    j = (vartbl[VarIndex].dims[0] - mmb_options.base);
    p = getstring(argv[2]);
    nbr = i = *p++;
    if (j * 8 < dest[0] + i) ERROR_INTEGER_ARRAY_TOO_SMALL;
    while (i--) *q++ = *p++;
    dest[0] += nbr;
}

static void longstring_clear(const char *tp) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    getargs(&tp, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *)ptr1;
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    dest[0] = 0;
}

static void longstring_copy(const char *tp) {
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    int64_t *dest = NULL, *src = NULL;
    char *p = NULL;
    char *q = NULL;
    int i = 0, j;
    getargs(&tp, 3, ",");
    if (argc != 3) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *)ptr1;
        dest[0] = 0;
        q = (char *)&dest[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    j = (vartbl[VarIndex].dims[0] - mmb_options.base);
    ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(2);
        src = (int64_t *)ptr2;
        p = (char *)&src[1];
        i = src[0];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(2);
    if (j * 8 < i) ERROR_DST_ARRAY_TOO_SMALL;
    while (i--) *q++ = *p++;
    dest[0] = src[0];
}

static void longstring_concat(const char *tp) {
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    int64_t *dest = NULL, *src = NULL;
    char *p = NULL;
    char *q = NULL;
    int i = 0, j, d = 0, s = 0;
    getargs(&tp, 3, ",");
    if (argc != 3) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *)ptr1;
        d = dest[0];
        q = (char *)&dest[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    j = (vartbl[VarIndex].dims[0] - mmb_options.base);
    ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(2);
        src = (int64_t *)ptr2;
        p = (char *)&src[1];
        i = s = src[0];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(2);
    if (j * 8 < (d + s)) ERROR_DST_ARRAY_TOO_SMALL;
    q += d;
    while (i--) *q++ = *p++;
    dest[0] += src[0];
}

static void longstring_lcase(const char *tp) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    char *q = NULL;
    int i;
    getargs(&tp, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *)ptr1;
        q = (char *)&dest[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    i = dest[0];
    while (i--) {
        if (*q >= 'A' && *q <= 'Z') *q += 0x20;
        q++;
    }
}

static void longstring_left(const char *tp) {
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    int64_t *dest = NULL, *src = NULL;
    char *p = NULL;
    char *q = NULL;
    int i, j, nbr;
    getargs(&tp, 5, ",");
    if (argc != 5) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *)ptr1;
        q = (char *)&dest[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    j = (vartbl[VarIndex].dims[0] - mmb_options.base);
    ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(2);
        src = (int64_t *)ptr2;
        p = (char *)&src[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(2);
    nbr = i = getinteger(argv[4]);
    if (nbr > src[0]) nbr = i = src[0];
    if (j * 8 < i) ERROR_DST_ARRAY_TOO_SMALL;
    while (i--) *q++ = *p++;
    dest[0] = nbr;
}

static void longstring_load(const char *tp) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    char *p;
    char *q = NULL;
    int i, j;
    getargs(&tp, 5, ",");
    if (argc != 5) ERROR_ARGUMENT_COUNT;
    int64_t nbr = getinteger(argv[2]);
    i = nbr;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *)ptr1;
        dest[0] = 0;
        q = (char *)&dest[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    j = (vartbl[VarIndex].dims[0] - mmb_options.base);
    p = getstring(argv[4]);
    if (nbr > *p) nbr = *p;
    p++;
    if (j * 8 < dest[0] + nbr) ERROR_INTEGER_ARRAY_TOO_SMALL;
    while (i--) *q++ = *p++;
    dest[0] += nbr;
    return;
}

static void longstring_mid(const char *tp) {
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    int64_t *dest = NULL, *src = NULL;
    char *p = NULL;
    char *q = NULL;
    int i, j, nbr, start;
    getargs(&tp, 7, ",");
    if (argc != 7) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *)ptr1;
        q = (char *)&dest[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    j = (vartbl[VarIndex].dims[0] - mmb_options.base);
    ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(2);
        src = (int64_t *)ptr2;
        p = (char *)&src[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(2);
    start = getint(argv[4], 1, src[0]);
    nbr = getinteger(argv[6]);
    p += start - 1;
    if (nbr + start > src[0]) {
        nbr = src[0] - start + 1;
    }
    i = nbr;
    if (j * 8 < nbr) ERROR_DST_ARRAY_TOO_SMALL;
    while (i--) *q++ = *p++;
    dest[0] = nbr;
}

static void longstring_print(const char *tp) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    char *q = NULL;
    int i, j, fnbr;
    getargs(&tp, 5, ",;");
    if (argc < 1 || argc > 4) ERROR_ARGUMENT_COUNT;

    if (argc > 0 && *argv[0] == '#') {
        // First argument is a file number.
        fnbr = parse_file_number(argv[0], true);
        if (fnbr == -1) {
            error_throw(kFileInvalidFileNumber);
            return;
        }
        // Set the next argument to be looked at.
        i = 1;
        if (argc >= 2 && *argv[1] == ',') i = 2;
    } else {
        // Use standard output.
        fnbr = 0;
        i = 0;
    }

    if (argc >= 1) {
        ptr1 = findvar(argv[i], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                if (i == 0)
                    ERROR_ARG_NOT_INTEGER_ARRAY(1);
                else
                    ERROR_ARG_NOT_INTEGER_ARRAY(2);
            }
            dest = (int64_t *)ptr1;
            q = (char *)&dest[1];
        } else {
            if (i == 0)
                ERROR_ARG_NOT_INTEGER_ARRAY(1);
            else
                ERROR_ARG_NOT_INTEGER_ARRAY(2);
        }
        j = dest[0];
        while (j--) {
            file_putc(fnbr, *q++);
        }
        i++;
    }
    if (argc > i) {
        if (*argv[i] == ';') return;
    }
    file_write(fnbr, "\r\n", 2);
}

static void longstring_replace(const char *tp) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    char *p = NULL;
    char *q = NULL;
    int i, nbr;
    getargs(&tp, 5, ",");
    if (argc != 5) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *)ptr1;
        q = (char *)&dest[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    p = getstring(argv[2]);
    nbr = getint(argv[4], 1, dest[0] - *p + 1);
    q += nbr - 1;
    i = *p++;
    while (i--) *q++ = *p++;
}

static void longstring_resize(const char *tp) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    int j = 0;
    getargs(&tp, 3, ",");
    if (argc != 3) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        j = (vartbl[VarIndex].dims[0] - mmb_options.base) * 8;
        dest = (int64_t *)ptr1;
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);

    // dest[0] = getint(argv[2], mmb_options.base, j - mmb_options.base) + 1;
    dest[0] = getint(argv[2], 0, j);
}

static void longstring_right(const char *tp) {
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    int64_t *dest = NULL, *src = NULL;
    char *p = NULL;
    char *q = NULL;
    int i, j, nbr;
    getargs(&tp, 5, ",");
    if (argc != 5) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *)ptr1;
        q = (char *)&dest[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    j = (vartbl[VarIndex].dims[0] - mmb_options.base);
    ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(2);
        src = (int64_t *)ptr2;
        p = (char *)&src[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(2);
    nbr = i = getinteger(argv[4]);
    if (nbr > src[0]) {
        nbr = i = src[0];
    } else
        p += (src[0] - nbr);
    if (j * 8 < i) ERROR_DST_ARRAY_TOO_SMALL;
    while (i--) *q++ = *p++;
    dest[0] = nbr;
}

void longstring_setbyte(const char *tp) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    int p = 0;
    uint8_t *q = NULL;
    int nbr;
    int j = 0;
    getargs(&tp, 5, ",");
    if (argc != 5) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        j = (vartbl[VarIndex].dims[0] - mmb_options.base) * 8 - 1;
        dest = (int64_t *)ptr1;
        q = (uint8_t *)&dest[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    p = getint(argv[2], mmb_options.base, j - mmb_options.base);
    nbr = getint(argv[4], 0, 255);
    q[p - mmb_options.base] = nbr;
    return;
}

void longstring_trim(const char *tp) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    uint32_t trim;
    char *p, *q = NULL;
    int i;
    getargs(&tp, 3, ",");
    if (argc != 3) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *)ptr1;
        q = (char *)&dest[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    trim = getint(argv[2], 1, dest[0] - 1);
    i = dest[0] - trim;
    p = q + trim;
    while (i--) *q++ = *p++;
    dest[0] -= trim;
}

void longstring_ucase(const char *tp) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    char *q = NULL;
    int i;
    getargs(&tp, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *)ptr1;
        q = (char *)&dest[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    i = dest[0];
    while (i--) {
        if (*q >= 'a' && *q <= 'z') *q -= 0x20;
        q++;
    }
}

void cmd_longstring(void) {
    const char *p;
    if ((p = checkstring(cmdline, "APPEND"))) {
        longstring_append(p);
    } else if ((p = checkstring(cmdline, "CLEAR"))) {
        longstring_clear(p);
    } else if ((p = checkstring(cmdline, "COPY"))) {
        longstring_copy(p);
    } else if ((p = checkstring(cmdline, "CONCAT"))) {
        longstring_concat(p);
    } else if ((p = checkstring(cmdline, "LCASE"))) {
        longstring_lcase(p);
    } else if ((p = checkstring(cmdline, "LEFT"))) {
        longstring_left(p);
    } else if ((p = checkstring(cmdline, "LOAD"))) {
        longstring_load(p);
    } else if ((p = checkstring(cmdline, "MID"))) {
        longstring_mid(p);
    } else if ((p = checkstring(cmdline, "PRINT"))) {
        longstring_print(p);
    } else if ((p = checkstring(cmdline, "REPLACE"))) {
        longstring_replace(p);
    } else if ((p = checkstring(cmdline, "RESIZE"))) {
        longstring_resize(p);
    } else if ((p = checkstring(cmdline, "RIGHT"))) {
        longstring_right(p);
    } else if ((p = checkstring(cmdline, "SETBYTE"))) {
        longstring_setbyte(p);
    } else if ((p = checkstring(cmdline, "TRIM"))) {
        longstring_trim(p);
    } else if ((p = checkstring(cmdline, "UCASE"))) {
        longstring_ucase(p);
    } else {
        ERROR_SYNTAX;
    }
}
