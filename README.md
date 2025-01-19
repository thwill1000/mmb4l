# MMBasic for Linux (MMB4L)

1. [Introduction](#1-introduction)
2. [How do I install it ?](#2-how-do-i-install-it-)
    * [Audio configuration on the Raspberry Pi](#audio-configuration-on-the-raspberry-pi)
3. [How do I run it ?](#3-how-do-i-run-it-)
    * [Where are the games?](#where-are-the-games-)
    * [Start with a shebang #!](#start-with-a-shebang-)
5. [How do I use it ?](#4-how-do-i-use-it-)
6. [The EDITor](#5-the-editor)
    * [Configuring GNU nano](#configuring-gnu-nano)
    * [Using GNU nano](#using-gnu-nano)
7. [Predefined Read Only Variables](#6-predefined-read-only-variables)
    * [MM.INFO()](#mminfo)
    * [MM.INFO$()](#mminfo)
    * [MM.HRES](#mmhres)
    * [MM.VER](#mmver)
    * [MM.VRES](#mmvres)
8. [Options](#7-options)
    * [OPTION AUDIO](#option-audio)
    * [OPTION AUTOSCALE](#option-autoscale)
    * [OPTION CODEPAGE](#option-codepage)
    * [OPTION CONSOLE](#option-console)
    * [OPTION EDITOR](#option-editor)
    * [OPTION F\<num>](#option-fnum)
    * [OPTION LIST](#option-list)
    * [OPTION LOAD](#option-load)
    * [OPTION RESET](#option-reset)
    * [OPTION SAVE](#option-save)
    * [OPTION SIMULATE](#option-simulate)
9. [Commands](#8-commands)
    * [CLS](#cls)
    * [CONSOLE](#console)
    * [DEVICE GAMEPAD](#device-gamepad)
    * [END](#end)
    * [ERROR](#error)
    * [GRAPHICS](#graphics)
    * [OPEN](#open)
    * [POKE](#poke)
    * [PRINT](#print)
    * [QUIT](#quit)
    * [RESTORE](#restore)
    * [RUN](#run)
    * [SETENV](#setenv)
    * [SYSTEM](#system)
    * [XMODEM](#xmodem)
10. [Functions](#9-functions)
    * [CHR$](#chr)
    * [DEVICE](#device)
    * [JSON$](#json)
    * [PEEK](#peek)
11. [Miscellaneous differences from MMBasic 6.0 for the PicoMite + PicoMite VGA](#10-miscellaneous-differences-from-mmbasic-60-for-the-picomite--picomite-vga)
    * [The "current program file"](#the-current-program-file)
    * [Automatic path completion](#automatic-path-completion)
    * [The "bang" command !](#the-bang-command-)
    * [Other limitations](#other-limitations)
12. [How do I build MMB4L from source ?](#12-how-do-i-build-mmb4l-from-source-)
13. [Credits](#13-credits)
14. [FAQ](#14-faq)

## 1. Introduction

MMB4L is a port of Geoff Graham's [MMBasic](https://mmbasic.com/) interpreter to the Linux platform (and its derivatives).

It was originally derived with permission from:
 * [MMBasic for DOS](https://geoffg.net/WindowsMMBasic.html)
     * Copyright 2011-2025 Geoff Graham

But also incorporates code and ideas from several other MMBasic ports:
 * [MMBasic for the PicoMite](https://geoffg.net/picomite.html) and [PicoMite VGA](https://geoffg.net/picomitevga.html)
     * Copyright 2011-2025 Geoff Graham
     * Copyright 2016-2025 Peter Mather
     * https://github.com/UKTailwind/PicoMiteAllVersions
 * [MMBasic for the Colour Maximite 2](https://geoffg.net/maximite.html)
     * Copyright 2011-2025 Geoff Graham
     * Copyright 2016-2025 Peter Mather
 * MMBasic for Windows
     * Copyright 2011-2025 Geoff Graham
     * Copyright 2016-2025 Peter Mather
     * https://github.com/UKTailwind/MMB4W
 * Mothballed Pi-cromite project by Peter Mather.

What little MMB4L specific code there is, is Copyright 2021-2025 Thomas Hugo Williams.

MMB4L is an open-source project distributed under a modified 4-clause BSD license, see the [LICENSE.MMBasic](LICENSE.MMBasic) file for details.

## 2. How do I install it ?

MMB4L is "alpha release" software so there are no fancy installer packages yet:

 1. Download the .tgz for the desired platform from the [latest release](https://github.com/thwill1000/mmb4l/releases/latest):
    * For 64-bit Linux running on Intel/AMD use the 'x86_64' build.
    * For 64-bit Linux Debian/Raspbian 11 "bullseye" running on ARM, e.g. Raspberry Pi 4 & 5, use the 'aarch64' build.
    * For 32-bit Linux Debian/Raspbian 11 "bullseye" running on ARM, e.g. Raspberry Pi Zero, 1, 2 & 3, use the 'armv6l' build.
        * Note that graphics and audio performance on the original Raspberry Pi Zero is very poor, I have not tested on a 1 or 2.

 2. Extract the archive into a temporary location:
    * ```mkdir -p ~/tmp && tar -xf mmb4l-<version>.tgz --directory ~/tmp```

 3. Copy the "mmbasic" executable to "/usr/local/bin":
    * ```sudo cp ~/tmp/mmb4l-<version>/mmbasic /usr/local/bin```
    * or copy it where you like and create a symbolic link from "/usr/local/bin/mmbasic"

 4. The .tgz also contains resource files for [configuring the nano editor](#configuring-gnu-nano) for use within MMB4L.

### Audio configuration on the Raspberry Pi

On Raspberry Pi devices I have had better results using the legacy "alsa" driver for audio rather than the default "pulseaudio" driver.

To configure this you need to set the `SDL_AUDIODRIVER` environment variable, e.g. by adding this to your `.bashrc` file:
```
export SDL_AUDIODRIVER=alsa
```

## 3. How do I run it ?

 * Type `mmbasic` at the Linux shell and it should show the start banner and display a BASIC command prompt:

    ![MMBasic prompt](resources/console-banner.png)

 * To have MMB4L immediately run a program ...
     * ... and exit when the program ends or reports an error:
         * `mmbasic myprogram.bas arg1 arg2 arg3`
     * ... and return to the BASIC command prompt when the program ends or reports an error use the `-i`, `--interactive` command-line option:
         * `mmbasic -i myprogram.bas arg1 arg2 arg3`
     * any command-line arguments to the BASIC program are retrievable using `MM.CMDLINE$`.

 * To start MMB4L in a specific directory:
     * either use the `-d`, `--directory` command-line option:
         *  `mmbasic -d ~/mmbasic-workspace`
     * or set the MMDIR environment variable:
         *  `export MMDIR=~/mmbasic-workspace`

 * To see other MMB4L command-line options use the `-h`, `--help` command-line option:
     * `mmbasic -h`

 * Use the `QUIT` command to exit from MMB4L
     * `Ctrl-C` does not exit MMB4L; it instead interrupts the currently running BASIC program and returns control to the BASIC prompt.

### Where are the games ?

Under [latest release](https://github.com/thwill1000/mmb4l/releases/latest) there is a `gamepack-<version>.tgz` file. Once downloaded and extracted it contains an executable `menu` program which should launch a menu of curated games for MMB4L.
 * This can be executed at the Linux terminal, you do not need to run it from MMB4L.
 * For this to work you must create `/usr/local/bin/mmbasic` as described in [2.3 above](#2-how-do-i-install-it-).

### Start with a shebang #!

Just like the Game★Pack `menu` program above you can use MMB4L to write executable scripts by starting them with a `#!` and the path to the `mmbasic` executable, e.g.

Write "hello-world.bas":
```
#!/usr/local/bin/mmbasic
Do
  Print "Hello World"
Loop
```
Make it executable:

`chmod 755 "hello-world.bas"`

And run it from the Linux shell:

`./hello-world.bas`

__IMPORTANT!__ The .bas file must have UNIX style line-endings, i.e. just LF (`\n`), not CRLF (`\r\n`). If it does not then you will get this error: `bash: ./hello-world.bas: cannot execute: required file not found`

## 4. How do I use it ?

Perhaps one day there will be a user manual specific to MMB4L, until then you are directed to:
 * [PicoMite User Manual](https://geoffg.net/Downloads/picomite/PicoMite_User_Manual.pdf)
 * This README which (hopefully) documents all significant differences between the behaviour of MMB4L and that of MMBasic on the PicoMite & PicoMite VGA.

## 5. The EDITor

Unlike other MMBasic platforms the ```EDIT``` command for MMB4L does not use a bespoke editor but instead relies on a third-party editor being installed. By default this is [GNU nano](https://www.nano-editor.org/), but this can be changed using the the [OPTION EDITOR](#option-editor) command.

### Configuring GNU nano

 1. Check you have nano installed, and determine the version:
    * ```nano --version```
    * If nano is not installed then [install it](https://phoenixnap.com/kb/use-nano-text-editor-commands-linux).

 2. The "mmb4l-\<version>.tgz" file contains resource files to configure nano to behave similarly to the PicoMite's integrated editor:
    * If you are running nano 4.8+:
      * ```
        mkdir -p ~/.mmbasic
        cp ~/tmp/mmb4l-<version>/mmbasic.nanorc ~/.mmbasic
        cp ~/tmp/mmb4l-<version>/mmbasic.syntax.nanorc ~/.mmbasic
        ```

    * If you have an earlier version and do not need to use nano for anything else:
      * ```
        mkdir -p ~/.nano
        cp ~/tmp/mmb4l-<version>/mmbasic.syntax.nanorc ~/.nano
        cp ~/tmp/mmb4l-<version>/mmbasic.nanorc ~/.nanorc
        ```
      * Edit "~/.nanorc" and change last line to: ```include ~/.nano/mmbasic.syntax.nanorc```

    * If you have an earlier version and are an existing nano user:
      * ```
        mkdir -p ~/.nano
        cp ~/tmp/mmb4l-<version>/mmbasic.syntax.nanorc ~/.nano
        ```
      * Edit "~/.nanorc" and append the line: ```include ~/.nano/mmbasic.syntax.nanorc```
      * If desired incorporate settings/bindings from "\~/tmp/mmb4l-\<version>/mmbasic.nanorc" into your existing "\~/.nanorc" file.

### Using GNU nano

The editor is started by typing `EDIT "myfile.bas"` at the BASIC prompt.
   
When using the default MMB4L nano resource files the following keyboard bindings are enabled:

    Ctrl+A              Display the nano help text
    Ctrl+O or F7        Insert a file into current one
    Ctrl+Q or F1        Exit the editor
    Ctrl+S or F2        Save the current file

    Ctrl+P              Goto the previous word
    Ctrl+N              Goto the next word
    Ctrl+B              Goto the first line
    Ctrl+E              Goto the last line
    Ctrl+L              Display the position of the cursor
    Ctrl+J              Goto line

    Ctrl+K              Delete current line / selection
    Ctrl+C              Copy marked test
    Ctrl+V              Paste the copy buffer
    Ctrl+X              Cut marked text

    Ctrl+Space or F4    Turn the mark on/off
    Ctrl+F              Find
    Ctrl+G or F3        Find Next
    Ctrl+H              Replace

    Ctrl+Z              Undo
    Ctrl+Y              Redo

    Alt+A               Toggle display of keyboard bindings at bottom of editor
    Alt+L               Toggle constant display of cursor position

Where not overridden by the above the [default nano keyboard bindings](https://www.nano-editor.org/dist/latest/cheatsheet.html) apply, e.g. **Tab** to indent, **Shift+Tab** to unindent, and **Alt+3** toggle the comments of a selection.

**Gotchas:**

 1. Unlike other MMBasic version there is no key combination to automatically **RUN** a program from the editor.
 2. If you rename a file whilst saving it MMB4L will not update its "current program file" state and will still be using the previous file.

## 6. Predefined Read Only Variables

### MM.INFO()

MMB4L supports reading these additional properties:

 * `MM.INFO(ARCH)`
     * Gets the Linux architecture that MMB4L is running on, currently one of:
         * "Android aarch64"
         * "Linux aarch64"
         * "Linux armv6l"
         * "Linux i686"
         * "Linux x86_64"
     * Note that `MM.INFO$(DEVICE)` will return "MMB4L" for all of these.

 * `MM.INFO(CALLDEPTH)`
     * Gets the the current function/subroutine call depth starting at 0 when not in a function/subroutine.
     * Primarily for debugging purposes, though a possible production use-case would be to allow a program to "bail out" if recursion gets too deep.

 * `MM.INFO(CPUTIME)`
     * Gets the value (in nanoseconds) of the CPU timer for the MMB4L process.

 * `MM.INFO$(CURRENT)`
     * Gets the name of the current program or `NONE` if called after a `NEW` command.

 * `MM.INFO$(DEVICE X)`
     * Gets the real/underlying device name on all MMBasic platforms, even when `OPTION SIMULATE` is being used on MMB4L.
     * _This relies on the lenient error checking currently exhibited by other MMBasic platforms not reporting the "extraneous" X as a syntax error._

 * `MM.INFO$(ENVVAR name$)`
     * Gets the value of the named environment variable, or the empty string if there is no such environment variable.

 * `MM.INFO$(ERRMSG [errno%])`
     * Without the `errno%` argument gets the current error message, c.f. `MM.ERRMSG$`.
     * With the `errno%` argument gets the default error message corresponding to that error number.
         * _Note that currently error numbers are not stable between MMB4L releases._

 * `MM.INFO$(ERRNO)`
     * Gets the current error number, c.f. `MM.ERRNO`.

 * `MM.INFO(EXISTS path$)`
     * Does the file / directory / device corresponding to `path$` exist ?

 * `MM.INFO(EXISTS SYMLINK path$)`
     * Does `path$` correspond to a symbolic link ?

 * `MM.INFO(EXITCODE)`
     * Gets the exit code "returned" by the last program run:
         * If an `END` command is executed then this will be the optional exit code (default 0) specified to that command.
         * If a program runs to its "natural end" without an explicit `END` command this will be 0.
         * If a program is interrupted by the break key combination (default CTRL-C) this will be 130.
         * If an unhandled `ERROR` occurs this will be 1 (this may change).
         * If a ```NEW``` command is executed this will be 0.

 * `MM.INFO$(GAMEPAD id%)`
     * Gets SDL identification/configuration string for an attached game controller.
     * If no controller is attached then returns the empty string.

  * `MM.INFO(HPOS)`
     * Gets the current horizontal position (in characters) following the last `PRINT` command.
         * `OPTION CONSOLE PIXEL` can be used to change this to return a value in pixels based on a nominal 8x12 font.
     * Unlike the PicoMite, drawing graphics and using the TEXT command does not change the reported position.

 * `MM.INFO(HRES)`
     * Gets the height of the current graphics window in pixels.
     * If no graphics window is selected then returns the height of the console in characters.
         * `OPTION CONSOLE PIXEL` can be used to change this to return a value in pixels based on a nominal 8x12 font.

 * `MM.INFO(LINE)`
     *  Gets the current MMBasic line number being executed.
 
 * `MM.INFO$(OPTION <option>)`
     * Gets the value of the named option; this is supported for all options.

 * `MM.INFO(PID)`
     * Gets the process ID of the MM4L process.

 * `MM.INFO(VERSION [MAJOR | MINOR | MICRO | BUILD] )`
     * Gets the MMB4L major, minor, micro version or build number as an integer.
         * The MAJOR and MINOR versions each have 2 digits.
         * The MICRO version has 3 digits, the most significant digit distinguishes the type:
             * 0 - alpha
             * 1 - beta
             * 2 - RC (release candidate)
             * 3 .. 9 - release, in which case the "real" MICRO version is the 3-digit number minus 300.
         * The BUILD number has 4 digits but is currently unused and always returns 0.
     * Without the additional argument it returns an integer = MAJOR * 100,000,000 + MINOR * 1,000,000 + MICRO * 10000 + BUILD, e.g. 5,000,000 for version 0.5.

  * `MM.INFO(VPOS)`
     * Gets the current vertical position (in characters) following the last `PRINT` command.
         * `OPTION CONSOLE PIXEL` can be used to change this to return a value in pixels based on a nominal 8x12 font.
     * Unlike the PicoMite, drawing graphics and using the TEXT command does not change the reported position.

 * `MM.INFO(VRES)`
     * Gets the width of the current graphics window in pixels.
     * If no graphics window is selected then returns the width of the console in characters.
         * `OPTION CONSOLE PIXEL` can be used to change this to return a value in pixels based on a nominal 8x12 font.

### MM.HRES

See `MM.INFO$(HRES)`.

### MM.VER

In MMB4L this inbuilt constant is an `INTEGER` value insted of a `FLOAT`, it has the same value as `MM.INFO(VERSION)`.

### MM.VRES

See `MM.INFO$(VRES)`.

## 7. Options

### OPTION AUDIO

`OPTION AUDIO {ON | OFF}`

Persistent option to enable/disable audio.

 * Default ON.
 * The intended purpose of this OPTION is to allow MMB4L to be easily run on Linux devices (in particular within Docker containers) where the SDL audio is too difficult for the user to be bothered to configure ;-).
 * Note that when OFF all the underlying audio boilerplate is still executed except for the bit that makes the noise and the calling of audio interrupts.

### OPTION AUTOSCALE

`OPTION AUTOSCALE {ON | OFF}`

Persistent option to enable/disable automatic scaling of windows when using OPTION SIMULATE.

 * Default ON.
 * When ON the window for the simulated device will be scaled up (in whole number multipliers) to be the largest that will fit on the display.

### OPTION CODEPAGE

`OPTION CODEPAGE {<page> | page$}`

Causes character bytes in the range 128-255 to be translated to Unicode (UTF-8) when printed to the console according to a given code page / mapping.

Supported code pages are:
 * `NONE` - no mapping, characters 128-255 are written as-is.
 * `CMM2` - best effort mapping to the characters of the CMM2 font.
 * `CP437` - characters 128-255 of the original IBM PC code 437. Note this was a non-ANSI code page which also included characters 1-31 which are not available in this mapping, but see "MMB4L".
 * `CP1252` - the classic Windows-1252 Latin alphabet code page.
 * `MMB4L` - same as `CP437` but with characters 129-159 replaced by the original non-ANSI CP437 characters 1-31 and with additional "useful" characters in positions 128, 161-170 and 255.

### OPTION CONSOLE

`OPTION CONSOLE {PIXEL|CHARACTER}`

Controls the resolution used for the return values of `HRES`, `VRES`, `MM.INFO(HPOS)`, `MM.INFO(HRES)`, `MM.INFO(VPOS)` and `MM.INFO(VRES)` when no graphics surface is selected, i.e. when writing to the console.

 * Default `CHARACTER`.
 * If `PIXEL` then the returned values are based on a nominal 8x12 font.

### OPTION EDITOR

`OPTION EDITOR {<editor> | editor$}`

Controls which editor is used by the EDIT command.

Supported editors are:
 * `ATOM`
 * `CODE`     (synonym for `VSCODE`)
 * `DEFAULT`  (synonym for `NANO`)
 * `GEANY`
 * `EDIT`
 * `LEAFPAD`
 * `NANO`
 * `SUBLIME`
 * `VI`
 * `VIM`
 * `VSCODE`
 * `XED`

Alternatively a string expression can be provided which either evaluates to one of the editors listed above or explicitly specifies a Linux shell command to execute to launch an editor, e.g.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`OPTION EDITOR "code -g ${file}:${line}"`

Where:
 * `${file}` is a placeholder for the filename to edit, the filename will be automatically enclosed in double-quotes.
 * `${line}` is a placeholder for the line number to set the initial cursor position to.

### OPTION F\<num>

`OPTION {F1 .. F12} string$`

Defines the string that will be generated when the given function key is pressed at the MMBasic prompt.
   * Unlike the PicoMite all of F1-F12 may be redefined by the user.
   * _HOWEVER depending on the window manager being used some function key presses may be captured by the window manager and not passed on to MMBasic._

### OPTION LIST

`OPTION LIST [ALL]`

Lists values of current options (permanent and non-permanent).
   * If `ALL` is specified then all options are listed.
   * Otherwise only options that have been changed from their default values are listed.

### OPTION LOAD

`OPTION LOAD filename$`

Loads permanent options from the named file and where possible applies them immediately.
   * If they can not be applied immediately then they will be applied when MMB4L is restarted.

### OPTION RESET

`OPTION RESET {ALL | <option>}`

Resets options to their default values.
   * If `ALL` is specified then all options are reset.
   * Otherwise only the named option is reset.

### OPTION SAVE

`OPTION SAVE filename$`

Saves permanent options that have been changed from their default values to the named file.

### OPTION SIMULATE

`OPTION SIMULATE device$`

Non-permanent option that configures MMB4L to attempt to "simulate" the behaviour of a given MMBasic `device$` which must be one of the following:
 * "Colour Maximite 2" or "CMM2"
 * "Game*Mite"
 * "MMB4L" to restore default MMB4L behaviour.
 * "PicoMiteVGA"
 * "MMBasic for Windows" or "MMB4W"
 
#### What is simulated for each device ?

 * `MM.DEVICE$`, `MM.INFO$(DEVICE)` and `MM.INFO$(PLATFORM)` will return the appropriate values for the simulated device.
     * _Whilst "simulating" use `MM.INFO$(DEVICE X)` can be used to retrieve the real device name, i.e. "MMB4L"._

##### Colour Maximite 2
 * Simulates USB controllers 1-3 being read as if they were Wii Classic controllers attached to I2C channels 1-3:
     * USB1 = I2C3
     * USB2 = I2C1
     * USB3 = I2C2.
 * Simulates commands:
     * `CONTROLLER CLASSIC { CLOSE | OPEN }`, `MODE`, `PAGE { COPY | SCROLL | WRITE }`
 * Simulates functions:
     * `CLASSIC()`

##### Game*Mite (RP2040):
 * Simulates sufficient GPIO to support USB controller 1 being read using `SETPIN` and `PORT()` as if it were the 8-button Game*Mite controller.
 * Simulates commands:
     * `FRAMEBUFFER`, `BLIT FRAMEBUFFER`, `FLASH DISK LOAD`, `SETPIN`
 * Simulates functions:
     * `MM.INFO(CPUSPEED)`, `MM.INFO(DRIVE)`, `MM.INFO(FLASH ADDRESS)`, `MM.INFO(PINNO)`, `PORT()`

##### MMBasic for Windows:
 * As "Colour Maximite 2" except supports a single USB controller being read with the `GAMEPAD` command and function.

##### PicoMiteVGA (RP2040):
 * Simulates sufficient GPIO to support USB controllers 1 & 2 being read using `PIN()`, `PULSE` and `SETPIN` as if they were 12-button SNES controllers wired according to the PicoGAME VGA 2.0 schematic.
 * Simulates commands:
     * `FRAMEBUFFER`, `BLIT FRAMEBUFFER`, `FLASH DISK LOAD`, `MODE`, `PIN`, `PULSE`, `SETPIN`
 * Simulates functions:
     * `MM.INFO(CPUSPEED)`, `MM.INFO(DRIVE)`, `MMM.INFO(FLASH ADDRESS)`, `MM.INFO(PINNO)`, `PIN()`
 * _Note that `MODE 1` coloured tiles are not currently supported._

## 8. Commands

### CLS

`CLS [colour%]`

If a graphics surface is selected (with the `GRAPHICS WRITE` command) then clears the current surface to the specified `colour%`.
 * If `colour%` is unspecified then clears the surface to the current background colour (as selected with the `COLOUR` command).
 * If a graphics surface is not selected then behaves like `CLS CONSOLE`.

#### CLS CONSOLE

`CLS CONSOLE`  

Clears the console using the current background colour AND moves the cursor to the origin (0, 0).
 * This is synonymous with the `CONSOLE CLEAR` command.

### CONSOLE

The CONSOLE commands manipulate the console/terminal using ANSI escape-codes.

#### CONSOLE BACKGROUND

`CONSOLE BACKGROUND {<colour_name>|colour%} `

Sets the background colour for future `PRINT` commands.
 * Allowed colours and equivalent integers are:
     * 0 = Black
     * 1 = Blue
     * 2 = Green
     * 3 = Cyan
     * 4 = Red
     * 5 = Magenta or Purple
     * 6 = Yellow
     * 7 = White

#### CONSOLE BELL

`CONSOLE BELL`

Sounds the console "bell".

#### CONSOLE CLEAR

`CONSOLE CLEAR`

Clears the console using the current background colour AND moves the cursor o the origin (0, 0).
 * This is synonymous with the `CLS CONSOLE` command.

#### CONSOLE GETCURSOR

`CONSOLE GETCURSOR x%, y%`

Reads the cursor position into the `x%` and `y%` variables.
 * `x%` and `y%` are character coordinates with origin (0, 0).

#### CONSOLE GETSIZE

`CONSOLE GETSIZE width%, height%`

Reads the current console size into the `width%` and `height%` variables.
 * `width%` and `size%` are in characters.

#### CONSOLE FOREGROUND

`CONSOLE FOREGROUND {[BRIGHT] <colour_name> | colour%}`

Sets the foreground colour for future `PRINT` commands.
 * Allowed colours are the same as for `CONSOLE BACKGROUND` but with an optional `BRIGHT` attribute:
     * 8 = Grey (Bright Black)
     * 9 = Bright Blue
     * 10 = Bright Green
     * 11 = Bright Cyan
     * 12 = Bright Red
     * 13 = Bright Magenta or Bright Purple
     * 14 = Bright Yellow
     * 15 = Bright White

#### CONSOLE HIDECURSOR

`CONSOLE HIDECURSOR [ON | TRUE | OFF | FALSE | z%]`

Hides or shows the cursor; without any argument this hides the cursor.

#### CONSOLE HOME

`CONSOLE HOME`

Moves the cursor to the origin (0, 0).

#### CONSOLE INVERSE

`CONSOLE INVERSE [ON | TRUE | OFF | FALSE | z%]`

Inverts the foreground and background colours.

#### CONSOLE INVERT

`CONSOLE INVERT [ON | TRUE | OFF | FALSE | z%]`

Synonym for `CONSOLE INVERSE`.

#### CONSOLE RESET

`CONSOLE RESET`

Resets the foreground, background and invert attributes.

#### CONSOLE SETCURSOR

`CONSOLE SETCURSOR x%, y%`

Moves the cursor to (`x%`, `y%`).
 * `x%` and `y%` are character coordinates with origin (0, 0).
 * _Note that all PRINTing starts at and moves the current cursor position._

#### CONSOLE SETSIZE

`CONSOLE SETSIZE [ATLEAST] width%, height%`

Sets the console size.
 * If the optional `ATLEAST` modifier is used then this will not reduce the console width or height.
 * This works by sending an XTerm control sequence and may not succeed with all Linux terminal programs.

#### CONSOLE SETTITLE

`CONSOLE SETTITLE title$`

Sets the console window title.

#### CONSOLE SHOWCURSOR

`CONSOLE SHOWCURSOR [ON | TRUE | OFF | FALSE | z%]`

Shows or hides the cursor; without any argument this shows the cursor.

### DEVICE GAMEPAD

The DEVICE GAMEPAD commands configure attached game controllers.
 * Up to four controllers are supported with ids 1, 2, 3 and 4.

#### DEVICE GAMEPAD CLOSE

`DEVICE GAMEPAD CLOSE id%`

Closes/terminates a game controller.

#### DEVICE GAMEPAD OPEN

`DEVICE GAMEPAD OPEN id% [, interrupt] [, bitmask%]`

Opens/initialises a game controller.
 * If `interrupt` is specified then calls the given interrupt subroutine when a digital button is pressed on the controller, interrupts are not fired for analog sticks/buttons.
 * If `bitmask%` is specified then only digital buttons whose bit matches the bitmask will cause an interrupt to occur.
 * If this command reports the error: `"Gamepad error: Couldn't find mapping for device (#)"` then the connected gamepad is not known to the SDL library. Try following the instructions at https://www.generalarcade.com/gamepadtool/ to generate an SDL mapping for the gamepad controller which can then be set using the `SDL_GAMECONTROLLERCONFIG` environment variable, e.g.  
   ```
   export SDL_GAMECONTROLLERCONFIG=0300c2f8830500006020000010010000,iBuffalo SNES Controller,a:b0,b:b1,back:b6,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b7,x:b2,y:b3,hint:SDL_GAMECONTROLLER_USE_BUTTON_LABELS:=1,platform:Linux
   ```

 #### DEVICE GAMEPAD VIBRATE

`DEVICE GAMEPAD VIBRATE id% [, low_freq%] [, high_freq%] [, duration_ms%]`

 Causes a game controller to vibrate.
 * `low_freq%` and `high_freq%` are the intensity of the low and high frequency vibrations from 0 to &hFFFF. They default to &hFFFF.
 * `duration_ms%` is the duration of the vibration in milliseconds. It defaults to 10,000 ms.
  * _Note this may not be available on all platforms._

 To stop a game controller from vibrating use:
 
`DEVICE GAMEPAD VIBRATE id% OFF`
 
### END

```END [exit_code%]```

Ends the running program "returning" an optional exit code (default 0).
 * If running interactively this returns to the BASIC command prompt and the optional exit code (default 0) is retrievable via ```MM.INFO(EXITCODE)```.
 * If running non-interactively this behaves the same as the [QUIT](#quit) command.
 * For details about exit codes see the [QUIT](#quit) command.

### ERROR

`ERROR [error_msg$ [, errno%]]`

Forces an error and terminates the program. This is normally used in debugging or to trap events that should not occur.
 * The value of `error_msg$` (default "Unspecified error") can be retrieved with `MM.ERRMSG$`.
 * The value of `errno%` (default 256) can be retrieved with `MM.ERRNO`.
    * 0 - no error.
    * 1..255 - standard error numbers (errno) for C library calls.
    * 256 - unclassified MMBasic error.
    * 257..1023 - other standard MMBasic errors.
        * _Note that currently error numbers are not stable between MMB4L releases._
    * 1024+ - recommended range for program specific errors.

### GRAPHICS

The GRAPHICS commands are used to create, destroy and manipulate MMB4L's graphics surfaces.

 * There are three types of surfaces:
     * Windows that are created with `GRAPHICS WINDOW`.
     * Buffers that are created with `GRAPHICS BUFFER` or `BLIT READ`. Buffers are manipulated by the `BLIT` commands.
     * Sprites that are created with `GRAPHICS SPRTE` or `SPRITE READ`. Sprites are manipulated by the `SPRITE` commands.
 * Surfaces are created with ids 0-255, but surface 0 can only be a window surface.
 * The MMBasic graphics primitive commands can write to ANY of these surface types and you may `BLIT` to and from any of these surface types.
 * *Note that the `PRINT` command cannot be used to print to a graphics surface, it always prints to the console. This restriction even applies when using `OPTION SIMULATE`.*

#### GRAPHICS BUFFER

`GRAPHICS BUFFER id%, width%, height%`

Creates an off-screen buffer graphics surface.

#### GRAPHICS CLS

`GRAPHICS CLS id% [, colour%]`

Clears a graphics surface.

 * If the `colour%` parameter is not specified then uses the current background colour as selected with the `COLOUR` command.

#### GRAPHICS COPY

`GRAPHICS COPY src_id% TO dst_id% [, when%] [, transparent%]`

Copies one graphics surface to another.

 * If the `transparent%` parameter is set to `T` or `1` then BLACK pixels on the source surface are considered transparent when copying. Other values are ignored.
 * The `when%` parameter is currently ignored. 
 * Copying between surfaces of different sizes is supported. The source surface is always copied to the top left (0, 0) of the destination surface and will either be clipped (if larger) or leave the destination pixels untouched (if smaller).

#### GRAPHICS DESTROY

`GRAPHICS DESTROY { id% | ALL }`

Destroys a graphics surface, or all graphics surface if `ALL` is specified.

#### GRAPHICS INTERRUPT

`GRAPHICS INTERRUPT id%, { interrupt | 0 }`

Sets or clears the interrupt subroutine for a window surface.

* The interrupt subroutine must have the following signature:
    * `SUB my_subroutine_name(window_id%, event_id%)`
* Within the subroutine the following constants for `event_id%` are automatically defined:
    * `WINDOW_EVENT_CLOSE`
    * `WINDOW_EVENT_FOCUS_GAINED`
    * `WINDOW_EVENT_FOCUS_LOST`
    * `WINDOW_EVENT_MINIMISED`
    * `WINDOW_EVENT_MAXIMISED`
    * `WINDOW_EVENT_RESTORED`

#### GRAPHICS LIST

`GRAPHICS LIST`

Writes diagnostic information about all graphics surfaces to the console.

#### GRAPHICS SPRITE

`GRAPHICS SPRITE id%, width%, height%`

Creates a sprite graphics surface.

#### GRAPHICS TITLE

`GRAPHICS TITLE id%, title$`

Sets the title of a window surface.

#### GRAPHICS WINDOW

`GRAPHICS WINDOW id%, width%, height% [, x%] [, y%] [, title$] [, scale%] [, interrupt]`

Creates a window graphics surface.

  * The optional `x%` and `y%` parameters are used to set the initial coordinates for the top left of the window. If unset (or -1) then the window will be horizontally and/or vertically centered.
  * If set then the optional `scale%` parameter will cause the window pixel size to be automatically scaled by this factor.
      * If the resulting window would be bigger than the display then the scaling factor will be automatically reduced so that the window fits. This reduction will be in integer steps whilst the scaling factor is greater than 2 and in steps of 0.05 when below that.
  * The optional `interrupt` parameter is used to set a subroutine to be called when various windows events occur, see the `GRAPHICS INTERRUPT` command for details.

#### GRAPHICS WRITE

`GRAPHICS WRITE { id% | NONE }`

Selects the surface to direct other graphics commands, e.g. `LINE`, to.

### OPEN

_WARNING! MMB4L serial communications are a work in progress and may be unnecessarily slow and flakey._

`OPEN comspec$ AS [#]fnbr`

Opens a serial communications port for reading and writing.

The `comspec$` has the format:

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`"device: baudrate, buffer-size, interrupt, interrupt-trigger, 7BIT, {EVEN | ODD}, S2, RTSCTS, XONXOFF"`.

**Arguments:**

The following arguments must be specified in order. If any argument is left out then all the following arguments must also be left out and will use their default values.
 * `device` - TTY device path
      * e.g. /dev/ttyACM0, /dev/ttyACM1, /dev/ttyS0, /dev/ttyS1, /dev/ttyUSB0, /dev/ttyUSB1, etc.
      * This is the only argument which is obligatory.
 * `baudrate` - one of:
      *  50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 500000, 576000, 921600, 1000000, 1152000, 1500000, 2000000, 2500000, 3000000, 3500000, 4000000.
      * Baudrates above 115.2K are theoretically supported by MMB4L but possibly not by any given Linux device driver, it may just fail to receive/transmit without reporting an actual error.
      * Default is 9600.
      * Arbitrary baudrates are not supported.
 * `buffer-size` - receive buffer size in bytes.
      * Default is 4096 bytes.
 * `interrupt` - user defined subroutine which will be called when the serial port has received some data.
      * Default is no interrupt.
 * `interrupt-trigger` - the interrupt subroutine is called when the receive buffer contains this number of bytes or greater.
      * Default is 1 byte.

**Flags:**

The following flags be specified in any order.
 * `7BIT` - use 7 bit transmit/receive, or 8 bit if using a parity bit.
 * `ODD` | `EVEN` - use odd or even parity bit.
     * This will transmit/receive 9 bits unless 7BIT is also specified in which case it will transmit/receive 8 bits.
     * Inline with other MMBasic platforms MMB4L does not actually perform parity checking on received data.
 * `S2` - use two stop bits following each character.
 * `RTSCTS` - enable RTS/CTS hardware flow control.
     * _At least in theory, this is untested._
 * `XONXOFF` - enable XON/XOFF software flow control.
     * _WARNING! not currently working._

MMB4L does not support the following flags that are available on the PicoMite and/or Colour Maximite 2: `DEP`, `DEN`, `INV` or `OC`.

**IMPORTANT!**

In order to open a serial port MMBasic must be running as a user that is a member of the `dialout` group.
```
sudo addgroup <user> dialout
```
Note that this change requires the user to logout and then login again to take effect.

### POKE

`POKE DATAPTR ptr%`

Sets the value of the "virtual pointer" used to track where the `READ` command reads `DATA` from.
 * Only values of `ptr%` previously retrieved by calling `PEEK(DATAPTR)` should be passed to this command.

### PRINT

`PRINT @(x%, y%) expression`

Outputs text to the console/terminal at a given character position followed by a carriage return/newline pair.
 * Unlike the PicoMite `x%` and `y%` are both obligatory and in character (not pixel) coordinates. There is no `mode` parameter.
 * It is equivalent to `CONSOLE SETCURSOR x, y : PRINT expression`

_Note that the `PRINT` command cannot be used to print to a graphics surface, it always prints to the console. This restriction even applies when using `OPTION SIMULATE`._

### QUIT

`QUIT [exit_code%]`

Exits MMB4L returning an optional exit code (default 0) to the shell.
 * The exit code should be between 0 and 255.
 * A value of 0 indicates success and a value of 1 indicates a general error.
 * Linux has no hard standard for other values, but for guidance see:
     * [Advanced Bash Scripting Guide](https://tldp.org/LDP/abs/html/exitcodes.html)
     * [sysexits - FreeBSD](https://www.freebsd.org/cgi/man.cgi?query=sysexits&apropos=0&sektion=0&manpath=FreeBSD+4.3-RELEASE&format=html)

### RESTORE

`RESTORE [<label> | label$ | line%]`

Resets the "virtual pointer" (line and position counters) for the `READ` statement.
   * With no argument the pointer is reset to the first `DATA` statement in the program.
   * With an explicit `<label>` or string-expression the pointer is reset to the first `DATA` statement following the corresponding label.
   * With an integer-expression the pointer is reset to the first `DATA` statement following the corresponding line number.

### RUN

`RUN [file$] [, cmdline$]`

Runs a program.
 * Both `file$` and `cmdline$` can be string expressions instead of the legacy behaviour where the command line argument was "not processed" by MMBasic and was copied verbatim into the `MM.CMDLINE$` of the new program.
     * If the text following the comma contains an unquoted minus sign then for backward compatibility MMB4L trys to use the legacy behaviour, but this does not work for all possible legacy command lines.
     * Note that the behaviour of the `*` command is unchanged; expressions in any command line provided via `*` are not evaluated by MMB4L, e.g.
        ```
        *foo a$ + b$
        ```
        is equivalent to:
        ```
        RUN "foo", "a$ + b$"
        ```

### SETENV

`SETENV name$, value$`  
`SETENV name$ = value$`  
`SETENV name$, value%()`  
`SETENV name$ = value%()`

 * Synonym for `SYSTEM SETENV`.

### SYSTEM

`SYSTEM cmd$ [, output$ [, exit_status%]]`  
`SYSTEM cmd$ [, output%() [, exit_status%]]`

Executes a Linux operating system command specified by `cmd$`.
 * To capture the output from the operating system command use an optional string `output$` or long string `output%()` parameter.
     * Otherwise output will appear on the console (stdout).
     * Output can also be directed to a file using standard Linux notation in the `cmd$` string.
 * An MMBasic error is only reported if the operating system command cannot actually be executed. If the command executes but fails then it's exit status can be captured in the optional `exit_status%` parameter.

#### SYSTEM GETENV

`SYSTEM GETENV name$, value$`  
`SYSTEM GETENV name$, value$%()`

Gets the value of an environment variable into a string or long string.
 * This supplements `MM.INFO$(ENVVAR)` by allowing values > 255 characters to be read into a long string.

#### SYSTEM SETENV

`SYSTEM SETENV name$, value$`  
`SYSTEM SETENV name$ = value$`  
`SYSTEM SETENV name$, value%`  
`SYSTEM SETENV name$ = value%`

Sets an environment variable to the value of a string or long string.

### XMODEM

`XMODEM {SEND | RECEIVE} file$, [#]fnbr`

Transfers a file to/from  a remote computer using the [XMODEM protocol](https://en.wikipedia.org/wiki/XMODEM).
 * `#fnbr` must first have been `OPEN`ed as a serial port.
 * Unlike other MMBasic platforms MMB4L cannot use `XMODEM` to send to or receive from the console.

## 9. Functions

### CHR$()

`CHR$(number%)`

Returns a one-character string consisting of the character corresponding to the
ASCII code (i.e. byte value) indicated by argument `number%`.

#### CHR$(UTF8 _args_)

`CHR$(UTF8 codepoint%)`

Returns a 1-4 character string encoding the Unicode `codepoint%` in UTF-8 format.
    
 * _Note that printing such characters is not compatible with `OPTION CODEPAGE` being set._

### DEVICE

`DEVICE(GAMEPAD id%, funct)`

Reads the current state of a game controller where `id%` is 1-4 and `funct` is a one or two letter code indicating the information to return as follows:
 * `B` - Returns a bitmap of the state of all the digital buttons. A bit will be set to 1 if the button is pressed:
     * BIT 0: Button R
     * BIT 1: Button START
     * BIT 2: Button HOME
     * BIT 3: Button SELECT
     * BIT 4: Button L
     * BIT 5: Button DOWN cursor
     * BIT 6: Button RIGHT cursor
     * BIT 7: Button UP cursor
     * BIT 8: Button LEFT cursor
     * BIT 9: Button ZR
     * BIT 10: Button X (triangle)
     * BIT 11: Button A (circle)
     * BIT 12: Button Y (square)
     * BIT 13: Button B (cross)
     * BIT 14: Button ZL
 * `LX` - Returns the position (-32768 .. 32767) of the analog left joystick x axis.
 * `LY` - returns the position (-32768 .. 32767) of the analog left joystick y axis.
 * `RX` - returns the position (-32768 .. 32767) of the analog right joystick x axis.
 * `RY` - returns the position (-32768 .. 32767) of the analog right joystick y axis.
 * `L` - returns the position (0 .. 32767) of the analog left trigger.
 * `R` - returns the position (0 .. 32767) of the analog right trigger.

### JSON$

`JSON$(array%(), string$ [, flags%])`

Returns a string representing a specific item out of the JSON input stored in the longstring `array%()`.
 * The optional flags parameter (default 0) specifies how explicit nulls and missing values should be handled:
     * &b00 - return empty strings for both.
     * &b01 - return `"<null>"` for explicit null.
     * &b10 - return `"<missing>"` for missing value.
     * &b11 - return both `"<null>"` and `"<missing>"`.

### PEEK

`pos% = PEEK(DATAPTR)`

Gets the current value of the "virtual pointer" used to track where the `READ` command reads `DATA` from.
 * This value is opaque and whilst it can be stored it should not be manipulated other than passing it to the `POKE DATAPTR ptr%` command.

## 10. Miscellaneous differences from MMBasic 6.0 for the PicoMite + PicoMite VGA

### The "current program file"

Similarly to the Colour Maximite 2, MMB4L always has the concept of a "current program file" which is set by the `LOAD <filename>` and `RUN <filename>` commands and cleared by the `NEW` command.
 * When you call `EDIT` without an explicit filename you are editing the actual file and not some representation in "flash" (as happens with the PicoMite) and any changes you make will be written to the file when the editor is exited, you do not need to explicitly use the `SAVE` command to persist the file from "flash" to the disk, infact MMB4L currently does not have a `SAVE` command.
 * Any command that operates on the "current program file" automatically reloads that program from disk before executing, i.e. `LIST`, `EDIT`, `RUN`. MMB4L never operates on a program that differs from that on disk.
 * When the source of an error is in a .INC file the `EDIT` command will open that file instead of the current .BAS file.
     * To explicitly open the current .BAS file use `EDIT CURRENT`

### Automatic path completion

At the MMB4L prompt pressing the TAB key will attempt to complete (as far as can be unambiguously determined) a filename that is currently being typed, or sound the console bell if no completion is possible.

### The "bang" command !

`!<command string>`

The `!` command may be used at the MMBasic prompt (but not within programs) as a short-cut for [SYSTEM](#system), e.g.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`!cp foo bar`

is translated to:

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`SYSTEM "cp foo bar"`

An exception is `!cd`, e.g.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`!cd foo`

is translated to:

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`CHDIR "foo"`

This is required because `!cd foo` would otherwise appear to do nothing due to the fact that each `SYSTEM` command is run in the context of a separate forked process instead of the MMBasic process itself.

_Note that the command string is passed verbatim as the first argument to the `SYSTEM` command and will not be evaluated by the interpreter for any variables, expressions or function calls._

### Other limitations

 * No GPIO commands/functions; this is only applicable to Raspberry Pi.
 * Supports `SETTICK` but not `SETTICK FAST`.
 * Since Linux is not a Real Time Operating System all timing commands such as `PAUSE` and `SETTICK` are subject to more error and variation than on microcontroller MMBasic implementations.
 * Paths are limited to 255 characters.
 * Arbitrary limit of 0.5 MB of program code and 1 MB of variable/other RAM.
 * Other limitations the alpha testers haven't told me about yet ...

## 11. How do I build MMB4L from source ?

 1. Clone the repository
    ```
    git clone --recursive https://github.com/thwill1000/mmb4l.git
    ```
 2. Run the build script
    ```
    ./build.sh
    ```
 3. Run MMBasic integration-tests:
    ```
    ( cd tests; ../build/build-release-<arch>/mmbasic ../sptools/sptest )
    ```
 4. Run "sptools" tests:
    ```
    ( cd sptools; ../build/build-release-<arch>/mmbasic sptest )
    ```

## 12. Credits

Obviously MMB4L would not have been possible without the work and generosity of Geoff Graham and Peter Mather.

The code was originally ported and is maintained by Thomas Hugo Williams.

Credit is also due to the denizens of [The Back Shed](https://www.thebackshed.com/forum/ViewForum.php?FID=16) forum including, but not limited to:
 * @Volhout - for test code for the MATH command/function.
 * @Mixtel90 - for coining the name MMB4L.

## 13. FAQ

**1. I heard rumours of Android and Chrome OS versions, do they exist ? can I have them ?**

I have previously compiled MMB4L to run under [Termux](https://termux.com/) and [Crostini](https://chromeos.dev/en/linux) but currently I am not providing pre-built distributions for these platforms and instead I am concentrating on supporting and extending the mainline Linux builds.

**2. How do I contact the maintainer ?**

Thomas "Tom" Hugo Williams can be contacted via:
 - https://github.com and as user "thwill1000"
 - [https://www.thebackshed.com](https://www.thebackshed.com/forum/ViewForum.php?FID=16) as user "thwill"
