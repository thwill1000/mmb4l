/*-*****************************************************************************

MMBasic for Linux (MMB4L)

MMBasic.c

Copyright 2011-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include "../Configuration.h"
#include "MMBasic.h"
#include "tokentbl.h"
#include "../common/error.h"

#include <strings.h>

/**
 * This is the token table that defines the various tokens for MMBasic functions
 * and operators.
 *
 * The format is:
 *    TEXT           TYPE                P  FUNCTION TO CALL
 *
 * TYPE - T_NA, T_FUN, T_FNA or T_OPER augmented by the types T_STR
 *        and/or T_NBR and/or T_INT
 * P    - Is the precedence, which is only used for operators.
 */
const struct s_tokentbl tokentbl[] = {
    { "@(",          T_FUN | T_STR,      0, fun_at       },
    { "Abs(",        T_FUN | T_NBR | T_INT, 0, fun_abs   },
    { "ACos(",       T_FUN | T_NBR,      0, fun_acos     },
    { "Asc(",        T_FUN | T_INT,      0, fun_asc      },
    { "ASin(",       T_FUN | T_NBR,      0, fun_asin     },
    { "Atn(",        T_FUN | T_NBR,      0, fun_atn      },
    { "Bin$(",       T_FUN | T_STR,      0, fun_bin      },
    { "Bin2Str$(" ,  T_FUN | T_STR,      0, fun_bin2str  },
    { "Bound(",      T_FUN | T_INT,      0, fun_bound    },
    { "Call(",       T_FUN | T_STR | T_INT | T_NBR, 0, fun_call },
    { "Choice(",     T_FUN | T_STR | T_INT | T_NBR, 0, fun_choice },
    { "Chr$(",       T_FUN | T_STR,      0, fun_chr,     },
    { "Cint(",       T_FUN | T_INT,      0, fun_cint     },
    { "Classic(",    T_FUN | T_INT,      0, fun_classic  },
    { "Cos(",        T_FUN | T_NBR,      0, fun_cos      },
    { "Cwd$",        T_FNA | T_STR,      0, fun_cwd      },
    { "Date$",       T_FNA | T_STR,      0, fun_date     },
    { "DateTime$(",  T_FUN | T_STR,      0, fun_datetime },
    { "Day$(",       T_FUN | T_STR,      0, fun_day      },
    { "Deg(",        T_FUN | T_NBR,      0, fun_deg      },
    { "Device(",     T_FUN | T_INT,      0, fun_device   },
    { "Dir$(",       T_FUN | T_STR,      0, fun_dir      },
    { "Eof(",        T_FUN | T_INT,      0, fun_eof      },
    { "Epoch(",      T_FUN | T_INT,      0, fun_epoch    },
    { "Eval(",       T_FUN | T_NBR | T_INT | T_STR, 0, fun_eval },
    { "Exp(",        T_FUN | T_NBR,      0, fun_exp      },
    { "Field$(",     T_FUN | T_STR,      0, fun_field    },
    { "Fix(",        T_FUN | T_INT,      0, fun_fix      },
    { "Format$(",    T_FUN | T_STR,      0, fun_format   },
    { "Hex$(",       T_FUN | T_STR,      0, fun_hex      },
    { "Inkey$",      T_FNA | T_STR,      0, fun_inkey    },
    { "Input$(",     T_FUN | T_STR,      0, fun_inputstr },
    { "Instr(",      T_FUN | T_INT,      0, fun_instr    },
    { "Int(",        T_FUN | T_INT,      0, fun_int      },
    { "Json$(",      T_FUN | T_STR,      0, fun_json     },
    { "Keydown(",    T_FUN | T_INT,      0, fun_keydown  },
    { "LCase$(",     T_FUN | T_STR,      0, fun_lcase    },
    { "LCompare(",   T_FUN | T_INT,      0, fun_lcompare },
    { "Left$(",      T_FUN | T_STR,      0, fun_left     },
    { "Len(",        T_FUN | T_INT,      0, fun_len      },
    { "LGetByte(",   T_FUN | T_INT,      0, fun_lgetbyte },
    { "LGetStr$(",   T_FUN | T_STR,      0, fun_lgetstr  },
    { "LInStr(",     T_FUN | T_INT,      0, fun_linstr   },
    { "LLen(",       T_FUN | T_INT,      0, fun_llen     },
    { "Loc(",        T_FUN | T_INT,      0, fun_loc      },
    { "Lof(",        T_FUN | T_INT,      0, fun_lof      },
    { "Log(",        T_FUN | T_NBR,      0, fun_log      },
    { "Math(",       T_FUN | T_NBR,      0, fun_math     },
    { "Max(",        T_FUN | T_NBR,      0, fun_max      },
    { "Mid$(",       T_FUN | T_STR,      0, fun_mid      },
    { "Min(",        T_FUN | T_NBR,      0, fun_min      },
    { "MM.CmdLine$", T_FNA | T_STR,      0, fun_mmcmdline },
    { "MM.Device$",  T_FNA | T_STR,      0, fun_mmdevice },
    { "MM.ErrMsg$",  T_FNA | T_STR,      0, fun_errmsg   },
    { "MM.Errno",    T_FNA | T_INT,      0, fun_errno    },
    { "MM.HRes",     T_FNA | T_INT,      0, fun_hres     },
    { "MM.Info$(",   T_FUN | T_STR | T_INT | T_NBR, 0, fun_mminfo },
    { "MM.Info(",    T_FUN | T_STR | T_INT | T_NBR, 0, fun_mminfo },
    { "MM.Ver",      T_FNA | T_INT,      0, fun_version  },
    { "MM.VRes",     T_FNA | T_INT,      0, fun_vres     },
    { "Nunchuk(",    T_FUN | T_INT,      0, fun_classic  },
    { "Oct$(",       T_FUN | T_STR,      0, fun_oct      },
    { "Peek(",       T_FUN | T_INT | T_NBR, 0, fun_peek  },
    { "Pi",          T_FNA | T_NBR,      0, fun_pi       },
    { "Pin(",        T_FUN | T_NBR | T_INT, 0, fun_pin   },
    { "Port(",       T_FUN | T_INT,      0, fun_port     },
    { "Pos",         T_FNA | T_INT,      0, fun_pos      },
    { "Rad(",        T_FUN | T_NBR,      0, fun_rad      },
    { "Rgb(",        T_FUN | T_INT,      0, fun_rgb      },
    { "Right$(",     T_FUN | T_STR,      0, fun_right    },
    { "Rnd(",        T_FUN | T_NBR,      0, fun_rnd      },  // This must come before Rnd - without bracket.
    { "Rnd",         T_FNA | T_NBR,      0, fun_rnd      },  // This must come after Rnd(.
    { "Sgn(",        T_FUN | T_INT,      0, fun_sgn      },
    { "Sin(",        T_FUN | T_NBR,      0, fun_sin      },
    { "Space$(",     T_FUN | T_STR,      0, fun_space    },
    { "Spc(",        T_FUN | T_STR,      0, fun_space    },
    { "Sqr(",        T_FUN | T_NBR,      0, fun_sqr      },
    { "Str$(",       T_FUN | T_STR,      0, fun_str      },
    { "Str2Bin(",    T_FUN | T_NBR | T_INT, 0, fun_str2bin  },
    { "String$(",    T_FUN | T_STR,      0, fun_string   },
    { "Tab(",        T_FUN | T_STR,      0, fun_tab,     },
    { "Tan(",        T_FUN | T_NBR,      0, fun_tan      },
    { "Time$",       T_FNA | T_STR,      0, fun_time     },
    { "Timer",       T_FNA | T_INT,      0, fun_timer    },
    { "UCase$(",     T_FUN | T_STR,      0, fun_ucase    },
    { "Val(",        T_FUN | T_NBR | T_INT, 0, fun_val   },

    { "As",          T_NA,               0, op_invalid   },
    { "Else",        T_NA,               0, op_invalid   },
    { "For",         T_NA,               0, op_invalid   },
    { "GoSub",       T_NA,               0, op_invalid   },
    { "GoTo",        T_NA,               0, op_invalid   },
    { "Step",        T_NA,               0, op_invalid   },
    { "Then",        T_NA,               0, op_invalid   },
    { "To",          T_NA,               0, op_invalid   },
    { "Until",       T_NA,               0, op_invalid   },
    { "While",       T_NA,               0, op_invalid   },

    { "^",           T_OPER | T_NBR | T_INT,         0, op_exp       },
    { "*",           T_OPER | T_NBR | T_INT,         1, op_mul       },
    { "/",           T_OPER | T_NBR,                 1, op_div       },
    { "\\",          T_OPER | T_INT,                 1, op_divint    },
    { "Mod",         T_OPER | T_INT,                 1, op_mod       },
    { "+",           T_OPER | T_NBR | T_INT | T_STR, 2, op_add       },
    { "-",           T_OPER | T_NBR | T_INT,         2, op_subtract  },
    { "Not",         T_OPER | T_NBR,                 3, op_not       },
    { "Inv",         T_OPER | T_NBR,                 3, op_inv       },
    { "<<",          T_OPER | T_INT,                 4, op_shiftleft },  // This must come before less than (<)
    { ">>",          T_OPER | T_INT,                 4, op_shiftright},  // This must come before greater than (>)
    { "<>",          T_OPER | T_NBR | T_INT | T_STR, 5, op_ne        },  // This must come before less than (<)
    { ">=",          T_OPER | T_NBR | T_INT | T_STR, 5, op_gte       },  // This must come before greater than (>)
    { "=>",          T_OPER | T_NBR | T_INT | T_STR, 5, op_gte       },  // This must come before equals (=)
    { "<=",          T_OPER | T_NBR | T_INT | T_STR, 5, op_lte       },  // This must come before less than (<)
    { "=<",          T_OPER | T_NBR | T_INT | T_STR, 5, op_lte       },  // This must come before equals (=)
    { "<",           T_OPER | T_NBR | T_INT | T_STR, 5, op_lt        },
    { ">",           T_OPER | T_NBR | T_INT | T_STR, 5, op_gt        },
    { "=",           T_OPER | T_NBR | T_INT | T_STR, 6, op_equal     },
    { "And",         T_OPER | T_INT,                 7, op_and       },
    { "Or",          T_OPER | T_INT,                 7, op_or        },
    { "Xor",         T_OPER | T_INT,                 7, op_xor       },

    { "",            0,                              0, cmd_null,    }  // This dummy entry is always at the end.
};

char tokenTHEN, tokenELSE, tokenGOTO, tokenEQUAL, tokenTO, tokenSTEP;
char tokenWHILE, tokenUNTIL, tokenGOSUB, tokenAS, tokenFOR;

void tokentbl_init() {
    tokentbl_size = sizeof(tokentbl) / sizeof(struct s_tokentbl);

    tokenTHEN  = tokentbl_get("Then");
    tokenELSE  = tokentbl_get("Else");
    tokenGOTO  = tokentbl_get("GoTo");
    tokenEQUAL = tokentbl_get("=");
    tokenTO    = tokentbl_get("To");
    tokenSTEP  = tokentbl_get("Step");
    tokenWHILE = tokentbl_get("While");
    tokenUNTIL = tokentbl_get("Until");
    tokenGOSUB = tokentbl_get("GoSub");
    tokenAS    = tokentbl_get("As");
    tokenFOR   = tokentbl_get("For");
}

int tokentbl_get(const char *s) {
    for (int i = 0; i < tokentbl_size - 1; i++) {
        if (strcasecmp(s, tokentbl[i].name) == 0) {
            return i + C_BASETOKEN;
        }
    }
    ERROR_INTERNAL_FAULT;
    return 0;
}
