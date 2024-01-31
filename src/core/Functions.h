/*-*****************************************************************************

MMBasic for Linux (MMB4L)

Functions.h

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

void fun_abs(void);
void fun_asc(void);
void fun_atn(void);
void fun_bin(void);
void fun_bin2str(void);
void fun_chr(void);
void fun_cint(void);
void fun_cos(void);
void fun_deg(void);
void fun_errno(void);
void fun_errmsg(void);
void fun_exp(void);
void fun_fix(void);
void fun_hex(void);
void fun_inkey(void);
void fun_instr(void);
void fun_int(void);
void fun_lcase(void);
void fun_left(void);
void fun_len(void);
void fun_log(void);
void fun_mid(void);
void fun_oct(void);
void fun_pi(void);
void fun_pos(void);
void fun_rad(void);
void fun_right(void);
void fun_rnd(void);
void fun_sgn(void);
void fun_sin(void);
void fun_space(void);
void fun_sqr(void);
void fun_str(void);
void fun_string(void);
void fun_str2bin(void);
void fun_tab(void);
void fun_tan(void);
void fun_ucase(void);
void fun_val(void);
#if !defined(MX170)
    void fun_eval(void);
#endif
void fun_version(void);
void fun_asin(void);
void fun_acos(void);
void fun_max(void);
void fun_min(void);


#define RADCONV   57.2957795130823229     // Used when converting degrees -> radians and vice versa
#define PI_VALUE  3.14159265358979323
#define Rad(a)  (((MMFLOAT)a) / RADCONV)


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
// note that the variable types (T_STR, etc) are not used as it is the responsibility of the function
// to set its return type in targ.
// and P is the precedence (which is only used for operators)
  { "ACos(",      T_FUN  | T_NBR,         0, fun_acos     },
  { "Abs(",       T_FUN  | T_NBR | T_INT, 0, fun_abs      },
  { "Asc(",       T_FUN  | T_INT,         0, fun_asc      },
  { "ASin(",      T_FUN  | T_NBR,         0, fun_asin     },
  { "Atn(",       T_FUN  | T_NBR,         0, fun_atn      },
  { "Bin$(",      T_FUN  | T_STR,         0, fun_bin      },
  { "Bin2Str$(",  T_FUN  | T_STR,         0, fun_bin2str  },
  { "Chr$(",      T_FUN  | T_STR,         0, fun_chr,     },
  { "Cint(",      T_FUN  | T_INT,         0, fun_cint     },
  { "Cos(",       T_FUN  | T_NBR,         0, fun_cos      },
  { "Deg(",       T_FUN  | T_NBR,         0, fun_deg      },
  { "MM.Errno",   T_FNA  | T_INT,         0, fun_errno    },
  { "MM.ErrMsg$", T_FNA  | T_STR,         0, fun_errmsg   },
  { "Exp(",       T_FUN  | T_NBR,         0, fun_exp      },
  { "Fix(",       T_FUN  | T_INT,         0, fun_fix      },
  { "Hex$(",      T_FUN  | T_STR,         0, fun_hex      },
  { "Inkey$",     T_FNA  | T_STR,         0, fun_inkey    },
  { "Instr(",     T_FUN  | T_INT,         0, fun_instr    },
  { "Int(",       T_FUN  | T_INT,         0, fun_int      },
  { "LCase$(",    T_FUN  | T_STR,         0, fun_lcase    },
  { "Left$(",     T_FUN  | T_STR,         0, fun_left     },
  { "Len(",       T_FUN  | T_INT,         0, fun_len      },
  { "Log(",       T_FUN  | T_NBR,         0, fun_log      },
  { "Mid$(",      T_FUN  | T_STR,         0, fun_mid      },
  { "MM.Ver",     T_FNA  | T_INT,         0, fun_version  },
  { "Oct$(",      T_FUN  | T_STR,         0, fun_oct      },
  { "Pi",         T_FNA  | T_NBR,         0, fun_pi       },
  { "Pos",        T_FNA  | T_INT,         0, fun_pos      },
  { "Rad(",       T_FUN  | T_NBR,         0, fun_rad      },
  { "Right$(",    T_FUN  | T_STR,         0, fun_right    },
  { "Rnd(",       T_FUN  | T_NBR,         0, fun_rnd      },      // this must come before Rnd - without bracket
  { "Rnd",        T_FNA  | T_NBR,         0, fun_rnd      },      // this must come after Rnd(
  { "Sgn(",       T_FUN  | T_INT,         0, fun_sgn      },
  { "Sin(",       T_FUN  | T_NBR,         0, fun_sin      },
  { "Space$(",    T_FUN  | T_STR,         0, fun_space    },
  { "Spc(",       T_FUN  | T_STR,         0, fun_space    },
  { "Sqr(",       T_FUN  | T_NBR,         0, fun_sqr      },
  { "Str$(",      T_FUN  | T_STR,         0, fun_str      },
  { "String$(",   T_FUN  | T_STR,         0, fun_string   },
  { "Str2Bin(",   T_FUN  | T_NBR | T_INT, 0, fun_str2bin  },
  { "Tab(",       T_FUN  | T_STR,         0, fun_tab,     },
  { "Tan(",       T_FUN  | T_NBR,         0, fun_tan      },
  { "UCase$(",    T_FUN  | T_STR,         0, fun_ucase    },
  { "Val(",       T_FUN  | T_NBR | T_INT, 0, fun_val      },
#if !defined(MX170)
  { "Eval(",      T_FUN  | T_NBR | T_INT | T_STR,   0, fun_eval   },
#endif
  { "Max(",       T_FUN  | T_NBR,         0, fun_max      },
  { "Min(",       T_FUN  | T_NBR,         0, fun_min      },

#endif
