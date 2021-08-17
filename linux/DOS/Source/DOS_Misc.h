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
    void cmd_pause(void);
    void cmd_timer(void);
    void cmd_option(void);
    void cmd_wedit(void);
    void cmd_cursor(void);
    void cmd_colour(void);
    void cmd_settitle(void);

    void fun_date(void);
    void fun_time(void);
    void fun_timer(void);
    void fun_type(void);
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
    { "Cls",        T_CMD,                0, cmd_cls      },
    { "Pause",      T_CMD,                0, cmd_pause    },
    { "Timer",      T_CMD | T_FUN,        0, cmd_timer    },
    { "WEdit",      T_CMD,                0, cmd_wedit    },
    { "Cursor",     T_CMD,                0, cmd_cursor   },
    { "Colour",     T_CMD,                0, cmd_colour   },
    { "Color",      T_CMD,                0, cmd_colour   },
    { "SetTitle",   T_CMD,                0, cmd_settitle },

#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

    { "As",           T_NA,               0, op_invalid   },
    { "Date$",        T_FNA | T_STR,      0, fun_date     },
    { "Time$",        T_FNA | T_STR,      0, fun_time     },
    { "Timer",        T_FNA | T_INT,      0, fun_timer    },
    { "MM.HRes",      T_FNA | T_INT,      0, fun_hres     },
    { "MM.VRes",      T_FNA | T_INT,      0, fun_vres     },
    { "MM.Device$",   T_FNA | T_STR,      0, fun_type     },
    { "MM.CmdLine$",  T_FNA | T_STR,      0, fun_cmdline  },

#endif


#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
    // General definitions used by other modules

    #ifndef DOS_MISC_HEADER
    #define DOS_MISC_HEADER

    struct option_s {
        char Tab;
        char Listcase;
        int  Height;
        int  Width;
        int  ProgFlashSize;
        int  Autorun;
    };

    extern struct option_s Option;


#endif
#endif
