#include "../common/mmb4l.h"

// return true if the char 'c' is contained in the string 'srch$'
// used only by scan_for_delimiter()  below
// Note: this operates on MMBasic strings
static int MInStr(char *srch, char c) {
    int i;
    for (i = 1; i <= *(unsigned char *)srch; i++)
        if (c == srch[i]) return true;
    return false;
}

// scan through string p and return p if it points to any char in delims
// this will skip any quoted text (quote delimiters in quotes)
// used only by fun_field() below
// Note: this operates on MMBasic strings
static int scan_for_delimiter(int start, char *p, char *delims, char *quotes) {
    int i;
    char qidx;
    for (i = start; i <= *(unsigned char *)p && !MInStr(delims, p[i]); i++) {
        if (MInStr(quotes, p[i])) {  // if we have a quote
            qidx = p[i];
            i++;  // step over the opening quote
            while (i < *(unsigned char *)p && p[i] != qidx)
                i++;  // skip the quoted text
        }
    }
    return i;
}

// syntax:  str$ = FIELD$(string1, nbr, string2, string3)
//          find field nbr in string1 using the delimiters in string2 to separate the fields
//          if string3 is present any chars quoted by chars in string3 will not be searched for delimiters
// Note: this operates on MMBasic strings
void fun_field(void) {
    char *p, *delims = "\1,", *quotes = "\0";
    int fnbr, i, j, k;
    getargs(&ep, 7, ",");
    if (!(argc == 3 || argc == 5 || argc == 7)) error("Syntax");
    p = getstring(argv[0]);                // the string containing the fields
    fnbr = getint(argv[2], 1, MAXSTRLEN);  // field nbr to return
    if (argc > 3 && *argv[4])
        delims = getstring(argv[4]);             // delimiters for fields
    if (argc == 7) quotes = getstring(argv[6]);  // delimiters for quoted text
    sret = GetTempStrMemory();  // this will last for the life of the command
    targ = T_STR;
    i = 1;
    while (--fnbr > 0) {
        i = scan_for_delimiter(i, p, delims, quotes);
        if (i > *p) return;
        i++;  // step over the delimiter
    }
    while (p[i] == ' ') i++;                       // trim leading spaces
    j = scan_for_delimiter(i, p, delims, quotes);  // find the end of the field
    *sret = k = j - i;
    for (j = 1; j <= k; j++, i++) sret[j] = p[i];  // copy to the return string
    for (k = *sret; k > 0 && sret[k] == ' '; k--)
        ;  // trim trailing spaces
    *sret = k;
}
