/***********************************************************************************************************************
MMBasic

External.c

Handles reading and writing to the digital and analog input/output pins ising the SETPIN and PIN commands

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

#define _SUPPRESS_PLIB_WARNING                      // required for XC1.33  Later compiler versions will need PLIB to be installed
//#include "/home/pi/github/mmb4l-src/MMBasic/MMBasic_Includes.h"
//#include "MMBasic_Includes.h"
//#include "/home/pi/github/mmb4l-src/linux-src/Hardware_Includes.h"
//#include "/home/pi/github/mmb4l-src/linux-src/Configuration.h" 
#include "version.h"
#include "IOPorts.h"
//#include "MM_Misc.h"
#include "external.h"
#include <string.h> // lb from R-pi IOT in C
#include <stdio.h> // lb from R-pi IOT in C
#include <unistd.h> // lb from R-pi IOT in C
#include <fcntl.h> // lb from R-pi IOT in C
#include <sys/ioctl.h> // lb from R-pi IOT in C
#include <linux/gpio.h> // lb from R-pi IOT in C
#include "/usr/include/gpiod.h" // lb from libgpiod doc
int iRevrsgpio[26]; // gpio pin # to connector pin #
struct s_inttbl inttbl[NBRINTERRUPTS]; // LB from MM_Misc.c

void INT1Interrupt(int gpio, int level, uint32_t tick) ;
void INT2Interrupt(int gpio, int level, uint32_t tick) ;
void INT3Interrupt(int gpio, int level, uint32_t tick) ;
void INT4Interrupt(int gpio, int level, uint32_t tick) ;
int gpioWrite(unsigned gpio, unsigned level) ; // lb
int gpioRead(unsigned gpio); // lb 
int gpioSetMode(unsigned gpio, unsigned mode) ; // lb
int testgpiowrite(int, int);
int ExtCurrentConfig[MAXNBRPINS + 1];
int ExtCurrentStatus[MAXNBRPINS + 1];
volatile int INT1Count, INT1Value, INT1InitTimer, INT1Timer;
volatile int INT2Count, INT2Value, INT2InitTimer, INT2Timer;
volatile int INT3Count, INT3Value, INT3InitTimer, INT3Timer;
volatile int INT4Count, INT4Value, INT4InitTimer, INT4Timer;
volatile uint32_t INT1tick, INT2tick,INT3tick,INT4tick;
int nextIRlevel;
int InterruptUsed;
volatile uint32_t TickArray[4];
volatile uint32_t LevelArray[4];
volatile uint32_t nedge;
// struct gpiohandle_request req[27]; // lb for ioctl pin access by gpio pin #
struct gpiod_chip *chip; // = gpiod_chip_open_by_name("gpiochip0");
//struct gpiod_line_request_config gpioconfig; // lb reset/reused for each line 
struct gpiod_request_config *gpiolinereqconfig; // lb reset/reused for each line 
struct gpiod_line_config *gpiolineconfig; // lb reset/reused for each line 
struct gpiod_line_request *gpioline[27]; // lb for libgpiod V2 pin access by gpio pin #
int gpio_offsets[1]; // lb single line number to get for libgpiod
int gpioVal[1]; // lb single line value to get for libgpiod
int gpioflags; // lb reset/reused for each line
	
// struct s_PinDef {char gpio, char mode};
const struct s_PinDef PinDef[NBR_PINS_40CHIP + 1] = {                                                                // debug cannot work with the table in boot flash

    {  0, UNUSED },                                                 // pin 0
    {  0, UNUSED },                                                 // pin 1
    {  0, UNUSED },                                                 // pin 2
    {  2, DIGITAL_IN | DIGITAL_OUT },                               // pin 3
    {  0, UNUSED },                                                 // pin 4
    {  3, DIGITAL_IN | DIGITAL_OUT },                               // pin 5
    {  0, UNUSED },                                                 // pin 6
    {  4, DIGITAL_IN | DIGITAL_OUT },                               // pin 7
    { 14, DIGITAL_IN | DIGITAL_OUT },                               // pin 8
    {  0, UNUSED },                                                 // pin 9
    { 15, DIGITAL_IN | DIGITAL_OUT },                               // pin 10
    { 17, DIGITAL_IN | DIGITAL_OUT },                               // pin 11
//
    { 18, DIGITAL_IN | DIGITAL_OUT },                               // pin 12
    { 27, DIGITAL_IN | DIGITAL_OUT },                               // pin 13
    {  0, UNUSED },                                                 // pin 14
    { 22, DIGITAL_IN | DIGITAL_OUT },                               // pin 15
    { 23, DIGITAL_IN | DIGITAL_OUT },                               // pin 16
    {  0, UNUSED },                                                 // pin 17
    { 24, DIGITAL_IN | DIGITAL_OUT },                               // pin 18
    { 10, DIGITAL_IN | DIGITAL_OUT },                               // pin 19
    {  0, UNUSED },                                                 // pin 20
    {  9, DIGITAL_IN | DIGITAL_OUT },                               // pin 21
    { 25, DIGITAL_IN | DIGITAL_OUT },                               // pin 22
    { 11, DIGITAL_IN | DIGITAL_OUT },                               // pin 23
    {  8, DIGITAL_IN | DIGITAL_OUT },                               // pin 24
    {  0, UNUSED },                                                 // pin 25
    {  7, DIGITAL_IN | DIGITAL_OUT },                               // pin 26
    {  0, DIGITAL_IN | DIGITAL_OUT },                               // pin 27
    {  1, DIGITAL_IN | DIGITAL_OUT },                               // pin 28
    {  5, DIGITAL_IN | DIGITAL_OUT },                               // pin 29
    {  0, UNUSED },                                                 // pin 30
    {  6, DIGITAL_IN | DIGITAL_OUT },                               // pin 31
    { 12, DIGITAL_IN | DIGITAL_OUT },                               // pin 32
    { 13, DIGITAL_IN | DIGITAL_OUT },                               // pin 33
//
    { 0, UNUSED },                                                  // pin 34
    { 19, DIGITAL_IN | DIGITAL_OUT },                               // pin 35
    { 16, DIGITAL_IN | DIGITAL_OUT },                               // pin 36
    { 26, DIGITAL_IN | DIGITAL_OUT },                               // pin 37
    { 20, DIGITAL_IN | DIGITAL_OUT },                               // pin 38
    { 0, UNUSED },                                                  // pin 39
    { 21, DIGITAL_IN | DIGITAL_OUT },                               // pin 40
};


/*******************************************************************************************
External I/O related commands in MMBasic
========================================
These are the functions responsible for executing the ext I/O related  commands in MMBasic
They are supported by utility functions that are grouped at the end of this file

Each function is responsible for decoding a command
all function names are in the form cmd_xxxx() (for a basic command) or fun_xxxx() (for a
basic function) so, if you want to search for the function responsible for the LOCATE command
look for cmd_name

There are 4 items of information that are setup before the command is run.
All these are globals.

int cmdtoken	This is the token number of the command (some commands can handle multiple
				statement types and this helps them differentiate)

char *cmdline	This is the command line terminated with a zero char and trimmed of leading
				spaces.  It may exist anywhere in memory (or even ROM).

char *nextstmt	This is a pointer to the next statement to be executed.  The only thing a
				y=spi(1,2,3command can do with it is save it or change it to some other location.

char *CurrentLinePtr  This is read only and is set to NULL if the command is in immediate mode.

The only actions a command can do to change the program flow is to change nextstmt or
execute longjmp(mark, 1) if it wants to abort the program.

********************************************************************************************/


// this is invoked as a command (ie, pin(3) = 1)
// first get the argument then step over the closing bracket.  Search through the rest of the command line looking
// for the equals sign and step over it, evaluate the rest of the command and set the pin accordingly
void cmd_pin(void) {
	int pin, value;

//printf("cmd_pin: %s ",++cmdline); // lb "++" -- why?
//	pin = getinteger(cmdline);
	pin = getinteger(++cmdline); // lb "++" -- why?
        CheckPin(pin, CP_IGNORE_INUSE);
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Syntax");
	++cmdline;
	if(!*cmdline) error("Syntax");
	value = getinteger(cmdline);
//printf("%i %i\n",pin, value);
	ExtSet(pin, value);
}


// this is invoked as a function (ie, x = pin(3) )
void fun_pin(void) {

    int pin;

    pin = getinteger(ep);
    targ = T_INT;


    CheckPin(pin, CP_IGNORE_INUSE);
    switch(ExtCurrentConfig[pin]) {
        case EXT_DIG_IN:
        case EXT_CNT_IN:
        case EXT_INT_HI:
        case EXT_INT_LO:
        case EXT_INT_BOTH:
        case EXT_DIG_OUT:
        case EXT_OE_OUT:
        case EXT_OC_OUT:    iret = ExtInp(pin);
                            return;
        case EXT_PER_IN:	// if period measurement get the count and average it over the number of cycles
                            if(pin == INT1PIN) fret = (MMFLOAT)ExtInp(pin) / (MMFLOAT)INT1InitTimer /1000.0;
                            else if(pin == INT2PIN)  fret = (MMFLOAT)ExtInp(pin) / (MMFLOAT)INT2InitTimer /1000.0;
                            else if(pin == INT3PIN)  fret = (MMFLOAT)ExtInp(pin) / (MMFLOAT)INT3InitTimer /1000.0;
                            else if(pin == INT4PIN)  fret = (MMFLOAT)ExtInp(pin) / (MMFLOAT)INT4InitTimer /1000.0;
                            targ = T_NBR;
                            return;
        case EXT_FREQ_IN:	// if frequency measurement get the count and scale the reading   				
                            if(pin == INT1PIN) fret = (MMFLOAT)(ExtInp(pin) * 100) / INT1InitTimer;
                            else if(pin == INT2PIN)  fret = (MMFLOAT)(ExtInp(pin) * 100) / INT2InitTimer;
                            else if(pin == INT3PIN)  fret = (MMFLOAT)(ExtInp(pin) * 100) / INT3InitTimer;
                            else if(pin == INT4PIN)  fret = (MMFLOAT)(ExtInp(pin) * 100) / INT4InitTimer;
                            targ = T_NBR;
                            return;
        default:            error("Pin % not input", pin);
    }


}



// this is invoked as a command (ie, port(3, 8) = Value)
// first get the arguments then step over the closing bracket.  Search through the rest of the command line looking
// for the equals sign and step over it, evaluate the rest of the command and set the pins accordingly
void cmd_port(void) {
	int pin, nbr, value;
    int i;
	getargs(&cmdline, NBRPINS * 4, ",");

	if((argc & 0b11) != 0b11) error("Argument count");

    // step over the equals sign and get the value for the assignment
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Syntax");
	++cmdline;
	if(!*cmdline) error("Syntax");
	value = getinteger(cmdline);

    for(i = 0; i < argc; i += 4) {
        pin = getinteger(argv[i]);
        nbr = getint(argv[i + 2], 1, MAX_ARG_COUNT);

        while(nbr) {
            CheckPin(pin, CP_IGNORE_INUSE);
            if(!(ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT)) error("Pin % not output", pin);
            ExtSet(pin, value & 1);
            value >>= 1;
            nbr--;
            pin++;
        }
    }
}



// this is invoked as a function (ie, x = port(10,8) )
void fun_port(void) {
	int pin, nbr, i, value = 0;

	getargs(&ep, NBRPINS * 4, ",");
	if((argc & 0b11) != 0b11) error("Argument count");

    for(i = argc - 3; i >= 0; i -= 4) {
        pin = getinteger(argv[i]);
        nbr = getinteger(argv[i + 2]);

        if(nbr < 0 || pin <= 0) error("Invalid argument");
        pin += nbr - 1;                                             // we start by reading the most significant bit

        while(nbr) {
            CheckPin(pin, CP_IGNORE_INUSE);
            if(!(ExtCurrentConfig[pin] == EXT_DIG_IN || ExtCurrentConfig[pin] == EXT_INT_HI || ExtCurrentConfig[pin] == EXT_INT_LO || ExtCurrentConfig[pin] == EXT_INT_BOTH || ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT)) error("Pin % not input", pin);
            value <<= 1;
            value |= PinRead(pin);
            nbr--;
            pin--;
        }
    }

    iret = value;
    targ = T_INT;
}

void cmd_setpin(void) {
	int i, pin, value, option = 0, ret; // ret by lb
	getargs(&cmdline, 7, ",");
	if(argc%2 == 0 || argc < 3) error("Argument count");
// lb TESTING
/*
    if(fdgpio==0) {
      fdgpio = open("/dev/gpiochip0", O_RDONLY);
      if(fdgpio<0) {
        for(i=0;i<27;i++) req[i].fd=-1; // no /dev/gpiochip0
        error("Missing /dev/gpiochip0 for pin I/O");
      } 
      else { // lb good handle for gpiochip0
        for(i=0;i<27;i++) iRevrsgpio[i]=-1;
        for(i=1;i<40;i++) // set up connector pin # for gpio pin #
          if(PinDef[i].mode!=UNUSED) iRevrsgpio[PinDef[i].gpio]=i;
      }
    }
*/
// using libgpiod
  if(chip==NULL) { // never opened
    chip = gpiod_chip_open("/dev/gpiochip0");
    if(!chip) {
      error("Missing /dev/gpiochip0 for pin I/O; MMB4L Terminated");
    }
// printf("chip /dev/gpiochip0 opened successfully\n");
  }

// lb END TESTING

    if(checkstring(argv[2], "OFF") || checkstring(argv[2], "0"))
        value = EXT_NOT_CONFIG;
    else if(checkstring(argv[2], "DIN"))
        value = EXT_DIG_IN;
    else if(checkstring(argv[2], "FIN"))
        value = EXT_FREQ_IN;
    else if(checkstring(argv[2], "PIN"))
        value = EXT_PER_IN;
    else if(checkstring(argv[2], "CIN"))
        value = EXT_CNT_IN;
    else if(checkstring(argv[2], "INTH"))
        value = EXT_INT_HI;
    else if(checkstring(argv[2], "INTL"))
        value = EXT_INT_LO;
    else if(checkstring(argv[2], "DOUT"))
        value = EXT_DIG_OUT;
    else if(checkstring(argv[2], "INTB"))
        value = EXT_INT_BOTH;
    else
        value = getint(argv[2], 1, 9);

    // check for any options
    switch(value) {
        case EXT_DIG_IN:    if(argc == 5) {
                                if(checkstring(argv[4], "PULLUP")) option = CNPUSET;
                                else if(checkstring(argv[4], "PULLDOWN")) option = CNPDSET;
                                else error("Invalid option");
                            } else
                                option = 0;
                            break;
        case EXT_INT_HI:
        case EXT_INT_LO:
        case EXT_INT_BOTH:  if(argc == 7) {
                                if(checkstring(argv[6], "PULLUP")) option = CNPUSET;
                                else if(checkstring(argv[6], "PULLDOWN")) option = CNPDSET;
                                else error("Invalid option");
                            } else
                                option = 0;
                            break;
        case EXT_FREQ_IN:   if(argc == 5)
                                option = getint((argv[4]), 10, 100000)/10;
                            else
                                option = 100;
                            break;
        case EXT_PER_IN:   if(argc == 5)
                                option = getint((argv[4]), 1, 10000);
                            else
                                option = 1;
                            break;
        case EXT_DIG_OUT:   if(argc == 5) {
                                if(checkstring(argv[4], "OC"))
                                    value = EXT_OC_OUT;
                                else if(checkstring(argv[4], "OE"))
                                    value = EXT_OE_OUT;
                                else
                                    error("Invalid option");
                            }
                            break;
        default:            if(argc > 3) error("Unexpected text");
    }

	pin = getinteger(argv[0]);


        CheckPin(pin, CP_IGNORE_INUSE);
        ExtCfg(pin, value, option);


	if(value == EXT_INT_HI || value == EXT_INT_LO || value == EXT_INT_BOTH) {
		// we need to set up a software interrupt
            if(argc < 5) error("Argument count");
            for(i = 0; i < NBRINTERRUPTS; i++) if(inttbl[i].pin == 0) break;
            if(i >= NBRINTERRUPTS) error("Too many interrupts");
            inttbl[i].pin = pin;
            inttbl[i].intp = GetIntAddress(argv[4]);					// get the interrupt routine's location
            inttbl[i].last = ExtInp(pin);								// save the current pin value for the first test
            switch(value) {                                             // and set trigger polarity
                case EXT_INT_HI:    inttbl[i].lohi = T_LOHI; break;
                case EXT_INT_LO:    inttbl[i].lohi = T_HILO; break;
                case EXT_INT_BOTH:  inttbl[i].lohi = T_BOTH; break;
            }
            InterruptUsed = true;
	}
}

/* lb

void cmd_pulse(void) {
    int pin, i, x, y, state;
    MMFLOAT f;

    getargs(&cmdline, 3, ",");
    if(argc != 3) error("Argument count");
    pin = getinteger(argv[0]);
    CheckPin(pin, CP_IGNORE_INUSE);
    if(!(ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT)) error("Pin is not an output");

    f = getnumber(argv[2]);                                         // get the pulse width
    if(f < 0) error("Number out of bounds");
    x = (int)(f*1000.0);                                                          // get the integer portion (in 100ths second)
    y = x % 10000;                         // get the fractional portion (in uSec)
    x=x/10000;
    if(x == 0 && y == 0) return;                                    // silently ignore a zero pulse width

    state = ((ExtCurrentConfig[pin] & 2) >> 1);       // get the current state of the output
    gpioSetTimerFunc(0, 10, NULL);

    PinSetBit(pin, state ? LATCLR : LATSET);                    // starting edge of the pulse
    if(y!=0) uSec(y);
    if(x==0) PinSetBit(pin, state ? LATSET : LATCLR);                    // finishing edge
    else {
    for(i = 0; i < NBR_PULSE_SLOTS; i++){                           // search looking to see if the pin is in use
        if(PulseCnt[i] != 0 && PulsePin[i] == pin) {
            PulseCnt[i] = x;                                        // and if the pin is in use, set its time to the new setting or reset if the user wants to terminate
           if(x == 0) PinSetBit(PulsePin[i], PulseDirection[i] ? LATSET : LATCLR);
            gpioSetTimerFunc(0, 10, T4Interrupt);
            return;
        }
    }

        for(i = 0; i < NBR_PULSE_SLOTS; i++) if(PulseCnt[i] == 0) break;                                 // find a spare slot

        if(i >= NBR_PULSE_SLOTS) error("Too many concurrent PULSE commands");

        PinSetBit(pin, state ? LATCLR : LATSET);                        // starting edge of the pulse
        PulsePin[i] = pin;                                              // save the details
        PulseDirection[i] = state;
        PulseCnt[i] = x;
        PulseActive = true;
    }
    gpioSetTimerFunc(0, 10, T4Interrupt);
}

void callback(int gpio, int level, uint32_t tick)
{
    if(nedge<3){
        LevelArray[nedge]=level;
        TickArray[nedge++]=tick;
    }
}

void fun_pulsin(void) {
    int pin, polarity;
    unsigned int t1, t2;
    int32_t diffTick;
    getargs(&ep, 7, ",");
    if((argc &1) != 1 || argc < 3) error("Argument count");
    pin = getinteger(argv[0]);
    CheckPin(pin, CP_IGNORE_INUSE);
    if(ExtCurrentConfig[pin] != EXT_DIG_IN) error("Pin is not an input");
    polarity = getinteger(argv[2]);

    t1 = t2 = 100000;                                               // default timeout is 100mS
    if(argc >= 5) t1 = t2 = getint(argv[4], 5, 1000000);
    if(argc == 7) t2 = getint(argv[6], 5, 1000000);
    fret = -1;                                                      // in anticipation of a timeout
    nedge=0;
    gofifo
    uSec(100);
    gpioSetAlertFunc(PinDef[pin].gpio, callback); // callback when GPIO changes state 
    uSecActive(t1+t2);
    while(nedge<2 && uSecActive(0));
    gpioSetAlertFunc(PinDef[pin].gpio, NULL); // stop callback 
    if(nedge>=2){
        if(polarity){
            if(LevelArray[0]){
                diffTick=TickArray[1]-TickArray[0];
            } else {
                diffTick=TickArray[2]-TickArray[1];
            }
        } else {
            if(!LevelArray[0]){
                diffTick=TickArray[1]-TickArray[0];
            } else {
                diffTick=TickArray[2]-TickArray[1];
            }
        }
        fret = (float)diffTick;
    } 
    gonormal
    targ = T_NBR;
}

*/ // lb

/****************************************************************************************************************************
IR routines
*****************************************************************************************************************************/

/* lb
void cmd_ir(void) {
    char *p;
    int i, pin, dev, cmd;
    if(checkstring(cmdline, "CLOSE")) {
        IrState = IR_CLOSED;
        gpioSetAlertFunc(PinDef[WAKEUP_PIN].gpio,NULL);
        IrInterrupt = NULL;
        ExtCfg(WAKEUP_PIN, EXT_NOT_CONFIG, 0);
    } else if((p = checkstring(cmdline, "SEND"))) {
        getargs(&p, 5, ",");
        pin = getinteger(argv[0]);
        dev = getint(argv[2], 0, 0b11111);
        cmd = getint(argv[4], 0, 0b1111111);
        CheckPin(pin, CP_CHECKALL);
        ExtCfg(pin, EXT_DIG_OUT, 0);
        cmd = (dev << 7) | cmd;
        IRSendSignal(pin, 186);
        for(i = 0; i < 12; i++) {
            uSec(600);
            if(cmd & 1)
                IRSendSignal(pin, 92);
            else
                IRSendSignal(pin, 46);
            cmd >>= 1;
        }
        ExtCfg(pin, EXT_NOT_CONFIG, 0);
    } else {
        getargs(&cmdline, 5, ",");
        if(IrState != IR_CLOSED) error("Already open");
        if(argc%2 == 0 || argc == 0) error("Argument count");
        IrVarType = 0;
        IrDev = findvar(argv[0], V_FIND);
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        if(vartbl[VarIndex].type & T_STR)  error("Invalid variable");
        if(vartbl[VarIndex].type & T_NBR) IrVarType |= 0b01;
        IrCmd = findvar(argv[2], V_FIND);
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        if(vartbl[VarIndex].type & T_STR)  error("Invalid variable");
        if(vartbl[VarIndex].type & T_NBR) IrVarType |= 0b10;
        InterruptUsed = true;
        IrInterrupt = GetIntAddress(argv[4]);							// get the interrupt location
        IrInit();
    }
}
#define fallingedge 0
#define risingedge 1

void IrInit(void) {
    TMR1 = (uint32_t)gpioTick();
    if(ExtCurrentConfig[WAKEUP_PIN] >= EXT_COM_RESERVED)  error("Pin is in use");
    ExtCfg(WAKEUP_PIN, EXT_DIG_IN, CNPUSET);
    ExtCfg(WAKEUP_PIN, EXT_COM_RESERVED, 0);
    IrReset();
}



void IrReset(void) {
    gpioSetAlertFunc(PinDef[WAKEUP_PIN].gpio,INT4Interrupt);
    IrState = IR_WAIT_START;
    nextIRlevel=fallingedge;
    IrCount = 0;
    TMR1 = (uint32_t)gpioTick();
}


// this modulates (at about 38KHz) the IR beam for transmit
// half_cycles is the number of half cycles to send.  ie, 186 is about 2.4mSec
void IRSendSignal(int pin, int half_cycles) {
    while(half_cycles--) {
        PinSetBit(pin, LATINV);
        uSec(13);
    }
}


*/ // lb


/****************************************************************************************************************************
 The LCD command
*****************************************************************************************************************************/

/* lb
static char lcd_pins[6];



void LCD_Nibble(int Data, int Flag, int Wait_uSec);
void LCD_Byte(int Data, int Flag, int Wait_uSec);
void LcdPinSet(int pin, int val);

void cmd_lcd(void)
 {
    char *p;
    int i, j;

    if((p = checkstring(cmdline, "INIT"))) {
        getargs(&p, 11, ",");
        if(argc != 11) error("Argument count");
        if(*lcd_pins) error("Already open");
        for(i = 0; i < 6; i++) {
            lcd_pins[i] = getinteger(argv[i * 2]);
            CheckPin(lcd_pins[i], CP_CHECKALL);
            ExtCfg(lcd_pins[i], EXT_DIG_OUT, 0);
            ExtCfg(lcd_pins[i], EXT_COM_RESERVED, 0);
        }
        LCD_Nibble(0b0011, 0, 5000);                                // reset
        LCD_Nibble(0b0011, 0, 5000);                                // reset
        LCD_Nibble(0b0011, 0, 5000);                                // reset
        LCD_Nibble(0b0010, 0, 2000);                                // 4 bit mode
        LCD_Byte(0b00101100, 0, 600);                               // 4 bits, 2 lines
        LCD_Byte(0b00001100, 0, 600);                               // display on, no cursor
        LCD_Byte(0b00000110, 0, 600);                               // increment on write
        LCD_Byte(0b00000001, 0, 3000);                              // clear the display
        return;
    }

    if(!*lcd_pins) error("Not open");
    if(checkstring(cmdline, "CLOSE")) {
        for(i = 0; i < 6; i++) {
			ExtCfg(lcd_pins[i], EXT_NOT_CONFIG, 0);					// all set to unconfigured
            *lcd_pins = 0;
        }
    } else if((p = checkstring(cmdline, "CLEAR"))) {                // clear the display
        LCD_Byte(0b00000001, 0, 3000);
    } else if((p = checkstring(cmdline, "CMD")) || (p = checkstring(cmdline, "DATA"))) { // send a command or data
        getargs(&p, MAX_ARG_COUNT * 2, ",");
        for(i = 0; i < argc; i += 2) {
            j = getint(argv[i], 0, 255);
            LCD_Byte(j, toupper(*cmdline) == 'D', 0);
        }
    } else {
        const char linestart[4] = {0, 64, 20, 84};
        int center, pos;

        getargs(&cmdline, 5, ",");
        if(argc != 5) error("Argument count");
        i = getint(argv[0], 1, 4);
        pos = 1;
        if(checkstring(argv[2], "C8"))
            center = 8;
        else if(checkstring(argv[2], "C16"))
            center = 16;
        else if(checkstring(argv[2], "C20"))
            center = 20;
        else if(checkstring(argv[2], "C40"))
            center = 40;
        else {
            center = 0;
            pos = getint(argv[2], 1, 256);
        }
        p = getstring(argv[4]);                                     // returns an MMBasic string
        i = 128 + linestart[i - 1] + (pos - 1);
        LCD_Byte(i, 0, 600);
        for(j = 0; j < (center - *p) / 2; j++) {
            LCD_Byte(' ', 1, 0);
        }
        for(i = 1; i <= *p; i++) {
            LCD_Byte(p[i], 1, 0);
            j++;
        }
        for(; j < center; j++) {
            LCD_Byte(' ', 1, 0);
        }
    }
}



void LCD_Nibble(int Data, int Flag, int Wait_uSec) {
    int i;
    LcdPinSet(lcd_pins[4], Flag);
    for(i = 0; i < 4; i++)
        LcdPinSet(lcd_pins[i], (Data >> i) & 1);
    LcdPinSet(lcd_pins[5], 1); uSec(250); LcdPinSet(lcd_pins[5], 0);
    if(Wait_uSec)
        uSec(Wait_uSec);
    else
        uSec(250);
}


void LCD_Byte(int Data, int Flag, int Wait_uSec) {
    LCD_Nibble(Data/16, Flag, 0);
    LCD_Nibble(Data, Flag, Wait_uSec);
}


void LcdPinSet(int pin, int val) {
    PinSetBit(pin, val ? LATSET : LATCLR);
}


*/ // lb

/****************************************************************************************************************************
 The DISTANCE function
*****************************************************************************************************************************/

/* // lb
void fun_distance(void) {
    int trig, echo, diffTick;
    uint32_t startTick, endTick;
    getargs(&ep, 3, ",");
    if((argc &1) != 1) error("Argument count");
    trig = getinteger(argv[0]);
    if(argc == 3)
        echo = getinteger(argv[2]);
    else
        echo = trig;                                                // they are the same if it is a 3-pin device
    CheckPin(trig, CP_CHECKALL);
    CheckPin(echo, CP_CHECKALL);
    gofifo
    ExtCfg(echo, EXT_DIG_IN, CNPUSET);                              // setup the echo input
    PinSetBit(trig, LATCLR);                                        // trigger output must start low
    ExtCfg(trig, EXT_DIG_OUT, 0);                                   // setup the trigger output
    PinSetBit(trig, LATSET); uSec(20); PinSetBit(trig, LATCLR);     // pulse the trigger
    uSec(50);
    ExtCfg(echo, EXT_DIG_IN, CNPUSET);                              // this is in case the sensor is a 3-pin type
    uSec(50);
    PauseTimer = 0;                                                 // this is our timeout
    while(PinRead(echo)) if(PauseTimer > 50) { fret = -2; goto exitfun; } // wait for the acknowledgement pulse start
    while(!PinRead(echo)) if(PauseTimer > 100) { fret = -2; goto exitfun;}// then its end
    startTick = gpioTick();
    PauseTimer = 0;
    while(PinRead(echo)) {                                          // now wait for the echo pulse
        if(PauseTimer > 32) {                                       // timeout is 32mS
            fret = -1;
            goto exitfun;
        }
    }
    endTick = gpioTick();
    diffTick = endTick - startTick;
    // we have the echo, convert the time to centimeters
    fret = (float) diffTick * 0.172;
    exitfun:
    gonormal
    targ = T_NBR;
    ExtCfg(trig, EXT_NOT_CONFIG, 0);
    ExtCfg(echo, EXT_NOT_CONFIG, 0);
}


*/ // lb

/****************************************************************************************************************************
 The KEYPAD command
*****************************************************************************************************************************/

/* // lb
static char keypad_pins[8];
float *KeypadVar;
char *KeypadInterrupt = NULL;
void KeypadClose(void);

void cmd_keypad(void) {
    int i, j;

    if(checkstring(cmdline, "CLOSE"))
        KeypadClose();
    else {
        getargs(&cmdline, 19, ",");
        if(argc%2 == 0 || argc < 17) error("Argument count");
        if(KeypadInterrupt != NULL) error("Already open");
        KeypadVar = findvar(argv[0], V_FIND);
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        if(!(vartbl[VarIndex].type & T_NBR)) error("Floating point variable required");
        InterruptUsed = true;
        for(i = 0; i < 8; i++) {
            if(i == 7 && argc < 19) {
                keypad_pins[i] = 0;
                break;
            }
            j = getinteger(argv[(i + 2) * 2]);
            CheckPin(j, CP_CHECKALL);
            if(i < 4) {
                ExtCfg(j, EXT_DIG_IN, CNPUSET);
            } else {
                ExtCfg(j, EXT_OC_OUT, 0);
                PinSetBit(j, LATSET);
            }
            ExtCfg(j, EXT_COM_RESERVED, 0);
            keypad_pins[i] = j;
        }
        KeypadInterrupt = GetIntAddress(argv[2]);					// get the interrupt location
    }
}


void KeypadClose(void) {
    int i;
    if(KeypadInterrupt == NULL) return;
    for(i = 0; i < 8; i++) {
        if(keypad_pins[i]) {
            ExtCfg(keypad_pins[i], EXT_NOT_CONFIG, 0);				// all set to unconfigured
        }
    }
    KeypadInterrupt = NULL;
}


int KeypadCheck(void) {
    static char count = 0, keydown = false;
    int i, j;
    const char PadLookup[16] = { 1, 2, 3, 20, 4, 5, 6, 21, 7, 8, 9, 22, 10, 0, 11, 23 };

    if(count++ % 64) return false;                                  // only check every 64 loops through the interrupt processor

    for(j = 4; j < 8; j++) {                                        // j controls the pull down pins
        if(keypad_pins[j]) {                                        // we might just have 3 pull down pins
            PinSetBit(keypad_pins[j], LATCLR);                      // pull it low
            for(i = 0; i < 4; i++) {                                // i is the row sense inputs
                if(PinRead(keypad_pins[i]) == 0) {                  // if it is low we have found a keypress
                    if(keydown) goto exitcheck;                     // we have already reported this, so just exit
                    uSec(40 * 1000);                                // wait 40mS and check again
                    if(PinRead(keypad_pins[i]) != 0) goto exitcheck;// must be contact bounce if it is now high
                    *KeypadVar = PadLookup[(i << 2) | (j - 4)];     // lookup the key value and set the variable
                    PinSetBit(keypad_pins[j], LATSET);
                    keydown = true;                                 // record that we know that the key is down
                    return true;                                    // and tell the interrupt processor that we are good to go
                }
            }
            PinSetBit(keypad_pins[j], LATSET);                      // wasn't this pin, clear the pulldown
        }
    }
    keydown = false;                                                // no key down, record the fact
    return false;

exitcheck:
    PinSetBit(keypad_pins[j], LATSET);
    return false;
}

*/ // lb

/****************************************************************************************************************************
 The DHT22 function
*****************************************************************************************************************************/

/* // lb
void cmd_dht22(void) {
    int pin;
    int64_t r;
    int i;
    uint32_t startTick, endTick;
    int diffTick;
    MMFLOAT *temp, *humid;
    
    getargs(&cmdline, 5, ",");
    if(argc != 5) error("Incorrect number of arguments");
    
    // get the two variables
	temp = findvar(argv[2], V_FIND);
	if(!(vartbl[VarIndex].type & T_NBR)) error("Invalid variable");
	humid = findvar(argv[4], V_FIND);
	if(!(vartbl[VarIndex].type & T_NBR)) error("Invalid variable");

    // get the pin number and set it up
    pin = getinteger(argv[0]);
    CheckPin(pin, CP_CHECKALL);
    gofifo
    PinSetBit(pin, LATSET);
    ExtCfg(pin, EXT_OC_OUT, 0);

    // pulse the pin low for 1mS
    PinSetBit(pin, LATCLR);
    uSec(1000);
    PinSetBit(pin, LATSET);

    PinSetBit(pin, CNPUSET);
    
    uSecActive(40*400);
    while(PinRead(pin))  if(!uSecActive(0)) goto error_exit;
    while(!PinRead(pin)) if(!uSecActive(0)) goto error_exit;
    while(PinRead(pin))  if(!uSecActive(0)) goto error_exit;
    // now we wait for the pin to go high and measure how long it stays high (> 50uS is a one bit, < 50uS is a zero bit)
    for(r = i = 0; i < 40; i++) {
        while(!PinRead(pin)) if(!uSecActive(0)) goto error_exit;
        startTick = gpioTick();
        while(PinRead(pin))  if(!uSecActive(0)) goto error_exit;
        endTick = gpioTick();
        diffTick = endTick - startTick;
        r <<= 1;
        r |= (diffTick>50);
    }

    // we have all 40 bits
    // first validate against the checksum
    if( ( ( ((r >> 8) & 0xff) + ((r >> 16) & 0xff) + ((r >> 24) & 0xff) + ((r >> 32) & 0xff) ) & 0xff) != (r & 0xff)) goto error_exit;                                           // returning temperature
    *temp = (MMFLOAT)((r >> 8) &0x7fff) / 10.0;                       // get the temperature
    if((r >> 8) &0x8000) *temp = -*temp;                            // the top bit is the sign
    *humid = (MMFLOAT)(r >> 24) / 10.0;                               // get the humidity
    goto normal_exit;

error_exit:
    *temp = *humid = 1000.0;                                        // an obviously incorrect reading

normal_exit:
    ExtCfg(pin, EXT_NOT_CONFIG, 0);
    gonormal
 }

*/ // lb

/*******************************************************************************************
********************************************************************************************

Utility routines for the external I/O commands and functions in MMBasic

********************************************************************************************
********************************************************************************************/

/* // lb
void ClearExternalIO(void) {
    int i;

    if(com1 && Option.SERIAL_CONSOLE !=1){
        com1 = false;
        ExtCfg(COM1_RX_PIN, EXT_NOT_CONFIG, 0);
        ExtCfg(COM1_TX_PIN, EXT_NOT_CONFIG, 0);
        gpioSerialReadClose(PinDef[COM1_RX_PIN].gpio);
    }    
    if(com2 && Option.SERIAL_CONSOLE !=2){
        com2 = false;
        ExtCfg(COM2_RX_PIN, EXT_NOT_CONFIG, 0);
        ExtCfg(COM2_TX_PIN, EXT_NOT_CONFIG, 0);
        gpioSerialReadClose(PinDef[COM2_RX_PIN].gpio);
    }
    SPIClose();                                                    // close the SPI
    ResetDisplay();
    if(Option.I2C_Data  && !Option.ADC)i2c_disable();                                                  // close I2C
    IrState = IR_CLOSED;
    IrInterrupt = NULL;
    IrGotMsg = false;
    KeypadInterrupt = NULL;
    mmOWvalue = 0;
    *lcd_pins = 0;                                                  // close the LCD

    ds18b20Timer = -1;                                              // turn off the ds18b20 timer

	for(i = 1; i < NBRPINS + 1; i++) {
		if(CheckPin(i, CP_NOABORT | CP_IGNORE_INUSE | CP_IGNORE_RESERVED )) {    // don't reset invalid or boot reserved pins
			ExtCfg(i, EXT_NOT_CONFIG, 0);	
                        PWMClose(i);// all set to unconfigured
		}
	}

	for(i = 0; i < NBRINTERRUPTS; i++) {
		inttbl[i].pin = 0;                                          // disable all interrupts
	}
	InterruptReturn = NULL;
	InterruptUsed = false;
    OnKeyGOSUB = NULL;

    for(i = 0; i < NBRSETTICKS; i++) TickInt[i] = NULL;

	for(i = 0; i < NBR_PULSE_SLOTS; i++) PulseCnt[i] = 0;           // disable any pending pulse commands
    PulseActive = false;
}



/****************************************************************************************************************************
Configure an I/O pin
*****************************************************************************************************************************/

void ExtCfg(int pin, int cfg, int option) {
	int i, tris,  oc;
    CheckPin(pin, CP_IGNORE_INUSE | CP_IGNORE_RESERVED);

    if(cfg >= EXT_COM_RESERVED) {
        ExtCurrentConfig[pin] = cfg;                                // don't do anything except set the config type
        return;
    }

	// make sure that interrupts are disabled in case we are changing from an interrupt input
// lb    gpioSetAlertFunc(PinDef[pin].gpio,NULL);//	if(pin == INT1PIN) ConfigINT1(EXT_INT_PRI_2 | RISING_EDGE_INT | EXT_INT_DISABLE);

    // make sure any pullups/pulldowns are removed in case we are changing from a digital input
    if(ExtCurrentStatus[pin] & 8){
        PinSetBit(pin, CNPUCLR);  PinSetBit(pin, CNPDCLR);
        ExtCurrentStatus[pin]&=7;
    }
        ExtCurrentStatus[pin]=1;
	for(i = 0; i < NBRINTERRUPTS; i++)
        if(inttbl[i].pin == pin)
            inttbl[i].pin = 0;                                           // start off by disable a software interrupt (if set) on this pin

	switch(cfg) {
		case EXT_NOT_CONFIG:	tris = 1; oc = 0;
                PinSetBit(pin,CNPDSET);
                break;


   		case EXT_FREQ_IN:		// same as counting, so fall through
		case EXT_PER_IN:		// same as counting, so fall through
		case EXT_CNT_IN:        if(pin == INT1PIN) {
// lb                                            gpioSetAlertFunc(PinDef[pin].gpio,INT1Interrupt);
                                            INT1tick = INT1Count = INT1Value = 0;
                                            INT1Timer = INT1InitTimer = option;  // only used for frequency and period measurement
                                            tris = 1;  oc = 0;
                                            break;
					}
					if(pin == INT2PIN) {
// lb                                            gpioSetAlertFunc(PinDef[pin].gpio,INT2Interrupt);
                                            INT2tick = INT2Count = INT2Value = 0;
                                            INT2Timer = INT2InitTimer = option;  // only used for frequency and period measurement
                                            tris = 1;  oc = 0;
                                            break;
					}
					if(pin == INT3PIN) {
// lb                                            gpioSetAlertFunc(PinDef[pin].gpio,INT3Interrupt);
                                            INT3tick = INT3Count = INT3Value = 0;
                                            INT3Timer = INT3InitTimer = option;  // only used for frequency and period measurement
                                            tris = 1; oc = 0;
                                            break;
					}
					if(pin == INT4PIN) {
// lb                                            gpioSetAlertFunc(PinDef[pin].gpio,INT4Interrupt);
                                            INT4tick = INT4Count = INT4Value = 0;
                                            INT4Timer = INT4InitTimer = option;  // only used for frequency and period measurement
                                            tris = 1;  oc = 0;
                                            break;
					}
					error("Invalid configuration");		// not an interrupt enabled pin
					return;

		case EXT_INT_LO:											// same as digital input, so fall through
		case EXT_INT_HI:											// same as digital input, so fall through
		case EXT_INT_BOTH:											// same as digital input, so fall through
		case EXT_DIG_IN:	if(!(PinDef[pin].mode & DIGITAL_IN)) error("Invalid configuration");
                                        if(option) PinSetBit(pin, option);
		                        tris = 1;  oc = 0;
					break;

		case EXT_DIG_OUT:	if(!(PinDef[pin].mode & DIGITAL_OUT)) error("Invalid configuration");
		                        tris = 0;  oc = 0;
					break;

		case EXT_OC_OUT:	if(!(PinDef[pin].mode & DIGITAL_OUT)) error("Invalid configuration");
					tris = 0;  oc = 1;
					break;
		case EXT_OE_OUT:	if(!(PinDef[pin].mode & DIGITAL_OUT)) error("Invalid configuration");
					tris = 0;  oc = 0;
					break;

	default:		error("Invalid configuration");
		                        return;
	}

	ExtCurrentConfig[pin] = cfg;
	PinSetBit(pin, tris ? TRISSET : TRISCLR);                       // if tris = 1 then it is an input
	if(cfg == EXT_NOT_CONFIG){ExtCurrentStatus[pin]=1;}						// set the default output to low
	PinSetBit(pin, oc ? ODCSET : ODCCLR);                           // if oc = 1 then open collector is turned ON
}



/****************************************************************************************************************************
Set the output of a digital I/O pin
*****************************************************************************************************************************/


void ExtSet(int pin, int val){

//printf("ExtSet: %i %i %i %i\n",pin,val,ExtCurrentConfig[pin],EXT_DIG_OUT);
    if(ExtCurrentConfig[pin] == EXT_NOT_CONFIG || ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT) {
        PinSetBit(pin, val ? LATSET : LATCLR);
    }
    else if(ExtCurrentConfig[pin] == EXT_CNT_IN){
        if(pin == INT1PIN)INT1Count=val;
        if(pin == INT2PIN)INT2Count=val;
        if(pin == INT3PIN)INT3Count=val;
        if(pin == INT4PIN)INT4Count=val;
    } else
        error("Pin is not an output");
}



/****************************************************************************************************************************
Get the value of an I/O pin and returns it
For digital returns 0 if low or 1 if high
For analog returns the reading as a 10 bit number with 0b1111111111 = 3.3V
*****************************************************************************************************************************/

int ExtInp(int pin){
 
	// read from a digital input
	if(ExtCurrentConfig[pin] == EXT_DIG_IN || ExtCurrentConfig[pin] == EXT_INT_HI || ExtCurrentConfig[pin] == EXT_INT_LO || ExtCurrentConfig[pin] == EXT_INT_BOTH || ExtCurrentConfig[pin] == EXT_OC_OUT || ExtCurrentConfig[pin] == EXT_OE_OUT|| ExtCurrentConfig[pin] == EXT_DIG_OUT) {
        return  PinRead(pin);
	}


	// read from a frequency/period input
	if(ExtCurrentConfig[pin] == EXT_FREQ_IN || ExtCurrentConfig[pin] == EXT_PER_IN) {
		// select input channel
        if(pin == INT1PIN) return INT1Value;
        if(pin == INT2PIN) return INT2Value;
        if(pin == INT3PIN) return INT3Value;
        if(pin == INT4PIN) return INT4Value;
	}

	// read from a counter input
	if(ExtCurrentConfig[pin] == EXT_CNT_IN) {
		// select input channel
        if(pin == INT1PIN) return INT1Count;
        if(pin == INT2PIN) return INT2Count;
        if(pin == INT3PIN) return INT3Count;
        if(pin == INT4PIN) return INT4Count;
	}
	return 0;
}



/****************************************************************************************************************************
New, more portable, method of manipulating an I/O pin
*****************************************************************************************************************************/

// set or clear a bit in the pin's sfr register
void PinSetBit(int pin, int offset) {
  unsigned i; // lb for input pin with optional pullup/down
  int gpio; //lb  gpio#, not connector #
  gpioflags|=GPIO_V2_LINE_FLAG_OUTPUT; // lb default to output
    switch(offset){
        case LATSET:// Set GPIO high.
            ExtCurrentStatus[pin]|=2; //remember the current status
            break;
        case LATCLR:// Set GPIO low
            ExtCurrentStatus[pin]&=5; //remember the current status
            break;
        case LATINV:// Set GPIO low
            ExtCurrentStatus[pin]^=2; //remember the current status
            break;
        case TRISSET:// Set GPIO high.
            ExtCurrentStatus[pin]|=1; //remember the current status
            break;
        case TRISCLR:// Set GPIO high.
            ExtCurrentStatus[pin]&=6; //remember the current status
            break;
        case ODCSET:// Set GPIO high.
            ExtCurrentStatus[pin]|=4; //remember the current status
            break;
        case ODCCLR:// Set GPIO high.
            ExtCurrentStatus[pin]&=3; //remember the current status
            break;
        case CNPUSET:// Sets a pull-up
// lb pigpio            gpioSetPullUpDown(PinDef[pin].gpio, PI_PUD_UP);
            gpioflags=GPIOD_LINE_BIAS_PULL_UP; // lb
            ExtCurrentStatus[pin]|=8;
            break;
        case CNPDSET:// Sets a pull-down
// lb pigpio            gpioSetPullUpDown(PinDef[pin].gpio, PI_PUD_DOWN);
            gpioflags=GPIOD_LINE_BIAS_PULL_DOWN; // lb
            ExtCurrentStatus[pin]|=8;
            break;
        case CNPUCLR:
        case CNPDCLR:
// lb pigpio            gpioSetPullUpDown(PinDef[pin].gpio, PI_PUD_OFF); 
            gpioflags=GPIOD_LINE_BIAS_DISABLED; // lb no pullup/pulldown
            ExtCurrentStatus[pin]&=7;
        break;
    }
//printf("PinSetBit: %i %i %i %i\n",pin,PinDef[pin].gpio,ExtCurrentStatus[pin],((ExtCurrentStatus[pin] & 2 ) >>1));
//ExtCurrentStatus[pin]=2; // lb
        switch(ExtCurrentStatus[pin]){
            case 0: //simple low output
            case 2:  //simple high output
            case 4: //output a low value in OC mode
            case 6: //output a high value in OC mode
            case 8:
            case 10:
            case 12:
            case 14:
//printf("PinSetBit: calling gpioWrite\n",pin,PinDef[pin].gpio,((ExtCurrentStatus[pin] & 2 ) >>1));
                gpioWrite(PinDef[pin].gpio,((ExtCurrentStatus[pin] & 2 ) >>1));
//printf("PinSetBit: back from gpioWrite\n",pin,PinDef[pin].gpio,((ExtCurrentStatus[pin] & 2 ) >>1));
// lb                gpioSetMode(PinDef[pin].gpio,PI_OUTPUT);
                break;
                
            case 1: //input
            case 3:
            case 5:
            case 7:
            case 9:
            case 11:
            case 13:
            case 15:
// lb pigpio                gpioSetMode(PinDef[pin].gpio,PI_INPUT);
//                if(offset==CNPUSET) { gpioflags=GPIOHANDLE_REQUEST_BIAS_PULL_UP; }
//                else if(offset==CNPDSET) { gpioflags=GPIOHANDLE_REQUEST_BIAS_PULL_DOWN; }
//                else { gpioflags=GPIOHANDLE_REQUEST_BIAS_DISABLE; }
                gpio=PinDef[pin].gpio;
//printf("PinSetBit: input: 0x%X 0x%X 0x%X\n",offset,CNPUSET, gpioflags);
                if(gpioline[gpio]==NULL) gpioRead(gpio); // set up the line
                break;

        }

}


// return the value of a pin's input
int PinRead(int pin) {

    if((ExtCurrentStatus[pin] & 1) || (ExtCurrentStatus[pin]==6)) 
      return gpioRead(PinDef[pin].gpio) & 1;
    else return (ExtCurrentStatus[pin] & 2)>>1;
}



void *IrDev, *IrCmd;
char IrVarType;
char IrState, IrGotMsg;
int IrBits, IrCount;
char *IrInterrupt;



/****************************************************************************************************************************
Interrupt service routines for the counting functions (eg, frequency, period)
*****************************************************************************************************************************/

// perform the counting functions for INT1
/*
void INT1Interrupt(int gpio, int level, uint32_t tick) {
    if(level==1){
        if(INT1tick==0)INT1tick=tick;
	if(ExtCurrentConfig[INT1PIN] == EXT_PER_IN) {
            if(INT1Timer-- <= 0) {
                INT1Value = tick - INT1tick;
                INT1Timer = INT1InitTimer;
                INT1Count = 0;
                INT1tick = 0;
            }
	}
	else
		INT1Count++;
    }

     return;
}



// perform the counting functions for INT2
void INT2Interrupt(int gpio, int level, uint32_t tick) {
    if(level==1){
        if(INT2tick==0)INT2tick=tick;
	if(ExtCurrentConfig[INT2PIN] == EXT_PER_IN) {
            if(INT2Timer-- <= 0) {
                INT2Value = tick - INT2tick;
                INT2Timer = INT2InitTimer;
                INT2Count = 0;
                INT2tick = 0;
            }
	}
	else
		INT2Count++;
    }
     return;
}




// perform the counting functions for INT3
void INT3Interrupt(int gpio, int level, uint32_t tick) {
    if(level==1){
        if(INT3tick==0)INT3tick=tick;
	if(ExtCurrentConfig[INT3PIN] == EXT_PER_IN) {
            if(INT3Timer-- <= 0) {
                INT3Value = tick - INT3tick;
                INT3Timer = INT3InitTimer;
                INT3Count = 0;
                INT3tick = 0;
            }
	}
	else
		INT1Count++;
    }

     return;
}




// perform the counting functions for INT4
// this interrupt is also used by the IR command and also to wake us from sleep
void INT4Interrupt(int gpio, int level, uint32_t tick) {
    int ElapsedMicroSec;
    static unsigned int LastIrBits;
    if(IrState == IR_CLOSED) {
    if(level==1){
        if(INT4tick==0)INT4tick=tick;
	if(ExtCurrentConfig[INT4PIN] == EXT_PER_IN) {
            if(INT4Timer-- <= 0) {
                INT4Value = tick - INT4tick;
                INT4Timer = INT4InitTimer;
                INT4Count = 0;
                INT4tick = 0;
            }
	}
	else
            INT4Count++;
        }
    } else {
        if(level!=nextIRlevel)return;
        // this is an IR interrupt
        ElapsedMicroSec = tick-TMR1;
        switch(IrState) {
            case IR_WAIT_START:
                TMR1 = tick;                                           // reset the timer
                nextIRlevel=risingedge;
                IrState = IR_WAIT_START_END;                        // wait for the end of the start bit
                break;
            case IR_WAIT_START_END:
                if(ElapsedMicroSec > 2000 && ElapsedMicroSec < 2800)
                    IrState = SONY_WAIT_BIT_START;                  // probably a Sony remote, now wait for the first data bit
                else if(ElapsedMicroSec > 8000 && ElapsedMicroSec < 10000)
                    IrState = NEC_WAIT_FIRST_BIT_START;             // probably an NEC remote, now wait for the first data bit
                else {
                    IrState = IR_WAIT_START;
                    nextIRlevel=fallingedge;
                    IrCount = 0;
                    TMR1 = tick;
                    break;
                }
                IrCount = 0;                                        // count the bits in the message
                IrBits = 0;                                         // reset the bit acumulator
                TMR1 = tick;                                           // reset the timer
                nextIRlevel=fallingedge;
                break;
            case SONY_WAIT_BIT_START:
                if(ElapsedMicroSec < 300 || ElapsedMicroSec > 900) { IrReset(); break; }
                TMR1 = tick;                                           // reset the timer
                nextIRlevel=risingedge;
                IrState = SONY_WAIT_BIT_END;                         // wait for the end of this data bit
                break;
            case SONY_WAIT_BIT_END:
                if(ElapsedMicroSec < 300 || ElapsedMicroSec > 1500 || IrCount > 20) { IrReset(); break; }
                IrBits |= (ElapsedMicroSec > 900) << IrCount;       // get the data bit
                IrCount++;                                          // and increment our count
                TMR1 = tick;                                           // reset the timer
                nextIRlevel=fallingedge;
                IrState = SONY_WAIT_BIT_START;                       // go back and wait for the next data bit
                break;
            case NEC_WAIT_FIRST_BIT_START:
                if(ElapsedMicroSec > 2000 && ElapsedMicroSec < 2500) {
                    IrBits = LastIrBits;                            // key is held down so just repeat the last code
                    IrCount = 32;                                   // and signal that we are finished
                    IrState = NEC_WAIT_BIT_END;
                    break;
                }
                else if(ElapsedMicroSec > 4000 && ElapsedMicroSec < 5000)
                    IrState = NEC_WAIT_BIT_END;                     // wait for the end of this data bit
                else {
                    IrState = IR_WAIT_START;
                    nextIRlevel=fallingedge;
                    IrCount = 0;
                    TMR1 = tick;
                    break;
                }
                TMR1 = tick;                                           // reset the timer
                nextIRlevel=risingedge;
                break;
            case NEC_WAIT_BIT_START:
                if(ElapsedMicroSec < 400 || ElapsedMicroSec > 1800) { IrReset(); break; }
                IrBits |= (ElapsedMicroSec > 840) << (31 - IrCount);// get the data bit
                LastIrBits = IrBits;
                IrCount++;                                          // and increment our count
                TMR1 = tick;                                           // reset the timer
                nextIRlevel=risingedge;
                IrState = NEC_WAIT_BIT_END;                         // wait for the end of this data bit
                break;
            case NEC_WAIT_BIT_END:
                if(ElapsedMicroSec < 400 || ElapsedMicroSec > 700) { IrReset(); break; }
                if(IrCount == 32) break;
                TMR1 = tick;                                           // reset the timer
                nextIRlevel=fallingedge;
                IrState = NEC_WAIT_BIT_START;                       // go back and wait for the next data bit
                break;
        }
    }

    return;
}

*/ // lb

int CheckPin(int pin, int action) {



    if(pin < 1 || pin > NBRPINS || (PinDef[pin].mode & UNUSED)) {
        if(!(action & CP_NOABORT)) error("Pin % is invalid", pin);
        return false;
    }
    
    if(!(action & CP_IGNORE_INUSE) && ExtCurrentConfig[pin] > EXT_NOT_CONFIG && ExtCurrentConfig[pin] < EXT_COM_RESERVED) {
        if(!(action & CP_NOABORT)) error("Pin % is in use", pin);
        return false;
    }
    
    if(!(action & CP_IGNORE_BOOTRES) && ExtCurrentConfig[pin] == EXT_BOOT_RESERVED) {
        if(!(action & CP_NOABORT)) {
            error("Pin % is reserved on startup", pin);
// LB           uSec(1000000);
             sleep(1);
        }
        return false;
    }
    
    if(!(action & CP_IGNORE_RESERVED) && ExtCurrentConfig[pin] >= EXT_COM_RESERVED) 
    {
        if(!(action & CP_NOABORT)) {
            error("Pin % is in use", pin); 
        }
        return false;
    }
    
    return true;
}


/* // lb
void cmd_wave(void){
    int wave_id, i, pin, initstate, nbrp;
    	void *ptr = NULL;
        float ftime;
        gpioPulse_t Send_Buffer[10000];
    	getargs(&cmdline, 5, ",");
        if(argc != 5)error("Argument count");
        pin = getinteger(argv[0]);
        CheckPin(pin, CP_IGNORE_INUSE);
        nbrp=getint(argv[2],1,10000);
	ptr = findvar(argv[4], V_NOFIND_NULL | V_EMPTY_OK);
	if(ptr == NULL) error("Invalid variable");
        if((vartbl[VarIndex].type & T_NBR) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// numeric array
            if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + nbrp) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
                    error("Insufficient data");
            } else {
                for (i = 0; i < nbrp; i++) {
                    Send_Buffer[i].usDelay = (int)(*((MMFLOAT *)ptr + i));
		}
            }
	} else if((vartbl[VarIndex].type & T_INT) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// integer array
            if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + nbrp) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
		error("Insufficient data");
            } else {
		for (i = 0; i < nbrp; i++) {
                    Send_Buffer[i].usDelay = (int)(*((long long int *)ptr + i));
                }
            }
	} else error("Invalid variable");
        initstate=(ExtCurrentStatus[pin] & 2 ) >>1;
        gpioWaveClear();
        for (i = 0; i < nbrp; i++) {
            if(initstate){
                Send_Buffer[i].gpioOff = 1<<PinDef[pin].gpio;
                Send_Buffer[i].gpioOn = 0;
                initstate=0;
            } else {
                Send_Buffer[i].gpioOn = 1<<PinDef[pin].gpio;
                Send_Buffer[i].gpioOff = 0;
                initstate=1;
            }
            ftime=Send_Buffer[i].usDelay + 0.5;
            if(gpioHardwareRevision()==0xa02082 || gpioHardwareRevision()==0xa22082){
                ftime/=Option.WaveTime;
                Send_Buffer[i].usDelay=(int)ftime;
            }
        }
        if(initstate){
            Send_Buffer[nbrp].gpioOff = 1<<PinDef[pin].gpio;
            Send_Buffer[nbrp].gpioOn = 0;
        } else {
            Send_Buffer[nbrp].gpioOn = 1<<PinDef[pin].gpio;
            Send_Buffer[nbrp].gpioOff = 0;
        }
        Send_Buffer[nbrp].usDelay=1;

    gpioWaveAddGeneric(nbrp+1, Send_Buffer);
    wave_id = gpioWaveCreate();
    gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT);
    while (gpioWaveTxBusy());
    gpioWaveDelete(wave_id);
}
*/ // lb

/* * * * * * * SUBSTITUTE ROUTINES FOR pigpio * * * * * */ // lb

int gpioWrite(unsigned gpio, unsigned level) { // lb substitute for pigpio routine
  int ret=-1; // -1;
  char sPin[]="Pin     ", sGPIO[]="gpioset 0 27=1", sConsumer[]="MMB4L";
//printf("gpioWrite: %i %i\n",gpio,level);
  if(!gpioline[gpio]) {
    gpio_offsets[0]=gpio;
    gpiolineconfig=gpiod_line_config_new();
    if(!gpiolineconfig) error("Unable to configure line (pin)");
    gpiod_line_config_set_direction(gpiolineconfig, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiolinereqconfig=gpiod_request_config_new();
    if(!gpiolinereqconfig) error("Unable to request line (pin)");
    gpiod_request_config_set_consumer(gpiolinereqconfig,sConsumer);
    gpiod_request_config_set_offsets(gpiolinereqconfig,1,gpio_offsets);
    gpioline[gpio]=gpiod_chip_request_lines(chip, gpiolinereqconfig,gpiolineconfig);
    if(!gpioline[gpio]) error("Unable to set line (pin)");
  }
// int gpiod_line_request_set_value(struct gpiod_line_request *request,
//       unsigned int offset, int value)
  ret = gpiod_line_request_set_value(gpioline[gpio],gpio,level);
  if(ret==-1) error("Error setting GPIO value");
  return ret;
}

int gpioRead(unsigned gpio) { // lb substitute for pigpio routine
  int ret=-1; // -1;
  char sPin[]="Pin     ", sGPIO[]="gpioset 0 27=1", sConsumer[]="MMB4L";
//printf("gpioRead: %i\n",gpio);
  if(!gpioline[gpio]) {
    gpio_offsets[0]=gpio;
    sprintf(sPin,"Pin %i",gpio);
//struct gpiod_line_config *gpiolineconfig; // lb reset/reused for each line 
    gpiolineconfig=gpiod_line_config_new();
    if(!gpiolineconfig) error("Unable to set line (pin)");
    gpiod_line_config_set_direction(gpiolineconfig, GPIOD_LINE_DIRECTION_INPUT);
    if(gpioflags) gpiod_line_config_set_bias(gpiolineconfig, gpioflags);
    gpiolinereqconfig=gpiod_request_config_new();
    if(!gpiolinereqconfig) error("Unable to set line (pin)");
    gpiod_request_config_set_consumer(gpiolinereqconfig,sConsumer);
    gpiod_request_config_set_offsets(gpiolinereqconfig,1,gpio_offsets);
    gpioline[gpio]=gpiod_chip_request_lines(chip, gpiolinereqconfig,gpiolineconfig);
    if(!gpioline[gpio]) error("Unable to set line (pin)");

//    printf("gpioRead2: after gpiod_line_request_bulk %i flags: 0x%X \n",gpio, gpioflags); 
  }
  ret = gpiod_line_request_get_values(gpioline[gpio],gpioVal);
//printf("gpioRead4: after gpiod_line_request_get_values: %i=%i; ret=%i\n",gpio,gpioVal[0],ret);
  if(ret<0) error("Error reading GPIO value");
/**/
  return gpioVal[0];
}

int gpioSetMode(unsigned gpio, unsigned mode) { // lb substitute for pigpio routine
 // lb 211029 -- setting input pullup, pulldown
  return 0;
}

