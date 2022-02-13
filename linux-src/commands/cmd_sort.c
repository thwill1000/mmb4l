#include <ctype.h>

#include "../common/mmb4l.h"

static void integersort(int64_t *iarray, int n, int64_t *index, int flags,
                 int startpoint) {
    int i, j = n, s = 1;
    int64_t t;
    if ((flags & 1) == 0) {
        while (s) {
            s = 0;
            for (i = 1; i < j; i++) {
                if (iarray[i] < iarray[i - 1]) {
                    t = iarray[i];
                    iarray[i] = iarray[i - 1];
                    iarray[i - 1] = t;
                    s = 1;
                    if (index != NULL) {
                        t = index[i - 1 + startpoint];
                        index[i - 1 + startpoint] = index[i + startpoint];
                        index[i + startpoint] = t;
                    }
                }
            }
            j--;
        }
    } else {
        while (s) {
            s = 0;
            for (i = 1; i < j; i++) {
                if (iarray[i] > iarray[i - 1]) {
                    t = iarray[i];
                    iarray[i] = iarray[i - 1];
                    iarray[i - 1] = t;
                    s = 1;
                    if (index != NULL) {
                        t = index[i - 1 + startpoint];
                        index[i - 1 + startpoint] = index[i + startpoint];
                        index[i + startpoint] = t;
                    }
                }
            }
            j--;
        }
    }
}

static void floatsort(MMFLOAT *farray, int n, int64_t *index, int flags,
               int startpoint) {
    int i, j = n, s = 1;
    int64_t t;
    MMFLOAT f;
    if ((flags & 1) == 0) {
        while (s) {
            s = 0;
            for (i = 1; i < j; i++) {
                if (farray[i] < farray[i - 1]) {
                    f = farray[i];
                    farray[i] = farray[i - 1];
                    farray[i - 1] = f;
                    s = 1;
                    if (index != NULL) {
                        t = index[i - 1 + startpoint];
                        index[i - 1 + startpoint] = index[i + startpoint];
                        index[i + startpoint] = t;
                    }
                }
            }
            j--;
        }
    } else {
        while (s) {
            s = 0;
            for (i = 1; i < j; i++) {
                if (farray[i] > farray[i - 1]) {
                    f = farray[i];
                    farray[i] = farray[i - 1];
                    farray[i - 1] = f;
                    s = 1;
                    if (index != NULL) {
                        t = index[i - 1 + startpoint];
                        index[i - 1 + startpoint] = index[i + startpoint];
                        index[i + startpoint] = t;
                    }
                }
            }
            j--;
        }
    }
}

static void stringsort(unsigned char *sarray, int n, int offset, int64_t *index,
                int flags, int startpoint) {
    int ii, i, s = 1, isave;
    int k;
    unsigned char *s1, *s2, *p1, *p2;
    unsigned char temp;
    int reverse = 1 - ((flags & 1) << 1);
    while (s) {
        s = 0;
        for (i = 1; i < n; i++) {
            s2 = i * offset + sarray;
            s1 = (i - 1) * offset + sarray;
            ii = *s1 < *s2 ? *s1 : *s2;  // get the smaller  length
            p1 = s1 + 1;
            p2 = s2 + 1;
            k = 0;  // assume the strings match
            while ((ii--) && (k == 0)) {
                if (flags & 2) {
                    if (toupper(*p1) > toupper(*p2)) {
                        k = reverse;  // earlier in the array is bigger
                    }
                    if (toupper(*p1) < toupper(*p2)) {
                        k = -reverse;  // later in the array is bigger
                    }
                } else {
                    if (*p1 > *p2) {
                        k = reverse;  // earlier in the array is bigger
                    }
                    if (*p1 < *p2) {
                        k = -reverse;  // later in the array is bigger
                    }
                }
                p1++;
                p2++;
            }
            // if up to this point the strings match
            // make the decision based on which one is shorter
            if (k == 0) {
                if (*s1 > *s2) k = reverse;
                if (*s1 < *s2) k = -reverse;
            }
            if (k == 1) {  // if earlier is bigger swap them round
                ii = *s1 > *s2 ? *s1 : *s2;  // get the bigger length
                ii++;
                p1 = s1;
                p2 = s2;
                while (ii--) {
                    temp = *p1;
                    *p1 = *p2;
                    *p2 = temp;
                    p1++;
                    p2++;
                }
                s = 1;
                if (index != NULL) {
                    isave = index[i - 1 + startpoint];
                    index[i - 1 + startpoint] = index[i + startpoint];
                    index[i + startpoint] = isave;
                }
            }
            // routinechecks(1);
        }
    }
}

void cmd_sort(void) {
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    MMFLOAT *a3float = NULL;
    int64_t *a3int = NULL, *a4int = NULL;
    unsigned char *a3str = NULL;
    int i, size, truesize, flags = 0, maxsize = 0, startpoint = 0;
    getargs(&cmdline, 9, ",");
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if (vartbl[VarIndex].type & T_NBR) {
        if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
            error("Argument 1 must be array");
        }
        a3float = (MMFLOAT *)ptr1;
    } else if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
            error("Argument 1 must be array");
        }
        a3int = (int64_t *)ptr1;
    } else if (vartbl[VarIndex].type & T_STR) {
        if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
            error("Argument 1 must be array");
        }
        a3str = (unsigned char *)ptr1;
        maxsize = vartbl[VarIndex].size;
    } else
        error("Argument 1 must be array");
    //if ((uint32_t)ptr1 != (uint32_t)vartbl[VarIndex].val.s)
    if (ptr1 != vartbl[VarIndex].val.s)
        error("Argument 1 must be array");
    truesize = size = (vartbl[VarIndex].dims[0] - OptionBase);
    if (argc >= 3 && *argv[2]) {
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 2 must be integer array");
            }
            a4int = (int64_t *)ptr2;
        } else
            error("Argument 2 must be integer array");
        if ((vartbl[VarIndex].dims[0] - OptionBase) != size)
            error("Arrays should be the same size");
        // if ((uint32_t)ptr2 != (uint32_t)vartbl[VarIndex].val.s)
        if (ptr2 != vartbl[VarIndex].val.s)
            error("Argument 2 must be array");
    }
    if (argc >= 5 && *argv[4]) flags = getint(argv[4], 0, 3);
    if (argc >= 7 && *argv[6])
        startpoint = getint(argv[6], OptionBase, size + OptionBase);
    size -= startpoint;
    if (argc == 9) size = getint(argv[8], 1, size + 1 + OptionBase) - 1;
    if (startpoint) startpoint -= OptionBase;
    if (a3float != NULL) {
        a3float += startpoint;
        if (a4int != NULL)
            for (i = 0; i < truesize + 1; i++) a4int[i] = i + OptionBase;
        floatsort(a3float, size + 1, a4int, flags, startpoint);
    } else if (a3int != NULL) {
        a3int += startpoint;
        if (a4int != NULL)
            for (i = 0; i < truesize + 1; i++) a4int[i] = i + OptionBase;
        integersort(a3int, size + 1, a4int, flags, startpoint);
    } else if (a3str != NULL) {
        a3str += ((startpoint) * (maxsize + 1));
        if (a4int != NULL)
            for (i = 0; i < truesize + 1; i++) a4int[i] = i + OptionBase;
        stringsort(a3str, size + 1, maxsize + 1, a4int, flags, startpoint);
    }
}
