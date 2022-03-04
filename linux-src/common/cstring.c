#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "cstring.h"
#include "utility.h"

int cstring_cat(char *dst, const char* src, size_t dst_sz) {
    size_t dst_len = strlen(dst);
    size_t src_len = strlen(src);
    size_t n = min(dst_sz - dst_len - 1, src_len);
    if (n > 0) {
        memmove(dst + dst_len, src, n);
        dst[dst_len + n] = '\0';
    }
    return dst_len + src_len < dst_sz ? 0 : -1;
}

int cstring_enquote(char *s, size_t n) {
    if (!s) return -1;
    size_t len = strlen(s);
    if (len + 2 >= n) return -1;
    memmove(s + 1, s, len);
    s[0] = '"';
    s[len + 1] = '"';
    s[len + 2] = '\0';
    return 0;
}

bool cstring_isquoted(const char *s) {
    size_t len = strlen(s);
    return len > 1 && s[0] == '"' && s[len - 1] == '"';
}

// TODO: be smart and safe about not overrunning target.
// TODO: unit test this better.
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

char *cstring_tolower(char *s) {
    char *p = s;
    while (*p) {
        *p = tolower(*p);
        p++;
    }
    return s;
}

char *cstring_toupper(char *s) {
    char *p = s;
    while (*p) {
        *p = toupper(*p);
        p++;
    }
    return s;
}

char *cstring_trim(char *s) {
    if (!s) return s;
    char *start;
    for (start = s; isspace(*start); start++);
    char *end;
    for (end = s + strlen(s) - 1; end >= start && isspace(*end); end--);
    if (end >= start) memmove(s, start, end - start + 1);
    s[end - start + 1] = '\0';
    return s;
}

char *cstring_unquote(char *s) {
    int len = strlen(s);
    if (len > 1 && s[0] == '\"' && s[len - 1] == '\"') {
        memmove(s, s + 1, len - 2);
        s[len - 2] = '\0';
    }
    return s;
}
