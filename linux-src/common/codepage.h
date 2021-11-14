#if !defined(MMB4L_CODEPAGE_H)
#define MMB4L_CODEPAGE_H

extern const char * const CODEPAGE_NAMES[];

extern char *codepage_current;

int codepage_set(const char *page_name);
int codepage_to_string(const char *codepage, char *out);

#endif
