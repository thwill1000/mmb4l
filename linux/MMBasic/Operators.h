/***********************************************************************************************************************
MMBasic

operators.h

Include file that contains the globals and defines for operators.c in MMBasic.

Copyright 2011 - 2021 Geoff Graham.  All Rights Reserved.

This file and modified versions of this file are supplied to specific individuals or organisations under the following
provisions:

- This file, or any files that comprise the MMBasic source (modified or not), may not be distributed or copied to any other
  person or organisation without written permission.

- Object files (.o and .hex files) generated using this file (modified or not) may not be distributed or copied to any other
  person or organisation without written permission.

- This file is provided in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

************************************************************************************************************************/



/**********************************************************************************
 the C language function associated with commands, functions or operators should be
 declared here
**********************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
// format:
//      void cmd_???(void)
//      void fun_???(void)
//      void op_???(void)

void op_invalid(void);
void op_exp(void);
void op_mul(void);
void op_div(void);
void op_divint(void);
void op_add(void);
void op_subtract(void);
void op_mod(void);
void op_ne(void);
void op_gte(void);
void op_lte(void);
void op_lt(void);
void op_gt(void);
void op_equal(void);
void op_and(void);
void op_or(void);
void op_xor(void);
void op_not(void);
void op_shiftleft(void);
void op_shiftright(void);

#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE
// the format is:
//    TEXT        TYPE                P  FUNCTION TO CALL
// where type is always T_CMD
// and P is the precedence (which is only used for operators and not commands)

#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE
// the format is:
//    TEXT        TYPE                    P  FUNCTION TO CALL
// where type is T_NA, T_FUN, T_FNA or T_OPER augmented by the types T_STR and/or T_NBR and/or T_INT
// and P is the precedence (which is only used for operators)
  { "^",          T_OPER | T_NBR | T_INT,           0, op_exp       },
  { "*",          T_OPER | T_NBR | T_INT,           1, op_mul       },
  { "/",          T_OPER | T_NBR,                   1, op_div       },
  { "\\",         T_OPER | T_INT,                   1, op_divint    },
  { "Mod",        T_OPER | T_INT,                   1, op_mod       },
  { "+",          T_OPER | T_NBR | T_INT | T_STR,   2, op_add       },
  { "-",          T_OPER | T_NBR | T_INT,           2, op_subtract  },
  { "Not",        T_OPER | T_NBR,                   3, op_not       },
  { "<<",         T_OPER | T_INT,                   4, op_shiftleft },      // this must come before less than (<)
  { ">>",         T_OPER | T_INT,                   4, op_shiftright},      // this must come before greater than (>)
  { "<>",         T_OPER | T_NBR | T_INT | T_STR,   5, op_ne        },      // this must come before less than (<)
  { ">=",         T_OPER | T_NBR | T_INT | T_STR,   5, op_gte       },      // this must come before greater than (>)
  { "=>",         T_OPER | T_NBR | T_INT | T_STR,   5, op_gte       },      // this must come before equals (=)
  { "<=",         T_OPER | T_NBR | T_INT | T_STR,   5, op_lte       },      // this must come before less than (<)
  { "=<",         T_OPER | T_NBR | T_INT | T_STR,   5, op_lte       },      // this must come before equals (=)
  { "<",          T_OPER | T_NBR | T_INT | T_STR,   5, op_lt        },
  { ">",          T_OPER | T_NBR | T_INT | T_STR,   5, op_gt        },
  { "=",          T_OPER | T_NBR | T_INT | T_STR,   6, op_equal     },
  { "And",        T_OPER | T_INT,                   7, op_and       },
  { "Or",         T_OPER | T_INT,                   7, op_or        },
  { "Xor",        T_OPER | T_INT,                   7, op_xor       },

#endif

