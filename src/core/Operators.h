/*-*****************************************************************************

MMBasic for Linux (MMB4L)

Operators.h

Copyright 2011-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

