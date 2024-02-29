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
#include "commandtbl.h"
#include "../common/error.h"

#include <strings.h>

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
    { "Box",         T_CMD,              0, cmd_box      },
    { "Call",        T_CMD,              0, cmd_call     },
    { "Case Else",   T_CMD,              0, cmd_case     },
    { "Case",        T_CMD,              0, cmd_case     },
    { "Cat",         T_CMD,              0, cmd_inc      }, // TODO: alias of INC.
    { "Chdir",       T_CMD,              0, cmd_chdir    },
    { "Clear",       T_CMD,              0, cmd_clear    },
    { "Close",       T_CMD,              0, cmd_close    },
    { "Cls",         T_CMD,              0, cmd_cls      },
    { "Color",       T_CMD,              0, cmd_colour   },
    { "Colour",      T_CMD,              0, cmd_colour   },
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
    { "Graphics",    T_CMD,              0, cmd_graphics },
    { "If",          T_CMD,              0, cmd_if       },
    { "Inc",         T_CMD,              0, cmd_inc      },
    { "Input",       T_CMD,              0, cmd_input    },
    { "IReturn",     T_CMD,              0, cmd_ireturn  },
    { "Kill",        T_CMD,              0, cmd_kill     },
    { "Let",         T_CMD,              0, cmd_let      },
    { "Line",        T_CMD,              0, cmd_line     },
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
    { "MmDebug",     T_CMD,              0, cmd_mmdebug  },
    { "Mode",        T_CMD,              0, cmd_mode     },
    { "New",         T_CMD,              0, cmd_new      },
    { "Next",        T_CMD,              0, cmd_next     },
    { "On",          T_CMD,              0, cmd_on       },
    { "Open",        T_CMD,              0, cmd_open     },
    { "Option",      T_CMD,              0, cmd_option   },
    { "Page",        T_CMD,              0, cmd_graphics },
    { "Pause",       T_CMD,              0, cmd_pause    },
    { "Play",        T_CMD,              0, cmd_dummy    },
    { "Pixel",       T_CMD,              0, cmd_pixel    },
    { "Poke",        T_CMD,              0, cmd_poke     },
    { "Print",       T_CMD,              0, cmd_print    },
    { "Quit",        T_CMD,              0, cmd_quit     },
    { "Randomize",   T_CMD,              0, cmd_randomize},
    { "RBox",        T_CMD,              0, cmd_rbox     },
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

CommandToken cmdCASE, cmdCASE_ELSE, cmdCFUN, cmdCSUB, cmdDATA, cmdDO;
CommandToken cmdELSE, cmdELSEIF, cmdELSE_IF, cmdENDIF, cmdEND_CSUB, cmdEND_FUNCTION;
CommandToken cmdENDIF, cmdEND_IF, cmdEND_SELECT, cmdEND_SUB, cmdFOR, cmdFUN;
CommandToken cmdIF, cmdIRET, cmdLET, cmdLOOP, cmdNEXT, cmdPRINT;
CommandToken cmdREM, cmdSELECT_CASE, cmdSUB, cmdWEND, cmdWHILE;

void commandtbl_init() {
    commandtbl_size = sizeof(commandtbl) / sizeof(struct s_tokentbl);

    cmdCASE = commandtbl_get("Case");
    cmdCASE_ELSE = commandtbl_get("Case Else");
    cmdCFUN = INVALID_COMMAND_TOKEN;
    cmdCSUB = commandtbl_get("CSub");
    cmdDATA = commandtbl_get("Data");
    cmdDO = commandtbl_get("Do");
    cmdELSE = commandtbl_get("Else");
    cmdELSEIF = commandtbl_get("ElseIf");
    cmdELSE_IF = commandtbl_get("Else If");
    cmdENDIF = commandtbl_get("EndIf");
    cmdEND_CSUB = commandtbl_get("End CSub");
    cmdEND_FUNCTION = commandtbl_get("End Function");
    cmdEND_IF = commandtbl_get("End If");
    cmdEND_SELECT = commandtbl_get("End Select");
    cmdEND_SUB = commandtbl_get("End Sub");
    cmdFOR = commandtbl_get("For");
    cmdFUN = commandtbl_get("Function");
    cmdIF = commandtbl_get("If");
    cmdIRET = commandtbl_get("IReturn");
    cmdLET = commandtbl_get("Let");
    cmdLOOP = commandtbl_get("Loop");
    cmdNEXT = commandtbl_get("Next");
    cmdPRINT = commandtbl_get("Print");
    cmdREM = commandtbl_get("Rem");
    cmdSELECT_CASE = commandtbl_get("Select Case");
    cmdSUB = commandtbl_get("Sub");
    cmdWEND = commandtbl_get("WEnd");
    cmdWHILE = commandtbl_get("While");
}

CommandToken commandtbl_get(const char *s) {
    for (int i = 0; i < commandtbl_size - 1; i++) {
        if (strcasecmp(s, commandtbl[i].name) == 0) {
            return i;
        }
    }
    ERROR_INTERNAL_FAULT;
    return INVALID_COMMAND_TOKEN;
}
