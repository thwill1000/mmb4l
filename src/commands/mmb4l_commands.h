/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmb4l_commands.h

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

// C-language functions to implement MMBasic commands.
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)

    void cmd_autosave(void);
    void cmd_call(void);
    void cmd_chdir(void);
    void cmd_close(void);
    void cmd_cls(void);
    void cmd_console(void);
    void cmd_copy(void);
    void cmd_cursor(void);
    void cmd_cfunction(void);
    void cmd_dummy(void);
    void cmd_edit(void);
    void cmd_execute(void);
    void cmd_files(void);
    void cmd_font(void);
    void cmd_inc(void);
    void cmd_ireturn(void);
    void cmd_kill(void);
    void cmd_load(void);
    void cmd_longstring(void);
    void cmd_math(void);
    void cmd_memory(void);
    void cmd_mid(void);
    void cmd_mkdir(void);
    void cmd_mode(void);
    void cmd_open(void);
    void cmd_option(void);
    void cmd_pause(void);
    void cmd_poke(void);
    void cmd_quit(void);
    void cmd_rename(void);
    void cmd_rmdir(void);
    void cmd_seek(void);
    void cmd_settick(void);
    void cmd_settitle(void);
    void cmd_sort(void);
    void cmd_system(void);
    void cmd_timer(void);
    void cmd_xmodem(void);

#endif

// Entries for the command table.
#if defined(INCLUDE_COMMAND_TABLE)

    { "AutoSave",   T_CMD,              0, cmd_autosave },
    { "Call",       T_CMD,              0, cmd_call     },
    { "Chdir",      T_CMD,              0, cmd_chdir    },
    { "Close",      T_CMD,              0, cmd_close    },
    { "Color",      T_CMD,              0, cmd_dummy    },
    { "Colour",     T_CMD,              0, cmd_dummy    },
    { "Copy",       T_CMD,              0, cmd_copy     },
    { "Console",    T_CMD,              0, cmd_console  },
    { "CSub",       T_CMD,              0, cmd_cfunction },
    { "End CSub",   T_CMD,              0, cmd_null     },
    { "Execute",    T_CMD,              0, cmd_execute  },
    { "Files",      T_CMD,              0, cmd_files    },
    { "Kill",       T_CMD,              0, cmd_kill     },
    { "Load",       T_CMD,              0, cmd_load     },
    { "Mid$(",      T_CMD | T_FUN,      0, cmd_mid      },
    { "Mkdir",      T_CMD,              0, cmd_mkdir    },
    { "Open",       T_CMD,              0, cmd_open     },
    { "Quit",       T_CMD,              0, cmd_quit     },
    { "Rename",     T_CMD,              0, cmd_rename   },
    { "Rmdir",      T_CMD,              0, cmd_rmdir    },
    { "Seek",       T_CMD,              0, cmd_seek     },
    { "SetTick",    T_CMD,              0, cmd_settick  },
    { "System",     T_CMD,              0, cmd_system   },
    { "Option",     T_CMD,              0, cmd_option   },
    { "Cat",        T_CMD,              0, cmd_inc      }, // TODO: alias of INC
    { "Cls",        T_CMD,              0, cmd_cls      },
    { "Edit",       T_CMD,              0, cmd_edit     },
    { "Font",       T_CMD,              0, cmd_dummy    },
    { "Inc",        T_CMD,              0, cmd_inc      },
    { "IReturn",    T_CMD,              0, cmd_ireturn  },
    { "LongString", T_CMD,              0, cmd_longstring },
    { "Pause",      T_CMD,              0, cmd_pause    },
    { "Timer",      T_CMD | T_FUN,      0, cmd_timer    },
    { "Cursor",     T_CMD,              0, cmd_cursor   },
    { "Math",       T_CMD,              0, cmd_math     },
    { "Memory",     T_CMD,              0, cmd_memory   },
    { "Mode",       T_CMD,              0, cmd_mode     },
    { "Page",       T_CMD,              0, cmd_dummy    },
    { "Play",       T_CMD,              0, cmd_dummy    },
    { "Poke",       T_CMD,              0, cmd_poke     },
    { "SetTitle",   T_CMD,              0, cmd_settitle },
    { "Sort",       T_CMD,              0, cmd_sort     },
    { "Text",       T_CMD,              0, cmd_dummy    },
    { "XModem",     T_CMD,              0, cmd_xmodem   },

#endif
