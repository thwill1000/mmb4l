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

void cmd_autosave(void);
void cmd_call(void);
void cmd_case(void);
void cmd_cfunction(void);
void cmd_chdir(void);
void cmd_clear(void);
void cmd_close(void);
void cmd_cls(void);
void cmd_console(void);
void cmd_const(void);
void cmd_continue(void);
void cmd_copy(void);
void cmd_cursor(void);
void cmd_dim(void);
void cmd_do(void);
void cmd_dummy(void);
void cmd_edit(void);
void cmd_else(void);
void cmd_endfun(void);
void cmd_endsub(void);
void cmd_end(void);
void cmd_erase(void);
void cmd_error(void);
void cmd_execute(void);
void cmd_exitfor(void);
void cmd_exit(void);
void cmd_files(void);
void cmd_font(void);
void cmd_for(void);
void cmd_gosub(void);
void cmd_goto(void);
void cmd_if(void);
void cmd_inc(void);
void cmd_input(void);
void cmd_ireturn(void);
void cmd_kill(void);
void cmd_let(void);
void cmd_lineinput(void);
void cmd_list(void);
void cmd_load(void);
void cmd_longstring(void);
void cmd_loop(void);
void cmd_math(void);
void cmd_memory(void);
void cmd_mid(void);
void cmd_mkdir(void);
void cmd_mode(void);
void cmd_new(void);
void cmd_next(void);
void cmd_null(void);
void cmd_null(void);
void cmd_on(void);
void cmd_open(void);
void cmd_option(void);
void cmd_pause(void);
void cmd_poke(void);
void cmd_print(void);
void cmd_quit(void);
void cmd_randomize(void);
void cmd_read(void);
void cmd_rename(void);
void cmd_restore(void);
void cmd_return(void);
void cmd_rmdir(void);
void cmd_run(void);
void cmd_seek(void);
void cmd_select(void);
void cmd_settick(void);
void cmd_settitle(void);
void cmd_sort(void);
void cmd_subfun(void);
void cmd_system(void);
void cmd_timer(void);
void cmd_trace(void);
void cmd_troff(void);
void cmd_tron(void);
void cmd_xmodem(void);

/**
 * This is the command table that defines the various tokens for MMBasic commands.
 *
 * The format is:
 *    TEXT           TYPE                P  FUNCTION TO CALL
 *
 * TYPE - T_NA, T_FUN, T_FNA or T_OPER augmented by the types T_STR
 *        and/or T_NBR and/or T_INT
 * P    - Is the precedence, which is only used for operators.
 */
const struct s_tokentbl commandtbl[] = {
    { "AutoSave",    T_CMD,              0, cmd_autosave },
    { "Call",        T_CMD,              0, cmd_call     },
    { "Case Else",   T_CMD,              0, cmd_case     },
    { "Case",        T_CMD,              0, cmd_case     },
    { "Cat",         T_CMD,              0, cmd_inc      }, // TODO: alias of INC.
    { "Chdir",       T_CMD,              0, cmd_chdir    },
    { "Clear",       T_CMD,              0, cmd_clear    },
    { "Close",       T_CMD,              0, cmd_close    },
    { "Cls",         T_CMD,              0, cmd_cls      },
    { "Color",       T_CMD,              0, cmd_dummy    },
    { "Colour",      T_CMD,              0, cmd_dummy    },
    { "Console",     T_CMD,              0, cmd_console  },
    { "Const",       T_CMD,              0, cmd_const    },
    { "Continue",    T_CMD,              0, cmd_continue },
    { "Copy",        T_CMD,              0, cmd_copy     },
    { "CSub",        T_CMD,              0, cmd_cfunction },
    { "Cursor",      T_CMD,              0, cmd_cursor   },
    { "Data",        T_CMD,              0, cmd_null     },
    { "Dim",         T_CMD,              0, cmd_dim      },
    { "Do",          T_CMD,              0, cmd_do       },
    { "Edit",        T_CMD,              0, cmd_edit     },
    { "Else If",     T_CMD,              0, cmd_else     },
    { "ElseIf",      T_CMD,              0, cmd_else     },
    { "Else",        T_CMD,              0, cmd_else     },
    { "End CSub",    T_CMD,              0, cmd_null     },
    { "End Function", T_CMD,             0, cmd_endfun   },
    { "End If",      T_CMD,              0, cmd_null     },
    { "EndIf",       T_CMD,              0, cmd_null     },
    { "End Select",  T_CMD,              0, cmd_null     },
    { "End Sub",     T_CMD,              0, cmd_return   },
    { "End",         T_CMD,              0, cmd_end      },
    { "Erase",       T_CMD,              0, cmd_erase    },
    { "Error",       T_CMD,              0, cmd_error    },
    { "Execute",     T_CMD,              0, cmd_execute  },
    { "Exit Do",     T_CMD,              0, cmd_exit     },
    { "Exit For",    T_CMD,              0, cmd_exitfor  },
    { "Exit Function", T_CMD,            0, cmd_endfun   },
    { "Exit Sub",    T_CMD,              0, cmd_return   },
    { "Exit",        T_CMD,              0, cmd_exit     },
    { "Files",       T_CMD,              0, cmd_files    },
    { "Font",        T_CMD,              0, cmd_dummy    },
    { "For",         T_CMD,              0, cmd_for      },
    { "Function",    T_CMD,              0, cmd_subfun   },
    { "GoSub",       T_CMD,              0, cmd_gosub    },
    { "GoTo",        T_CMD,              0, cmd_goto     },
    { "If",          T_CMD,              0, cmd_if       },
    { "Inc",         T_CMD,              0, cmd_inc      },
    { "Input",       T_CMD,              0, cmd_input    },
    { "IReturn",     T_CMD,              0, cmd_ireturn  },
    { "Kill",        T_CMD,              0, cmd_kill     },
    { "Let",         T_CMD,              0, cmd_let      },
    { "Line Input",  T_CMD,              0, cmd_lineinput},
    { "List",        T_CMD,              0, cmd_list     },
    { "Load",        T_CMD,              0, cmd_load     },
    { "Local",       T_CMD,              0, cmd_dim      },
    { "LongString",  T_CMD,              0, cmd_longstring },
    { "Loop",        T_CMD,              0, cmd_loop     },
    { "Math",        T_CMD,              0, cmd_math     },
    { "Memory",      T_CMD,              0, cmd_memory   },
    { "Mid$(",       T_CMD | T_FUN,      0, cmd_mid      },
    { "Mkdir",       T_CMD,              0, cmd_mkdir    },
    { "Mode",        T_CMD,              0, cmd_mode     },
    { "New",         T_CMD,              0, cmd_new      },
    { "Next",        T_CMD,              0, cmd_next     },
    { "On",          T_CMD,              0, cmd_on       },
    { "Open",        T_CMD,              0, cmd_open     },
    { "Option",      T_CMD,              0, cmd_option   },
    { "Page",        T_CMD,              0, cmd_dummy    },
    { "Pause",       T_CMD,              0, cmd_pause    },
    { "Play",        T_CMD,              0, cmd_dummy    },
    { "Poke",        T_CMD,              0, cmd_poke     },
    { "Print",       T_CMD,              0, cmd_print    },
    { "Quit",        T_CMD,              0, cmd_quit     },
    { "Randomize",   T_CMD,              0, cmd_randomize},
    { "Read",        T_CMD,              0, cmd_read     },
    { "Rem",         T_CMD,              0, cmd_null,    },
    { "Rename",      T_CMD,              0, cmd_rename   },
    { "Restore",     T_CMD,              0, cmd_restore  },
    { "Return",      T_CMD,              0, cmd_return,  },
    { "Rmdir",       T_CMD,              0, cmd_rmdir    },
    { "Run",         T_CMD,              0, cmd_run      },
    { "Seek",        T_CMD,              0, cmd_seek     },
    { "Select Case", T_CMD,              0, cmd_select   },
    { "SetTick",     T_CMD,              0, cmd_settick  },
    { "SetTitle",    T_CMD,              0, cmd_settitle },
    { "Sort",        T_CMD,              0, cmd_sort     },
    { "Static",      T_CMD,              0, cmd_dim      },
    { "Sub",         T_CMD,              0, cmd_subfun   },
    { "System",      T_CMD,              0, cmd_system   },
    { "Text",        T_CMD,              0, cmd_dummy    },
    { "Timer",       T_CMD | T_FUN,      0, cmd_timer    },
    { "Trace",       T_CMD,              0, cmd_trace    },
    { "TrOff",       T_CMD,              0, cmd_troff    },
    { "TrOn",        T_CMD,              0, cmd_tron     },
    { "Wend",        T_CMD,              0, cmd_loop     },
    { "While",       T_CMD,              0, cmd_do       },
    { "XModem",      T_CMD,              0, cmd_xmodem   },
    { "",            0,                  0, cmd_null,    }  // This dummy entry is always at the end.
};

int commandtbl_size() {
    return sizeof(commandtbl) / sizeof(struct s_tokentbl);
}
