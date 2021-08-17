This is the source to MMBasic for DOS on Windows.

The following instructions assume that you are using a Windows (XP/Vista/Win7) platform.

First:  Rename the file "DOS\BuildAll.bat.txt" to "DOS\BuildAll.bat"
        This was forced on me because some email systems reject .bat files because they are an executable.

You should install the free Watcom version 1.9 C/C++ compiler for Windows 32 bit.
This is available from http://www.openwatcom.org.

When installing the compiler is best to let it install at its default location (C:\WATCOM).  If you install it
somewhere else you will have to edit the file BuildAll.bat to suit.

MMBasic does not use any special features of the compiler so it should compile with later versions of the Watcom compiler.

This source will also compile with Microsoft Visual C/C++ Express which is also free to download and use.  This is
more complex to setup so only use it if you have had some experience with it.  In this case you will have to instruct
Visual C/C++ Express to make all chars unsigned by default.  You will also have to set the paths of include files and
add the following switches to the compiler's command line: /TC /D "MSVCC"

Unzip the MMBasic source files keeping the directory structure.

To compile the source (using the Watcom compiler) simply run the batch command file:  BuildAll.bat
This will create the executable (MMBasic.exe) in the same directory.

Go to http://mmbasic.com for updates and licensing

Copyright 2011 - 2018 Geoff Graham.  All Rights Reserved.

This file and modified versions of this file are supplied to specific individuals or organisations under the following
provisions:

- This file, or any files that comprise the MMBasic source (modified or not), may not be distributed or copied to any other
  person or organisation without written permission.

- Object files (.o and .hex files) generated using this file (modified or not) may not be distributed or copied to any other
  person or organisation without written permission.

- This file is provided in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

