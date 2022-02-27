#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "cstring.h"

int cstring_cat(char *dst, const char* src, size_t dst_sz) {
    assert(dst_sz > strlen(dst));
    const char *limit = dst + dst_sz - 1;
    char *p = dst + strlen(dst);
    char *sp = (char *) src;
    while ((p < limit) && *sp) *p++ = *sp++;
    *p = '\0';
    return *sp ? -1 : 0;
}

// TODO: be smart and safe about not overrunning target.
void cstring_replace(char *target, const char *needle, const char *replacement) {
    char buffer[288] = {0};
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t needle_len = strlen(needle);
    size_t repl_len = strlen(replacement);

    while (1) {
        const char *p = strstr(tmp, needle);

        // walked past last occurrence of needle; copy remaining part
        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }

        // copy part before needle
        memcpy(insert_point, tmp, p - tmp);
        insert_point += p - tmp;

        // copy replacement string
        memcpy(insert_point, replacement, repl_len);
        insert_point += repl_len;

        // adjust pointers, move on
        tmp = p + needle_len;
    }

    // write altered string back to target
    strcpy(target, buffer);
}

char *cstring_toupper(char *s) {
    char *p = s;
    while (*p != '\0') {
        *p = toupper(*p);
        p++;
    }
    return s;
}

void cstring_unquote(char *str) {
    if (str[0] == '\"' && str[strlen(str) - 1] == '\"') {
        int len = strlen(str);
        for (int i = 0; i < len - 2; ++i) {
            str[i] = str[i + 1];
        }
        str[len - 2] = '\0';
    }
}
