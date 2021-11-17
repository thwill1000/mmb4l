/***********************************************************************************************************************
MMBasic

MM_Misc.h

Include file that contains the globals and defines for Misc.c in MMBasic.
These are miscelaneous commands and functions that do not easily sit anywhere else.

Copyright 2011 - 2018 Geoff Graham.  All Rights Reserved.

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
// format:
//      void cmd_???(void)
//      void fun_???(void)
//      void op_???(void)

void cmd_option(void);
void cmd_pause(void);
void cmd_timer(void);
void cmd_ireturn(void);
void cmd_settick(void);
void cmd_watchdog(void);
void cmd_cpu(void);
void cmd_poke(void);

void fun_timer(void);
void fun_date(void);
void fun_time(void);
void fun_keydown(void);
void fun_restart(void);
void fun_peek(void);
void fun_epoch(void);
void fun_datetime(void);
void fun_day(void);

#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE

	{ "Option",	T_CMD,				0, cmd_option	},
	{ "Pause",	T_CMD,				0, cmd_pause	},
	{ "Timer",	T_CMD | T_FUN,                  0, cmd_timer	},
	{ "IReturn",	T_CMD,				0, cmd_ireturn 	},
	{ "Poke",	T_CMD,				0, cmd_poke	},
	{ "SetTick",	T_CMD,				0, cmd_settick 	},
	{ "WatchDog",	T_CMD,				0, cmd_watchdog	},
	{ "CPU",	T_CMD,				0, cmd_cpu 	},

#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

	{ "As",		T_NA,			0, op_invalid	},
	{ "Timer",	T_FNA | T_INT,		0, fun_timer	},
	{ "Date$",	T_FNA | T_STR,		0, fun_date	},
	{ "Peek(",	T_FUN  | T_INT,		0, fun_peek	},
	{ "Time$",	T_FNA | T_STR,		0, fun_time	},
	{ "Day$(",	T_FUN | T_STR,		0, fun_day	},
	{ "Epoch(",	T_FUN  | T_INT,		0, fun_epoch		},
	{ "DateTime$(",	T_FUN | T_STR,		0, fun_datetime		},
//	{ "MM.Watchdog",T_FNA | T_INT,		0, fun_restart	},

#endif


#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
    // General definitions used by other modules

    #ifndef MISC_HEADER
    #define MISC_HEADER

    extern void OtherOptions(void);

    extern char *InterruptReturn;
    extern int check_interrupt(void);
    extern char *GetIntAddress(char *p);

    // struct for the interrupt configuration
    #define T_LOHI   1
    #define T_HILO   2
    #define T_BOTH   3
    struct s_inttbl {
            int pin;                                   // the pin on which the interrupt is set
            int last;					// the last value of the pin (ie, hi or low)
            char *intp;					// pointer to the interrupt routine
            int lohi;                                  // trigger condition (T_LOHI, T_HILO, etc).
    };
    #define NBRINTERRUPTS	    10			// number of interrupts that can be set
    extern struct s_inttbl inttbl[NBRINTERRUPTS];

    extern int TickPeriod[NBRSETTICKS];
    extern volatile int TickTimer[NBRSETTICKS];
    extern char *TickInt[NBRSETTICKS];

    extern unsigned int ClockSpeed;
    extern unsigned int BusSpeed;
    extern int64_t starttime;


#endif
#endif
