/***********************************************************************************************************************
MMBasic

MM_Misc.h

Include file that contains the globals and defines for DOS_Misc.c in MMBasic.
These are miscelaneous commands and functions that do not easily sit anywhere else.

Copyright 2011 - 2020 Geoff Graham.  All Rights Reserved.

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

    void cmd_exitmmb(void);
    void cmd_system(void);
    void cmd_cls(void);
    void cmd_inc(void);
    void cmd_ireturn(void);
    void cmd_longstring(void);
    void cmd_pause(void);
    void cmd_poke(void);
    void cmd_program(void);
    void cmd_timer(void);
    void cmd_option(void);
    void cmd_wedit(void);
    void cmd_cursor(void);
    void cmd_colour(void);
    void cmd_settitle(void);
    void cmd_sort(void);

    void fun_bound(void);
    void fun_choice(void);
    void fun_date(void);
    void fun_device(void);
    void fun_format(void);
    void fun_info(void);
    void fun_lgetbyte(void);
    void fun_peek(void);
    void fun_time(void);
    void fun_timer(void);
    void fun_cmdline(void);
    void fun_hres(void);
    void fun_vres(void);

#endif

/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#if defined(INCLUDE_COMMAND_TABLE)

    { "Quit",       T_CMD,                0, cmd_exitmmb  },
    { "System",     T_CMD,                0, cmd_system   },
    { "Option",     T_CMD,                0, cmd_option   },
    { "Cat",        T_CMD,                0, cmd_inc      }, // TODO: alias of INC
    { "Cls",        T_CMD,                0, cmd_cls      },
    { "Inc",        T_CMD,                0, cmd_inc      },
    { "IReturn",		T_CMD,			        	0, cmd_ireturn 	},
    { "LongString", T_CMD,                0, cmd_longstring },
    { "Ls",         T_CMD,                0, cmd_files    },
    { "Pause",      T_CMD,                0, cmd_pause    },
    { "Timer",      T_CMD | T_FUN,        0, cmd_timer    },
    { "WEdit",      T_CMD,                0, cmd_wedit    },
    { "Cursor",     T_CMD,                0, cmd_cursor   },
    { "Colour",     T_CMD,                0, cmd_colour   },
    { "Color",      T_CMD,                0, cmd_colour   },
    { "Poke",       T_CMD,                0, cmd_poke     },
    { "Program",    T_CMD,                0, cmd_program  },
    { "SetTitle",   T_CMD,                0, cmd_settitle },
    { "Sort",       T_CMD,                0, cmd_sort     },

#endif

/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

    { "As",           T_NA,               0, op_invalid   },
    { "Bound(",       T_FUN | T_INT,      0, fun_bound    },
    { "Choice(",      T_FUN | T_STR | T_INT | T_NBR, 0, fun_choice },
    { "Date$",        T_FNA | T_STR,      0, fun_date     },
    { "Format$(",     T_FUN | T_STR,      0, fun_format   },
    { "LGetByte(",    T_FUN | T_INT, 0, fun_lgetbyte },
    { "Peek(",        T_FUN | T_INT | T_NBR, 0, fun_peek  },
    { "Time$",        T_FNA | T_STR,      0, fun_time     },
    { "Timer",        T_FNA | T_INT,      0, fun_timer    },
    { "MM.HRes",      T_FNA | T_INT,      0, fun_hres     },
    { "MM.Info(",     T_FUN | T_STR | T_INT | T_NBR, 0, fun_info },
    { "MM.VRes",      T_FNA | T_INT,      0, fun_vres     },
    { "MM.Device$",   T_FNA | T_STR,      0, fun_device   },
    { "MM.CmdLine$",  T_FNA | T_STR,      0, fun_cmdline  },

#endif
