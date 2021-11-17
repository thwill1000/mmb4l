/***********************************************************************************************************************
MMBasic

IOPorts.h

Include file that defines the IOPins for the Raspberry Pi in MMBasic.
  
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




// these are the valid peek/poke memory ranges for the MX170/MX270

// General defines
#define P_INPUT				1						// for setting the TRIS on I/O bits
#define P_OUTPUT			0
#define P_ON				1
#define P_OFF				0


// Structure that defines the SFR, bit number and mode for each I/O pin
struct s_PinDef {
    char gpio;           // this is the bit number of the port, eg the 7 in B7
    char mode;             // the various modes that an I/O pin can be set to (defined below)
};
    
// Defines for the various modes that an I/O pin can be set to
#define UNUSED       (1 << 0)
#define DIGITAL_IN   (1 << 2)
#define COUNTING     (1 << 3)
#define DIGITAL_OUT  (1 << 4)
#define DO_NOT_RESET (1 << 5)

#define NBR_PINS_40CHIP     40					    // number of pins for external i/o on a 44 pin chip
#define NBRPINS             NBR_PINS_40CHIP	// number of pins for external i/o
#define MAXNBRPINS          40
    
// lb pigpio defines
#define PI_INPUT 0
#define PI_OUTPUT 1
#define PI_ALT0 4
#define PI_ALT1 5
#define PI_ALT2 6
#define PI_ALT3 7
#define PI_ALT4 3
#define PI_ALT5 2

#define PI_FILE_READ  1
#define PI_FILE_WRITE 2
#define PI_FILE_RW    3
#define PI_FILE_APPEND 4
#define PI_FILE_CREATE 8
#define PI_FILE_TRUNC  16

// Define the structure for the I/O pins
// the first element of the structure contains a pointer to the SFR for the port to be used
// the second element is the bit number within that port to use
// the third is a set of flags that defines what that I/O pin can do


// Define the counting pin numbers.  INT1PIN refers to the PIC32 external interrupt #1, an so on for the others
#define INT1PIN             29

#define INT2PIN             12

#define INT3PIN             24

#define INT4PIN             33
#define WAKEUP_PIN             33

// I2C pin numbers
#define P_I2C_SDA           3
#define P_I2C_SCL           5

// COM1: port pin numbers
#define COM1_RX_PIN         10
#define COM1_TX_PIN         8

// COM2: port pin numbers
#define COM2_RX_PIN         11
#define COM2_TX_PIN         7
    
// SPI pin numbers
#define SPI_INP_PIN         21
#define SPI_OUT_PIN         19
#define SPI_CLK_PIN         23

// touch controller interface
#define TOUCH_SPI_CHANNEL       SPI_CHANNEL1

// SPI LCD controller interface
#define SPI_LCD_SPI_CHANNEL     SPI_CHANNEL1


//SSD1963_pins
#define SSD1963_WR_PIN 32 //GPIO 12
#define SSD1963_RS_PIN 13 //GPIO 27
#define SSD1963_RESET_PIN 31 //GPIO 6

#define SSD1963_DAT1 35 //GPIO 19
#define SSD1963_DAT2 38 //GPIO 20 
#define SSD1963_DAT3 40 //GPIO 21
#define SSD1963_DAT4 15 //GPIO 22
#define SSD1963_DAT5 16 //GPIO 23
#define SSD1963_DAT6 18 //GPIO 24
#define SSD1963_DAT7 22 //GPIO 25
#define SSD1963_DAT8 37 //GPIO 26
