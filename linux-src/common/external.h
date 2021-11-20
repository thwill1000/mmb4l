/***********************************************************************************************************************
MMBasic

External.h

Define the MMBasic commands for reading and writing to the digital and analog input/output pins

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

void cmd_setpin(void);
void cmd_pulse(void);

void cmd_pin(void);
void fun_pin(void);

void cmd_port(void);
void fun_port(void);

void cmd_ir(void);
void cmd_lcd(void);
void cmd_keypad(void);
void fun_distance(void);
void fun_pulsin(void);
void cmd_wave(void);
void cmd_dht22(void);

#endif



/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE
// the format is:
//    TEXT      	TYPE                P  FUNCTION TO CALL
// where type is always T_CMD
// and P is the precedence (which is only used for operators and not commands)

	{ "Pin(",		T_CMD | T_FUN,		0, cmd_pin          },
	{ "SetPin",		T_CMD,			0, cmd_setpin       },
	{ "Pulse",		T_CMD,			0, cmd_pulse        },
	{ "Port(",		T_CMD | T_FUN,		0, cmd_port	    },
	{ "IR",                 T_CMD,			0, cmd_ir           },
	{ "LCD",                T_CMD,			0, cmd_lcd          },
	{ "KeyPad",             T_CMD,			0, cmd_keypad       },
	{ "DHT22",              T_CMD,			0, cmd_dht22        },
	{ "HUMID",              T_CMD,			0, cmd_dht22        },
	{ "Bitstream",               T_CMD,			0, cmd_wave       },

#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE
// the format is:
//    TEXT      	TYPE                P  FUNCTION TO CALL
// where type is T_NA, T_FUN, T_FNA or T_OPER argumented by the types T_STR and/or T_NBR
// and P is the precedence (which is only used for operators)
	{ "Pin(",		T_FUN | T_NBR | T_INT,	0, fun_pin		},
	{ "Port(",		T_FUN | T_INT,		0, fun_port		},
	{ "Distance(",		T_FUN | T_NBR,		0, fun_distance		},
	{ "Pulsin(",		T_FUN | T_NBR,		0, fun_pulsin		},

#endif


#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
// General definitions used by other modules

#ifndef EXTERNAL_HEADER
#define EXTERNAL_HEADER

#define NBR_PULSE_SLOTS     5                       // number of concurrent pulse commands, each entry is 8 bytes


extern char *InterruptReturn;
extern int check_interrupt(void);
// lb extern void ClearExternalIO(void);

// lb copied from MM_Misc.h
    extern char *InterruptReturn;
    extern int check_interrupt(void);
    extern char *GetIntAddress(char *p);

    // struct for the interrupt configuration
    #define T_LOHI   1
    #define T_HILO   2
    #define T_BOTH   3
    struct s_inttbl {
            int pin;                                   // the pin on which the $
            int last;                                   // the last value of th$
            char *intp;                                 // pointer to the inter$
            int lohi;                                  // trigger condition (T_$
    };
    #define NBRINTERRUPTS           10                  // number of interrupts$
    extern struct s_inttbl inttbl[NBRINTERRUPTS];
// end of MM_Misc.h copy

/****************************************************************************************************************************
New, more portable, method of manipulating an I/O pin
*****************************************************************************************************************************/

// the basic functions
extern void PinSetBit(int pin, int offset);
extern int PinRead(int pin);

// some useful defines
#define PinOpenCollectorOn(x)   PinSetBit(x, ODCSET)
#define PinOpenCollectorOff(x)  PinSetBit(x, ODCCLR)
#define PinHigh(x)              PinSetBit(x, LATSET)
#define PinLow(x)               PinSetBit(x, LATCLR)
#define PinSetOutput(x)         PinSetBit(x, TRISCLR)
#define PinSetInput(x)          PinSetBit(x, TRISSET)

// Define the offsets from the PORT address
#define ANSEL               -8
#define ANSELCLR            -7
#define ANSELSET            -6
#define ANSELINV            -5
#define TRIS                -4
#define TRISCLR             -3
#define TRISSET             -2
#define TRISINV             -1
#define PORT                0
#define PORTCLR             1
#define PORTSET             2
#define PORTINV             3
#define LAT                 4
#define LATCLR              5
#define LATSET              6
#define LATINV              7
#define ODC                 8
#define ODCCLR              9
#define ODCSET              10
#define ODCINV              11
#define CNPU                12
#define CNPUCLR             13
#define CNPUSET             14
#define CNPUINV             15
#define CNPD                16
#define CNPDCLR             17
#define CNPDSET             18
#define CNPDINV             19
#define CNCON               20
#define CNCONCLR            21
#define CNCONSET            22
#define CNCONINV            23
#define CNEN                24
#define CNENCLR             25
#define CNENSET             26
#define CNENINV             27
#define CNSTAT              28
#define CNSTATCLR           29
#define CNSTATSET           30
#define CNSTATINV           31


#define EXT_NOT_CONFIG                          0
#define EXT_OE_OUT				1
#define EXT_DIG_IN				2
#define EXT_FREQ_IN				3
#define EXT_PER_IN				4
#define EXT_CNT_IN				5
#define EXT_INT_HI				6
#define EXT_INT_LO				7
#define EXT_DIG_OUT				8
#define EXT_OC_OUT				9
#define EXT_INT_BOTH				10
#define EXT_COM_RESERVED                        100                 // this pin is reserved and SETPIN and PIN cannot be used
#define EXT_BOOT_RESERVED                       101                 // this pin is reserved at bootup and cannot be used
#define EXT_DS18B20_RESERVED    102                 // this pin is reserved for DS18B20 and cannot be used

extern int ExtCurrentConfig[MAXNBRPINS + 1];
extern int ExtCurrentStatus[MAXNBRPINS + 1];
extern volatile int INT1Count, INT1Value, INT1InitTimer, INT1Timer;
extern volatile int INT2Count, INT2Value, INT2InitTimer, INT2Timer;
extern volatile int INT3Count, INT3Value, INT3InitTimer, INT3Timer;
extern volatile int INT4Count, INT4Value, INT4InitTimer, INT4Timer;

extern void initExtIO(void);
extern void ExtCfg(int pin, int cfg, int option) ;
extern void ExtSet(int pin, int val);
extern int ExtInp(int pin);
extern const struct s_PinDef PinDef[];
// for CheckPin() action can be set to:
#define CP_CHECKALL          0b0000     // abort with an error if invalid, in use or reserved
#define CP_NOABORT           0b0001     // the function will not abort with an error
#define CP_IGNORE_INUSE      0b0010     // the function will ignore pins that are in use (but not including reserved pins)
#define CP_IGNORE_RESERVED   0b0100     // the function will ignore reserved pins (EXT_COM_RESERVED and EXT_BOOT_RESERVED)
#define CP_IGNORE_BOOTRES    0b1000     // the function will ignore the boot reserved pins (EXT_BOOT_RESERVED)
extern int CheckPin(int pin, int action);

extern int InterruptUsed;

// IR related stuff
extern void *IrDev, *IrCmd;
extern char IrVarType, IrState, IrGotMsg;
extern int IrBits, IrCount;
extern char *IrInterrupt;
void IrInit(void);
void IrReset(void);
void IRSendSignal(int pin, int half_cycles);

// numpad declares
extern char *KeypadInterrupt;
int KeypadCheck(void);

#define IR_CLOSED                   0
#define IR_WAIT_START               1
#define IR_WAIT_START_END           2
#define SONY_WAIT_BIT_START         3
#define SONY_WAIT_BIT_END           4
#define NEC_WAIT_FIRST_BIT_START    5
#define NEC_WAIT_BIT_START          7
#define NEC_WAIT_BIT_END            8
#define elapsed             100//((1000 * TMR1) / (BusSpeed / 8000))


#endif
#endif

