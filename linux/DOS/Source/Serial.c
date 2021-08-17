/***********************************************************************************************************************
MMBasic

Misc.c

Handles all the miscellaneous commands and functions in DOS MMBasic.  These are commands and functions that do not
comfortably fit anywhere else.

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


#include <time.h>
#include <stdio.h>
#include <windows.h>
#include <wincon.h>
#include <process.h>
#include <tchar.h>
#include <conio.h>
#include <strsafe.h>

#include "..\..\Version.h"

extern FILE *MMFilePtr[MAXOPENFILES];
extern HANDLE *MMComPtr[MAXOPENFILES];
int cback = -1;

// Retrieve, format, and print out a message from the last error.
void system_error(char *msg) {
    char *ptr = NULL;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        0,
        GetLastError(),
        0,
        (char *)&ptr,
        1024,
        NULL);

    error(msg, ptr);
    LocalFree(ptr);
}


void SerialOpen(char *arg1, char *arg2) {
    HANDLE fd;
    BOOL fSuccess;
    COMMTIMEOUTS timeouts;
    DCB port;
    char ComPort[20] = "\\\\.\\";                                 // "
    char b;
    int i, fnbr, read = 0;
    if(*arg2 == '#') arg2++;
    fnbr = getint(arg2, 1, 10);
    fnbr--;
    if(MMFilePtr[fnbr] != NULL || MMComPtr[fnbr] != NULL) error("File number is already open");
  i = strlen(ComPort);
  while(*arg1 != ' ' && *arg1 != ':' && *arg1) {
      ComPort[i] = toupper(*arg1);
      i++ ; arg1++;
  }
  ComPort[i] = 0;
  if(*arg1 == ':') arg1++;
  skipspace(arg1);

    //  Open a handle to the specified com port.
    MMComPtr[fnbr] = CreateFile(ComPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
    if (MMComPtr[fnbr] == INVALID_HANDLE_VALUE) {
      MMComPtr[fnbr] = NULL;
      system_error("Cannot open port: $");
    }

  if(*arg1) {
        // get the current DCB, and adjust.
        memset(&port, 0, sizeof(port));
        port.DCBlength = sizeof(port);
        if ( !GetCommState(MMComPtr[fnbr], &port))
            system_error("Cannot configure port: $");
        if (!BuildCommDCB(arg1, &port))
            system_error("Cannot configure port: $");
        if (!SetCommState(MMComPtr[fnbr], &port))
            system_error("Cannot configure port: $");
  }

    // set short timeouts on the comm port.
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.ReadTotalTimeoutConstant = 1;
    timeouts.WriteTotalTimeoutMultiplier = 1;
    timeouts.WriteTotalTimeoutConstant = 1;
    if (!SetCommTimeouts(MMComPtr[fnbr], &timeouts))
        system_error("Cannot configure port: $");
}


void SerialClose(HANDLE fd) {
  CloseHandle(fd);
}


int SerialEOF(HANDLE fd) {
    unsigned long int read;
    unsigned char c;
    if(cback != -1) return false;
    if(!ReadFile(fd, &c, 1, &read, NULL))
      system_error("Reading port: $");
    if(read == 0) return true;
    cback = c;
    return false;
}



int Serialgetc(HANDLE fd) {
  int c;
  while(SerialEOF(fd)) CheckAbort();
  c = cback;
  cback = -1;
  return c;
}


int Serialputc(int c, HANDLE fd) {
  unsigned long int written;
  if(!WriteFile(fd, &c, 1, &written, NULL))
      system_error("Writing to port: $");
    return c;
 }


int SerialRxQueueSize(HANDLE fd) {
    unsigned long int t;
    COMSTAT stat;
    if(!ClearCommError(fd, &t, &stat))
        system_error("Accessing port: $");
    targ = T_INT;
    return stat.cbInQue;
}



