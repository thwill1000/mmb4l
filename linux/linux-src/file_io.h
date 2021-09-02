/***********************************************************************************************************************
MMBasic

File_IO.h

Include file that contains the globals and defines for File_IO.c in MMBasic.

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

#define FF_MAX_LFN  127

/**********************************************************************************
 the C language function associated with commands, functions or operators should be
 declared here
**********************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
    void cmd_chdir(void);
    void cmd_close(void);
    void cmd_copy(void);
    void cmd_files(void);
    void cmd_kill(void);
    void cmd_load(void);
    void cmd_mkdir(void);
    void cmd_name(void);
    void cmd_open(void);
    void cmd_reload(void);
    void cmd_rmdir(void);
    void cmd_seek(void);
    void cmd_save(void);
    void fun_inputstr(void);
    void fun_cwd(void);
    void fun_dir(void);
    void fun_eof(void);
    void fun_loc(void);
    void fun_lof(void);

#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE

    { "Chdir",        T_CMD,              0, cmd_chdir    },
    { "Close",        T_CMD,              0, cmd_close    },
    { "Copy",         T_CMD,              0, cmd_copy     },
    { "Files",        T_CMD,              0, cmd_files    },
    { "Kill",         T_CMD,              0, cmd_kill     },
    { "Load",         T_CMD,              0, cmd_load     },
    { "Mkdir",        T_CMD,              0, cmd_mkdir    },
    { "Name",         T_CMD,              0, cmd_name     },
    { "Open",         T_CMD,              0, cmd_open     },
    { "Reload",       T_CMD,              0, cmd_reload   },
    { "Rmdir",        T_CMD,              0, cmd_rmdir    },
    { "Seek",         T_CMD,              0, cmd_seek     },
    { "Save",         T_CMD,              0, cmd_save     },

#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

    { "Input$(",  T_FUN | T_STR,      0, fun_inputstr },
    { "Eof(",     T_FUN | T_INT,      0, fun_eof      },
    { "Loc(",     T_FUN | T_INT,      0, fun_loc      },
    { "Lof(",     T_FUN | T_INT,      0, fun_lof      },
    { "Cwd$",     T_FNA | T_STR,      0, fun_cwd      },
    { "Dir$(",     T_FUN | T_STR,      0, fun_dir      },

#endif


#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
    // General definitions used by other modules

    #ifndef FILE_IO_HEADER
    #define FILE_IO_HEADER

    // File related I/O
    extern char MMfputc(char c, int fnbr);
    extern int MMfgetc(int filenbr);
    extern void MMfopen(char *fname, char *mode, int fnbr);
    extern int MMfeof(int filenbr);
    extern void MMfclose(int fnbr);
    extern int FindFreeFileNbr(void);
    extern void CloseAllFiles(void);
    extern void MMgetline(int filenbr, char *p);

    extern int OptionFileErrorAbort;
    extern char CurrentFile[STRINGSIZE];

#endif
#endif
