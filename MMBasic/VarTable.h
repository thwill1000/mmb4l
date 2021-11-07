#if !defined(VAR_TABLE_H)
#define VAR_TABLE_H

struct s_vartbl {                                     // structure of the variable table
    char name[MAXVARLEN];                             // variable's name
    char type;                                        // its type (T_NUM, T_INT or T_STR)
    char level;                                       // its subroutine or function level (used to track local variables)
    short int dims[MAXDIM];                           // the dimensions. it is an array if the first dimension is NOT zero
    unsigned char size;                               // the number of chars to allocate for each element in a string array
    union u_val {
        MMFLOAT f;                                    // the value if it is a float
        long long int i;                              // the value if it is an integer
        MMFLOAT *fa;                                  // pointer to the allocated memory if it is an array of floats
        long long int *ia;                            // pointer to the allocated memory if it is an array of integers
        char *s;                                      // pointer to the allocated memory if it is a string
    } __attribute__ ((aligned (8))) val;
};

#endif
