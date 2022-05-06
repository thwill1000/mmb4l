/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmb4l_functions.h

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

// C-language functions to implement MMBasic functions and operators.
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)

    void fun_at(void);
    void fun_bound(void);
    void fun_call(void);
    void fun_choice(void);
    void fun_cwd(void);
    void fun_date(void);
    void fun_datetime(void);
    void fun_day(void);
    void fun_dir(void);
    void fun_eof(void);
    void fun_epoch(void);
    void fun_field(void);
    void fun_format(void);
    void fun_inputstr(void);
    void fun_json(void);
    void fun_lcompare(void);
    void fun_lgetbyte(void);
    void fun_lgetstr(void);
    void fun_linstr(void);
    void fun_llen(void);
    void fun_loc(void);
    void fun_lof(void);
    void fun_math(void);
    void fun_mmcmdline(void);
    void fun_mmdevice(void);
    void fun_mminfo(void);
    void fun_peek(void);
    void fun_time(void);
    void fun_timer(void);
    void fun_rgb(void);
    void fun_hres(void);
    void fun_vres(void);

#endif

// Entries for the token table.
#ifdef INCLUDE_TOKEN_TABLE

    { "@(",           T_FUN | T_STR,      0, fun_at       },
    { "Bound(",       T_FUN | T_INT,      0, fun_bound    },
    { "Call(",        T_FUN | T_STR | T_INT | T_NBR, 0, fun_call },
    { "Choice(",      T_FUN | T_STR | T_INT | T_NBR, 0, fun_choice },
    { "Cwd$",         T_FNA | T_STR,      0, fun_cwd      },
    { "Date$",        T_FNA | T_STR,      0, fun_date     },
    { "DateTime$(",   T_FUN | T_STR,      0, fun_datetime },
    { "Day$(",        T_FUN | T_STR,      0, fun_day      },
    { "Dir$(",        T_FUN | T_STR,      0, fun_dir      },
    { "Eof(",         T_FUN | T_INT,      0, fun_eof      },
    { "Epoch(",       T_FUN | T_INT,      0, fun_epoch    },
    { "Field$(",      T_FUN | T_STR,      0, fun_field    },
    { "Format$(",     T_FUN | T_STR,      0, fun_format   },
    { "Input$(",      T_FUN | T_STR,      0, fun_inputstr },
    { "Json$(",       T_FUN | T_STR,      0, fun_json     },
    { "LCompare(",    T_FUN | T_INT,      0, fun_lcompare },
    { "LGetByte(",    T_FUN | T_INT,      0, fun_lgetbyte },
    { "LGetStr$(",    T_FUN | T_STR,      0, fun_lgetstr  },
    { "LInStr(",      T_FUN | T_INT,      0, fun_linstr   },
    { "LLen(",        T_FUN | T_INT,      0, fun_llen     },
    { "Loc(",         T_FUN | T_INT,      0, fun_loc      },
    { "Lof(",         T_FUN | T_INT,      0, fun_lof      },
    { "Math(",        T_FUN | T_NBR,      0, fun_math     },
    { "Peek(",        T_FUN | T_INT | T_NBR, 0, fun_peek  },
    { "Rgb(",         T_FUN | T_INT,      0, fun_rgb      },
    { "Time$",        T_FNA | T_STR,      0, fun_time     },
    { "Timer",        T_FNA | T_INT,      0, fun_timer    },
    { "MM.HRes",      T_FNA | T_INT,      0, fun_hres     },
    { "MM.Info(",     T_FUN | T_STR | T_INT | T_NBR, 0, fun_mminfo },
    { "MM.Info$(",    T_FUN | T_STR | T_INT | T_NBR, 0, fun_mminfo },
    { "MM.VRes",      T_FNA | T_INT,      0, fun_vres     },
    { "MM.Device$",   T_FNA | T_STR,      0, fun_mmdevice   },
    { "MM.CmdLine$",  T_FNA | T_STR,      0, fun_mmcmdline  },

#endif
