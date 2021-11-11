// C-language functions to implement MMBasic functions and operators.
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)

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
