#if !defined(MMB4L_CODEPAGE_H)
#define MMB4L_CODEPAGE_H

#include "options.h"

extern const char * const CODEPAGE_NAMES[];

/** @return  0 on success, -1 on error. */
int codepage_set(Options *options, const char *page_name);

/** @return  0 on success, -1 on error. */
int codepage_to_string(const char *codepage, char *out);

#endif
