/************************************************************************************************************************
Micromite

I2C.c

Routines to handle I2C access.

Copyright 2011 Gerard Sexton
This file is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

************************************************************************************************************************/


#include "version.h"
#include "i2c.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


// Declare functions
void i2cEnable(char *p);
void i2cDisable(char *p);
void i2cSend(char *p);
void i2cReceive(char *p);
void i2cCheck(char *p) ;
void i2c_disable(void);
void i2c_enable(int bps);
void i2c_masterCommand(int timer); 

MMFLOAT *I2C_Rcvbuf_Float;										// pointer to the master receive buffer for a float
long long int *I2C_Rcvbuf_Int;								// pointer to the master receive buffer for an integer
char *I2C_Rcvbuf_String;										// pointer to the master receive buffer for a string
unsigned int I2C_Addr;										// I2C device address
volatile unsigned int I2C_Sendlen;							// length of the master send buffer
volatile unsigned int I2C_Rcvlen;							// length of the master receive buffer
char I2C_Send_Buffer[256];                                // I2C send buffer
char I2C_Receive_Buffer[256];                                // I2C send buffer
unsigned int I2C_enabled=0;									// I2C enable marker
unsigned int I2C_Timeout;									// master timeout value
unsigned int I2C_Status;									// status flags
unsigned int i2cwait;
static int mmI2Cvalue;	
unsigned int I2C_Timer;
int i2c_options;// value of MM.I2C
int i2cfile;  // lb on pi, /dev/i2c-1 file
char i2cbuf[33]; // lb
char I2C_Clock=3, I2C_Data=5; // lb from flash.h

/*******************************************************************************************
							  I2C related commands in MMBasic
                              ===============================
These are the functions responsible for executing the I2C related commands in MMBasic
They are supported by utility functions that are grouped at the end of this file

********************************************************************************************/

void cmd_i2c(void) {
    char *p;//, *pp;

    if((p = checkstring(cmdline, "OPEN")) != NULL)
        i2cEnable(p);
    else if((p = checkstring(cmdline, "CLOSE")) != NULL)
        i2cDisable(p);
    else if((p = checkstring(cmdline, "WRITE")) != NULL)
// e.g.,  I2C WRITE MCP23017,0,2,IODIRA,0 ' set direction to output
        i2cSend(p);
    else if((p = checkstring(cmdline, "READ")) != NULL)
        i2cReceive(p);
    else
        error("Unknown command");
}
void i2cADC(void){
    
}
// enable the I2C1 module - master mode
void i2cEnable(char *p) {
	int speed, timeout;
    int adapter_nr = 1; /* 1 for pi, else dynamically determined */
    char filename[20];

    I2C_Clock=3; I2C_Data=5; // lb for pi, else determined how?
	getargs(&p, 3, ",");
	if(argc != 3) error("Syntax");
	speed = getinteger(argv[0]);
	if(speed < 10 || speed > 400) error("Number out of bounds");
        i2cwait=500/speed;
	timeout = getinteger(argv[2]);
	if(timeout < 0 || (timeout > 0 && timeout < 100)) error("Number out of bounds" );
	if(I2C_enabled) error("I2C already OPEN");
	I2C_Timeout = timeout;
// lb	SetupI2C(speed); // how to do this with libi2c

    sprintf(filename,"/dev/i2c-%d",adapter_nr);
    if ((i2cfile = open(filename,O_RDWR)) < 0) {
      // ERROR HANDLING; you can check errno to see what went wrong
      if ((i2cfile = open("/dev/i2c-0",O_RDWR)) < 0) {
        error("Couldn't open %s or -0; for raspberry pi, run raspi-config\n",filename);
      }
    }
	I2C_enabled=1;
}


// disable the I2C1 module - master mode
void i2cDisable(char *p) {
	I2C_enabled=0;
        close(i2cfile);
//	i2c_disable();
}


// send data to an I2C slave - master mode
void i2cSend(char *p) {
	int addr, sendlen, i;
	void *ptr = NULL;
	char *cptr = NULL;
// e.g.,  I2C WRITE MCP23017,0,2,IODIRA,0 ' set direction to output
	getargs(&p, 99, ",");
	if(!(argc & 0x01) || (argc < 7)) error("Syntax");
	if(!I2C_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
	if(addr < 0x00 || (addr > 0x00 && addr > 0x77)) error("Invalid address");
	i2c_options = getinteger(argv[2]);
	if(i2c_options < 0 || i2c_options > 1) error("Number out of bounds");
	I2C_Status = 0;
	I2C_Addr = addr;
	sendlen = getinteger(argv[4]);
	if(sendlen < 1 || sendlen > 255) error("Number out of bounds");

	if(sendlen == 1 || argc > 7) {		// numeric expressions for data
            if(sendlen != ((argc - 5) >> 1)) error("Argument count");
		for (i = 0; i < sendlen; i++) {
                    I2C_Send_Buffer[i] = getinteger(argv[i + i + 6]);
		}
            } else {		// an array of float, integer or a string
		ptr = findvar(argv[6], V_NOFIND_NULL | V_EMPTY_OK);
		if(ptr == NULL) error("Invalid variable");
		if((vartbl[VarIndex].type & T_STR) && vartbl[VarIndex].dims[0] == 0) {		// string
			cptr = (char *)ptr;
			cptr++;																	// skip the length byte in a MMBasic string
			for (i = 0; i < sendlen; i++) {
				I2C_Send_Buffer[i] = (int)(*(cptr + i));
			}
		} else if((vartbl[VarIndex].type & T_NBR) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// numeric array
			if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((float *)ptr + i));
				}
			}
		} else if((vartbl[VarIndex].type & T_INT) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// integer array
			if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((long long int *)ptr + i));
				}
			}
		} else error("Invalid variable");
	}
	I2C_Sendlen = sendlen;
	I2C_Rcvlen = 0;
        if (ioctl(i2cfile,I2C_SLAVE,addr) < 0) { 
          error("Couldn't find I2C device at %x\n", addr);
    /* ERROR HANDLING; you can check errno to see what went wrong */
        }
        if (write(i2cfile,I2C_Send_Buffer,sendlen) != sendlen)
          error("Write to I2C device at %x failed\n", addr);
//printf("i2cWrite len,bytes: %d %x %x\n",sendlen,I2C_Send_Buffer[0],I2C_Send_Buffer[1]);
//	i2c_masterCommand(1);

}

// receive data from an I2C slave - master mode
void i2cReceive(char *p) {
	int addr, rcvlen;
	void *ptr = NULL;
	getargs(&p, 7, ",");
        I2C_Rcvbuf_Float = NULL;
        I2C_Rcvbuf_Int = NULL;
        I2C_Rcvbuf_String = NULL;
	if(argc != 7) error("Syntax");
	if(!I2C_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
	if(addr < 0x00 || (addr > 0x00 && addr > 0x77)) error("Invalid address");
	i2c_options = getinteger(argv[2]);
	if(i2c_options < 0 || i2c_options > 1) error("Number out of bounds");
	I2C_Status = 0;
	if(i2c_options & 0x01) I2C_Status = I2C_Status_BusHold;
	I2C_Addr = addr;
	rcvlen = getinteger(argv[4]);
	if(rcvlen < 1 || rcvlen > 255) error("Number out of bounds");

	ptr = findvar(argv[6], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
	if(ptr == NULL) error("Invalid variable");
	if(vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C_Rcvbuf_Float = (MMFLOAT*)ptr;
//printf("i2cReceive: %s %d\n",vartbl[VarIndex].name,vartbl[VarIndex].type);
        if (read(i2cfile,I2C_Rcvbuf_Float,rcvlen) != rcvlen) 
          error("Invalid I2C read");
    } else if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C_Rcvbuf_Int = (long long int *)ptr;
        if (read(i2cfile,I2C_Rcvbuf_Int,rcvlen) != rcvlen) 
          error("Invalid I2C read");
//printf("i2cReceive2: %s %d %d\n",vartbl[VarIndex].name,vartbl[VarIndex].type,vartbl[VarIndex].val);
    } else if(vartbl[VarIndex].type & T_STR) {
//printf("i2cReceive3: %s %d\n",vartbl[VarIndex].name,vartbl[VarIndex].type);
        if(vartbl[VarIndex].dims[0] != 0) error("Invalid variable");
        *(char *)ptr = rcvlen;
        I2C_Rcvbuf_String = (char *)ptr + 1;
        if (read(i2cfile,I2C_Rcvbuf_String,rcvlen) != rcvlen) 
          error("Invalid I2C read");
    } else error("Invalid variable");
	I2C_Rcvlen = rcvlen;

	I2C_Sendlen = 0;

// lb	i2c_masterCommand(1);
}


/**************************************************************************************************
Disable the I2C1 module - master mode
***************************************************************************************************/
void i2c_disable() {
/*
    I2C_Status = I2C_Status_Disable;
    I2C_Rcvbuf_String = NULL;                                       // pointer to the master receive buffer
    I2C_Rcvbuf_Float = NULL;
    I2C_Rcvbuf_Int = NULL;
    I2C_Sendlen = 0;												// length of the master send buffer
    I2C_Rcvlen = 0;													// length of the master receive buffer
    I2C_Addr = 0;													// I2C device address
    I2C_Timeout = 0;												// master timeout value
    PinSetBit(I2C_Data, CNPUCLR);                                  // remove pullup resitors that may have been set
    PinSetBit(I2C_Clock, CNPUCLR);
    ExtCfg(I2C_Data, EXT_NOT_CONFIG, 0);							// set pins to unconfigured
    ExtCfg(I2C_Clock, EXT_NOT_CONFIG, 0);
    I2C_enabled = 0;
    i2cfile=0; // on pi, /dev/i2c-1 made unavailable
*/
}


//void fun_mmi2c(void) {
//	iret = mmI2Cvalue;
//    targ = T_INT;
//}

/**************************************************************************************************
Send and/or Receive data - master mode
***************************************************************************************************/
void i2c_masterCommand(int timer) {
//	char start_type,
/*
        DoI2C(I2C_Addr);
	if(I2C_Rcvlen){
	char i;
				for(i=0;i<I2C_Rcvlen;i++){
					if(I2C_Rcvbuf_String!=NULL){
						if(!mmI2Cvalue)*I2C_Rcvbuf_String=I2C_Receive_Buffer[i];
                                                else *I2C_Rcvbuf_String=0;
						I2C_Rcvbuf_String++;
					}
					if(I2C_Rcvbuf_Float!=NULL){
						if(!mmI2Cvalue)*I2C_Rcvbuf_Float=I2C_Receive_Buffer[i];
                                                else *I2C_Rcvbuf_Float=0.0;
						I2C_Rcvbuf_Float++;
					}
					if(I2C_Rcvbuf_Int!=NULL){
						if(!mmI2Cvalue)*I2C_Rcvbuf_Int=I2C_Receive_Buffer[i];
                                                else *I2C_Rcvbuf_Int=0;
						I2C_Rcvbuf_Int++;
					}
				}
	}
*/
}

// lb below specific ADC I2C modules not yet implemented
/*
int domcp3424(int channel, int range, int precision){ // lb A/D converter--ADC
    char buff[4]={0,0,0,0};
    char config=0x80;
    int32_t rval;
    if(range==256)config |= 3;
    else if(range==512)config |= 2; 
    else if(range==1024)config |= 1; 
    if(channel==1)config |= 0x20;
    else if(channel==2)config |=0x40;
    else if(channel==3)config |=0x60;
    if(precision==14)config |= 0x04;
    else if(precision==16)config |= 0x08;
    else if(precision==18)config |= 0x0C;
    I2C_Status = 0;
    I2C_Sendlen = 1;                                                // send one byte
    I2C_Rcvlen = 0;
    I2C_Send_Buffer[0] = config;                                           // the first register to read
    I2C_Addr = ADCaddr;                                                // address of the device
    i2c_masterCommand(1);
    do {
        uSec(1000);
        I2C_Rcvbuf_String = buff;                                       // we want a string of bytes
        I2C_Rcvbuf_Float = NULL;
        I2C_Rcvbuf_Int = NULL;
        I2C_Rcvlen = 4;                                                 // get 7 bytes
        I2C_Sendlen = 0;
        I2C_Addr = ADCaddr;                                                // address of the device
        i2c_masterCommand(1);
    } while((buff[3] & 0x80) && mmI2Cvalue==0 );
    if(precision==18){
        rval= (buff[0]<<16) | (buff[1]<<8) | buff[2];
        if(buff[0] & 0x80)rval |= 0xFF000000;
    } else {
        rval= (buff[0]<<8) | buff[1];
        if(buff[0] & 0x80)rval |= 0xFFFF0000;
    }
    rval<<=1;
    return rval;
}
int doads1015(int channel, int range){ // lb A/D converter--ADC
    char hb;
    int rval;
    char buff[2]={0,0};
    
    channel=((channel+4)<<4);
    hb=0x8F;
    if(range==512)hb=0x89;
    else if(range==1024)hb=0x87;
    else if(range==2048)hb=0x85;
    else if(range==4096)hb=0x83;
    else if(range==6144)hb=0x81;
    hb = hb | channel;
    I2C_Status = 0;
    I2C_Sendlen = 3;                                                // send one byte
    I2C_Rcvlen = 0;
    I2C_Send_Buffer[0] = 1;                                           // the first register to read
    I2C_Send_Buffer[1] = hb;                                           // the first register to read
    I2C_Send_Buffer[2] = 83;                                           // the first register to read
    I2C_Addr = ADCaddr;                                                // address of the device
    i2c_masterCommand(1);
    while(!(buff[0] & 0x80) && mmI2Cvalue==0 ){
        I2C_Sendlen = 1;                                                // send one byte
        I2C_Rcvlen = 0;
        I2C_Send_Buffer[0] = 1;                                           // the first register to read
        i2c_masterCommand(1);
        I2C_Rcvbuf_String = buff;                                       // we want a string of bytes
        I2C_Rcvbuf_Float = NULL;
        I2C_Rcvbuf_Int = NULL;
        I2C_Rcvlen = 2;                                                 // get 7 bytes
        I2C_Sendlen = 0;
        I2C_Addr = ADCaddr;                                                // address of the device
        i2c_masterCommand(1);
    }
    I2C_Sendlen = 1;                                                // send one byte
    I2C_Rcvlen = 0;
    I2C_Send_Buffer[0] = 0;                                           // the first register to read
    i2c_masterCommand(1);
    //read the value
    I2C_Rcvbuf_String = buff;                                       // we want a string of bytes
    I2C_Rcvbuf_Float = NULL;
    I2C_Rcvbuf_Int = NULL;
    I2C_Rcvlen = 2;                                                 // get 7 bytes
    I2C_Sendlen = 0;
    I2C_Addr = ADCaddr;                                                // address of the device
    i2c_masterCommand(1);
    rval= (buff[0]<<8) | buff[1];
    if(buff[0] & 0x80)rval |= 0xFFFF0000;
    rval<<=1;
    return rval;
}
void fun_adc(void){
    char channel;
    int range,counts, precision;
    getargs(&ep, 5, ",");
    if(ADC==0)error("No ADC");
    if(argc != 3 && (ADC == 1 || ADC==3)) error("Syntax");
    if(argc != 5 && ADC == 2) error("Syntax");
    channel=getint(argv[0],0,3);
    if(ADC==1 || ADC==3){
        if(checkstring(argv[2], "AUTO"))range=6144; 
        else  range=getint(argv[2],256,6144);
        if(!(range==256 || range==512 || range==1024 || range==2048 || range==4096 || range==6144))error("Invalid range");
        counts=doads1015(channel,range);
        if(checkstring(argv[2], "AUTO") && counts<1280){
            if(counts<80*32)range=256;
            else if(counts<160*32)range=512;
            else if(counts<320*32)range=1024;
            else if(counts<640*32)range=2048;
            else if(counts<1280*32)range=4096;
            counts=doads1015(channel,range);
        }
        fret = ((MMFLOAT)counts +0.5l)/65536.0l * (MMFLOAT)range /1000.0l;
        if(fret<0.0)fret=0;
        if(mmI2Cvalue) fret=1000.0l;

    } else if(ADC==2){
        if(checkstring(argv[2], "AUTO"))range=2048; 
        else  range=getint(argv[2],256,2048);
        if(!(range==256 || range==512 || range==1024 || range==2048))error("Invalid range");
        precision=getint(argv[4],12,18);
        if(!(precision==12 || precision==14 || precision==16 || precision==18))error("Invalid precision");
        counts=domcp3424(channel,range,precision);
        int autocount=counts;
        if(precision==18)autocount>>=6;
        else if(precision==16)autocount>>=4;
        else if(precision==14)autocount>>=2;
        if(checkstring(argv[2], "AUTO") && autocount<1000){
            if(abs(autocount)<250)range=256;
            else if(abs(autocount)<500)range=512;
            else if(abs(autocount)<1000)range=1024;
            counts=domcp3424(channel,range,precision);
        }

            
#if defined(MMFLOATPRECISION)
        fret = ((MMFLOAT)counts +0.5l)/((MMFLOAT)(1<<precision)) * (MMFLOAT)range /1000.0l;
        if(mmI2Cvalue) fret=1000.0l;
#else
        fret = (float)counts/2048.0l * (float)range /1000.0l;
#endif
    }
    targ = T_NBR;
}
// 
*/

