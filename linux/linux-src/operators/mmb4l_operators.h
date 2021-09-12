// C-language functions to implement MMBasic functions and operators.
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)

    void op_inv(void);

#endif

// Entries for the token table.
#ifdef INCLUDE_TOKEN_TABLE

    { "As",           T_NA,               0, op_invalid   },
    { "Inv",          T_OPER | T_NBR,     3, op_inv       },

#endif
