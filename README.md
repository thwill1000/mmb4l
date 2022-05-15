# MMBasic for Linux (MMB4L)

1. [Introduction](#1-introduction)
2. [How do I install it ?](#2-how-do-i-install-it-)
3. [How do I run it ?](#3-how-do-i-run-it-)
    * [Start with a shebang #!](#start-with-a-shebang-)
4. [How do I use it ?](#4-how-do-i-use-it-)
5. [The EDITor](#5-the-editor)
    * [Configuring GNU nano](#configuring-gnu-nano)
    * [Using GNU nano](#using-gnu-nano)
6. [Differences from MMBasic 5.07.03 for the Colour Maximite 2](#6-differences-from-mmbasic-50703-for-the-colour-maximite-2)
7. [MMB4L specific extensions to MMBasic](#7-mmb4l-specific-extensions-to-mmbasic)
    * [The "bang" command !](#the-bang-command-)
    * [CHR$](#chr)
    * [CONSOLE](#console)
    * [END](#end)
    * [ERROR](#error)
    * [JSON$()](#json)
    * [MM.INFO()](#mminfo)
    * [OPEN](#open)
    * [OPTION CODEPAGE](#option-codepage)
    * [OPTION EDITOR](#option-editor)
    * [OPTION F\<num>](#option-fnum)
    * [OPTION LIST](#option-list)
    * [OPTION LOAD](#option-load)
    * [OPTION RESET](#option-reset)
    * [OPTION SAVE](#option-save)
    * [PEEK](#peek)
    * [POKE](#poke)
    * [PRINT](#print)
    * [QUIT](#quit)
    * [READ](#read)
    * [RESTORE](#restore)
    * [SYSTEM](#system)
    * [XMODEM](#xmodem)
8. [Limitations](#8-limitations)
9. [How do I build MMB4L from source ?](#9-how-do-i-build-mmb4l-from-source-)
10. [Credits](#10-credits)
11. [FAQ](#11-faq)

## 1. Introduction

MMB4L is a port of Geoff Graham's [MMBasic](https://mmbasic.com/) interpreter to the Linux platform (and its derivatives).

It was originally derived with permission from:
 * [MMBasic for DOS](https://geoffg.net/WindowsMMBasic.html)
     * Copyright 2011-2022 Geoff Graham<br/>

But also incorporates code and ideas from several other MMBasic ports:
 * [MMBasic for the PicoMite](https://geoffg.net/picomite.html) and [PicoMite VGA](https://geoffg.net/picomitevga.html)
     * Copyright 2011-2022 Geoff Graham
     * Copyright 2016-2022 Peter Mather
     * https://github.com/UKTailwind/PicoMite
     * https://github.com/UKTailwind/PicoMite-VGA-Edition<br/>
 * [MMBasic for the Colour Maximite 2](https://geoffg.net/maximite.html)
     * Copyright 2011-2022 Geoff Graham
     * Copyright 2016-2022 Peter Mather<br/>
 * MMBasic for Windows
     * Copyright 2011-2022 Geoff Graham
     * Copyright 2016-2022 Peter Mather
     * https://github.com/UKTailwind/MMB4W
 * Mothballed Pi-cromite project by Peter Mather.

What little MMB4L specific code there is, is Copyright 2021-2022 Thomas Hugo Williams.

MMB4L is an open-source project distributed under a modified 4-clause BSD license, see the [LICENSE.MMBasic](LICENSE.MMBasic) file for details.

## 2. How do I install it ?

MMB4L is in the early alpha phase so there are no fancy installers yet:

 1. Download the .tgz for the desired platform from the [latest release](https://github.com/thwill1000/mmb4l/releases/latest):
    * For 64-bit Linux running on Intel/AMD use the 'x86_64' build.
    * For 32-bit Linux running on Intel/AMD use the 'i686' build.
    * For 32-bit Raspbian/Linux running on Raspberry Pi ("Buster") use the 'armv6l' build.

 2. Extract the archive into a temporary location:
    * ```mkdir -p ~/tmp && tar -xf mmb4l-<version>.tgz --directory ~/tmp```

 3. Copy the "mmbasic" executable to "/usr/local/bin":
    * ```sudo cp ~/tmp/mmb4l-<version>/mmbasic /usr/local/bin```
    * or copy it where you like and create a symbolic link from "/usr/local/bin/mmbasic"

 4. The .tgz also contains resource files for [configuring the nano editor](#configuring) for use within MMB4L.

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

### Start with a shebang #!

You can use MMB4L to write executable scripts by starting them with a `#!` and the path to the `mmbasic` executable, e.g.

Write "hello_world.bas":
```
#!/usr/local/bin/mmbasic
Do
  Print "Hello World"
Loop
```
Make it executable:

`chmod 755 "hello_world.bas"`

And run it from the Linux shell:

`./hello_world.bas`

## 4. How do I use it ?

Perhaps one day there will be a user manual specific to MMB4L, until then you are directed to:
 * [Colour Maximite 2 User Manual](https://geoffg.net/Downloads/Maximite/Colour_Maximite_2_User_Manual.pdf)
 * [Programming with the Colour Maximite 2](https://geoffg.net/Downloads/Maximite/Programming_with_the_Colour_Maximite_2.pdf)
 * This README which (hopefully) documents all significant differences between the behaviour of MMB4L and that of MMBasic on the Colour Maximite 2 (CMM2).

## 5. The EDITor

Unlike other MMBasic platforms the ```EDIT``` command for MMB4L does not use a bespoke editor but instead relies on a third-party editor being installed. By default this is [GNU nano](https://www.nano-editor.org/), but this can be changed using the the [OPTION EDITOR](#option-editor) command.

### Configuring GNU nano

 1. Check you have nano installed, and determine the version:
    * ```nano --version```
    * If nano is not installed then [install it](https://phoenixnap.com/kb/use-nano-text-editor-commands-linux).

 2. The "mmb4l-\<version>.tgz" file contains resource files to configure nano to behave similarly to the Colour Maximite 2's integrated editor:
    * If you are running nano 4.8+:
      * ```
        mkdir -p ~/.mmbasic
        cp ~/tmp/mmb4l-<version>/mmbasic.nanorc ~/.mmbasic
        cp ~/tmp/mmb4l-<version>/mmbasic.syntax.nanorc ~/.mmbasic
        ```

    * If you have an earlier version and do not need to use it for anything else:
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

Where not overridden by the above the [default nano keyboard bindings](https://www.nano-editor.org/dist/latest/cheatsheet.html) apply, so you can, for example, use **Tab** to indent, **Shift+Tab** to unindent, and **Alt+3** toggle the comments of a selection.

**Gotchas:**

 1. Unlike other MMBasic version there is no key combination to automatically **Run** a program from the editor.
 2. If you rename a file whilst saving it MMB4L will not update its "current stored program" state and will still be using the previous file.

## 6. Differences from MMBasic 5.07.03 for the Colour Maximite 2

The principle difference between MMB4L and the Colour Maximite 2 is the lack of commands/functions for high-resolution graphics, sound and GPIO.

* Unlike the CMM2 there is a ```LOAD program_file$``` command which updates the "current stored program".
    * Note that any command that operates on the "current stored program" automatically reloads that program from disk before executing, i.e. `LIST`, `EDIT`, `RUN`. MMB4L never operates on a program that differs from that on disk.
    * As with the CMM2 there is no `SAVE program_file` command.
* By default the functions `HRES`, `VRES`, `MM.INFO(HPOS)`, `MM.INFO(HRES)`, `MM.INFO(VPOS)` and `MM.INFO(VRES)` return values in character rather than pixel resolution.
    * `OPTION CONSOLE PIXEL` changes this to use pixel resolution based on a nominal 8x12 font.
    * `OPTION CONSOLE CHARACTER` returns to using character resolution.
* When the source of an error is in a .INC file the `EDIT` command will open that file instead of the current .BAS file.
     * To explicitly open the current .BAS file use `EDIT CURRENT`
* The `RUN` command accepts a string expression, e.g. `RUN s1$ + s2`
     * However any optional command-line is still passed verbatim / without evaluation to `MM.CMDLINE`
         * e.g. `RUN "foo.bas", this_expression$ + will% + not_be_evaluated!`
         * as on the CMM2 the `EXECUTE` command can be used to workaround this limitation.
* As is the default for most BASICs the `FILES` command lists the contents of the current directory.
     * The CMM2 is "non-standard" in this respect and instead shows a TUI file manager.
* There is no `LS` command, use `FILES` or for more flexibility `!ls`.
* Also see [Limitations](#8-limitations).

## 7. MMB4L specific extensions to MMBasic

MMB4L implements a small number of extensions to MMBasic 5.07.03:

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

### CHR$()

`CHR$(UTF8 codepoint%)`

Given a Unicode code point this extension to the CHR$() function will return a 1-4 character string encoding that character in UTF-8 format.
    
_Note that printing such characters is not compatible with `OPTION CODEPAGE` being set.__

### CONSOLE

The CONSOLE command manipulates the console/terminal using ANSI escape-codes:

 * `CONSOLE BACKGROUND {<colour_name>|colour%} `
     * Sets the background colour for future `PRINT` commands.
     * Allowed colours and equivalent integers are:
         * 0 = Black
         * 1 = Blue
         * 2 = Green
         * 3 = Cyan
         * 4 = Red
         * 5 = Magenta or Purple
         * 6 = Yellow
         * 7 = White

 * `CONSOLE BELL`
     * Sounds the console "bell".

 * `CONSOLE CLEAR`
     * Clears the console using the current background colour AND moves the cursor o the origin (0, 0) *.
     * This is synonymous with the top-level `CLS` command.

 * `CONSOLE GETCURSOR x%, y%`
     * Reads the cursor position into the `x%` and `y%` variables.
     * `x%` and `y%` are character coordinates with origin (0, 0).

 * `CONSOLE GETSIZE width%, height%`
     * Reads the current console size into the `width%` and `height%` variables.
     * `width%` and `size%` are in characters.

 * `CONSOLE FOREGROUND {[BRIGHT] <colour_name> | colour%}`
     * Sets the foreground colour for future `PRINT` commands.
     * Allowed colours are the same as for `CONSOLE BACKGROUND` but with an optional `BRIGHT` attribute:
         * 8 = Grey (Bright Black)
         * 9 = Bright Blue
         * 10 = Bright Green
         * 11 = Bright Cyan
         * 12 = Bright Red
         * 13 = Bright Magenta or Bright Purple
         * 14 = Bright Yellow
         * 15 = Bright White

 * `CONSOLE HIDECURSOR [ON | TRUE | OFF | FALSE | z%]`
     * Hides or shows the cursor; without any argument this hides the cursor.

 * `CONSOLE HOME`
     * Moves the cursor to the origin (0, 0) *.

 * `CONSOLE {INVERSE|INVERT} [ON | TRUE | OFF | FALSE | z%]`
     * Inverts the foreground and background colours.

 * `CONSOLE RESET`
     * Resets the foreground, background and invert attributes.

 * `CONSOLE SETCURSOR x%, y%`
     * Moves the cursor to (`x%`, `y%`) *.
     * `x%` and `y%` are character coordinates with origin (0, 0).

 * `CONSOLE SETSIZE [ATLEAST] width%, height%`
    * Sets the console size.
    * If the optional `ATLEAST` modifier is used then this will not reduce the console width or height.
    * This works by sending an XTerm control sequence and may not succeed with all Linux terminal programs.

 * `CONSOLE SETTITLE title$`
     * Sets the console title.

 * `CONSOLE SHOWCURSOR [ON | TRUE | OFF | FALSE | z%]`
     * Shows or hides the cursor; without any argument this shows the cursor.

_\* Note that all PRINTing starts at and moves the current cursor position._

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
    * 1024+ - recommended range for program specific errors.

### JSON$()

`JSON$(array%(), string$ [, flags%])`

Returns a string representing a specific item out of the JSON input stored in the longstring `array%()`.
 * The optional flags parameter (default 0) specifies how explicit nulls and missing values should be handled:
     * &b00 - return empty strings for both.
     * &b01 - return `"<null>"` for explicit null.
     * &b10 - return `"<missing>"` for missing value.
     * &b11 - return both `"<null>"` and `"<missing>"`.

### MM.INFO()

In MMB4L this function can return values for these additional properties:

 * `MM.INFO$(ARCH)`
     * What architecture/platform is MMB4L running on, currently one of:
         * "Android aarch64"
         * "Linux aarch64"
         * "Linux armv6l"
         * "Linux i686"
         * "Linux x86_64"
     * Note that `MM.INFO$(DEVICE)` will return "MMB4L" for all of these.

 * `MM.INFO$(ENVVAR name$)`
     * Gets the value of the named environment variable, or the empty string if there is no such environment variable.

 * `MM.INFO(EXISTS path$)`
     * Does the file / directory / device referred to by `path$` exist ?

 * `MM.INFO(EXISTS SYMLINK path$)`
     * Does `path$` refer to a symbolic link ?

 * `MM.INFO(EXITCODE)`
     * Gets the exit code "returned" by the last program run:
         * If an `END` command is executed then this will be the optional exit code (default 0) specified to that command.
         * If a program runs to its "natural end" without an explicit `END` command this will be 0.
         * If a program is interrupted by the break key combination (default CTRL-C) this will be 130.
         * If an unhandled `ERROR` occurs this will be 1 (this may change).
         * If a ```NEW``` command is executed this will be 0.
 
 * `MM.INFO$(OPTION <option>)`
     * Gets the value of the named option, this is supported for all options.

### OPEN

_WARNING! MMB4L serial communications are still very much a work in progress and are known to be unnecessarily slow and flakey._

`OPEN comspec$ AS [#]fnbr`

Opens a serial communications port for reading and writing.

The `comspec$` has the format:

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`"device: baudrate, buffer-size, interrupt, interrupt-trigger, 7BIT, {EVEN | ODD}, S2, RTSCTS, XONXOFF"`.

**Arguments:**

The following arguments must be specified in order. If any argument is left out then all the following arguments must also be left out and will use their default values.
 * device - TTY device path
      * e.g. /dev/ttyACM0, /dev/ttyACM1, /dev/ttyS0, /dev/ttyS1, /dev/ttyUSB0, /dev/ttyUSB1, etc.
      * This is the only argument which is obligatory.
 * baudrate - one of:
      *  50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 500000, 576000, 921600, 1000000, 1152000, 1500000, 2000000, 2500000, 3000000, 3500000, 4000000.
      * Baudrates above 115.2K are theoretically supported by MMB4L but possibly not by any given Linux device driver, it may just fail to receive/transmit without reporting an actual error.
      * Default is 9600.
      * Arbitrary baudrates are not supported.
 * buffer-size - receive buffer size in bytes.
      * Default is 4096 bytes.
 * interrupt - user defined subroutine which will be called when the serial port has received some data.
      * Default is no interrupt.
 * interrupt-trigger - the interrupt subroutine is called when the receive buffer contains this number of bytes or greater.
      * Default is 1 byte.

**Flags:**

The following flags be specified in any order.
 * 7BIT - use 7 bit transmit/receive, or 8 bit if using a parity bit.
 * ODD | EVEN - use odd or even parity bit.
     * This will transmit/receive 9 bits unless 7BIT is also specified in which case it will transmit/receive 8 bits.
     * Inline with other MMBasic platforms MMB4L does not actually perform parity checking on received data.
 * S2 - use two stop bits following each character.
 * RTSCTS - enable RTS/CTS hardware flow control.
     * *At least in theory, this is untested.*
 * XONXOFF - enable XON/XOFF software flow control.
     * _WARNING! not currently working._

MMB4L does not support the following CMM2 flags: DEP, DEN, INV or OC.

**IMPORTANT!**

In order to open a serial port MMBasic must be running as a user that is a member of the `dialout` group.
```
sudo addgroup <user> dialout
```
Note that this change requires the user to logout and then login again to take effect.

### OPTION CODEPAGE

`OPTION CODEPAGE {<page> | page$}`

Causes character bytes in the range 128-255 to be translated to Unicode (UTF-8) when printed to the console according to a given code page / mapping.

Supported code pages are:
 * NONE - no mapping, characters 128-255 are written as-is.
 * CMM2 - best effort mapping to the characters of the CMM2 font.
 * CP437 - characters 128-255 of the original IBM PC code 437. Note this was a non-ANSI code page which also included characters 1-31 which are not available in this mapping, but see "MMB4L".
 * CP1252 - the classic Windows-1252 Latin alphabet code page.
 * MMB4L - as "CP437" but with characters 129-159 replaced by the original non-ANSI CP437 characters 1-31 and with additional "useful" characters in positions 128, 161-170 and 255.

### OPTION EDITOR

`OPTION EDITOR {<editor> | editor$}`

Controls which editor is used by the EDIT command.

Supported editors are:
 * ATOM
 * CODE     (synonym for VSCODE)
 * DEFAULT  (synonym for NANO)
 * GEANY
 * EDIT
 * LEAFPAD
 * NANO
 * SUBLIME
 * VI
 * VIM
 * VSCODE
 * XED

Alternatively a string expression can be provided which either evaluates to one of the editors listed above or explicitly specifies a Linux shell command to execute to launch an editor, e.g.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`OPTION EDITOR "code -g ${file}:${line}"`

Where:
 * `${file}` is a placeholder for the filename to edit, the filename will be automatically enclosed in double-quotes.
 * `${line}` is a placeholder for the line number to set the initial cursor position to.

### OPTION F\<num>

`OPTION {F1 .. F12} string$`

Defines the string that will be generated when the given function key is pressed at the BASIC command prompt.
   * Unlike the CMM2 all of F1-F12 may be redefined by the user.
   * _WARNING! depending on the window manager being used some function key presses may be captured and not passed on to MMBasic._

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

### PEEK

`pos% = PEEK(DATAPTR)`

Gets the current value of the "virtual pointer" used to track where the [READ](#read) command reads `DATA` from.
 * This value is opaque and whilst it can be stored it should not be manipulated other than passing it to the `POKE DATAPTR ptr%` command.

### POKE

`POKE DATAPTR ptr%`

Sets the value of the "virtual pointer" used to track where the [READ](#read) command reads `DATA` from.
 * Only values of `ptr%` previously retrieved by calling `PEEK(DATAPTR)` should be passed to this command.

### PRINT

`PRINT @(x%, y%) expression`

Outputs text to the console/terminal at a given character position followed by a carriage return/newline pair.
 * Unlike the CMM2 `x%` and `y%` are both obligatory and in character (not pixel) coordinates. There is no `mode` parameter.
 * It is equivalent to `CONSOLE SETCURSOR x, y : PRINT expression`

### QUIT

`QUIT [exit_code%]`

Exits MMB4L returning an optional exit code (default 0) to the shell.
 * The exit code should be between 0 and 255.
 * A value of 0 indicates success and a value of 1 indicates a general error.
 * Linux has no hard standard for other values, but for guidance see:
     * [Advanced Bash Scripting Guide](https://tldp.org/LDP/abs/html/exitcodes.html)
     * [sysexits - FreeBSD](https://www.freebsd.org/cgi/man.cgi?query=sysexits&apropos=0&sektion=0&manpath=FreeBSD+4.3-RELEASE&format=html)

### READ

`READ {SAVE | RESTORE}`

The standard behaviour of the `READ variable [, variable] ...` command is to read data from a global "virtual pointer". When a program starts this pointer is initialised to point at the first item of the program's first `DATA` statement and can be reset using the `RESTORE` command.

Because this pointer is global, a subroutine or function that wishes to safely access its own `DATA` without interfering with data reads being performed by its caller should use the `READ SAVE` and `READ RESTORE` commands to save and restore the value of the pointer before and after doing so. In MMB4L 50 levels of nested `READ SAVE` and `READ RESTORE` are allowed.

e.g.

```
SUB foo()
  READ SAVE     ' Save the current value of the pointer.
  LOCAL i%, f!, s$
  RESTORE foo_data
  READ i%, f!, s$
  READ RESTORE  ' Restore the pointer to the previously saved value.
END SUB

foo_data:
DATA 42, 3.142, "Hello World"
```

_Note that this extension to the `READ` command is expected to become standard in MMBasic 5.03.04 implementations._

Also see: [PEEK(DATAPTR)](#peek) and [POKE DATAPTR ptr%](#poke).

### RESTORE

`RESTORE [<label> | label$ | line%]`

Resets the "virtual pointer" (line and position counters) for the `READ` statement.
   * With no argument the pointer is reset to the first `DATA` statement in the program.
   * With an explicit `<label>` or string-expression the pointer is reset to the first `DATA` statement following the corresponding label.
   * With an integer-expression the pointer is reset to the first `DATA` statement following the corresponding line number.

### SYSTEM

`SYSTEM cmd$ [, output$ [, exit_status%]]`\
`SYSTEM cmd$ [, output%() [, exit_status%]]`

Executes the Linux operating system command specified by `cmd$`.
 * To capture the output from the operating system command use an optional string `output$` or long string `output%()` parameter.
 * Otherwise output will appear on the console (stdout).
 * Output can also be directed to a file using standard Linux notation.
 * An MMBasic error is only reported if the operating system command cannot actually be executed. If the command executes but fails then it's exit status can be captured by using the optional `exit_status%` parameter.

`SYSTEM GETENV name$, {value$ | value%()}`

Gets the value of an environment variable `name$` into either a string `value$` or long string `value%()` parameter.
 * This supplements `MM.INFO$(ENVVAR)` because it allows values > 255 characters to be read into a long string.

`SYSTEM SETENV name$, {value$ | value%()}`

Sets the value of an environment variable `name$` from either a string `value$` or long string `value%()` parameter.

### XMODEM

`XMODEM {SEND | RECEIVE} file$, [#]fnbr`

Transfers a file to/from  a remote computer using the [XMODEM protocol](https://en.wikipedia.org/wiki/XMODEM).
 * `#fnbr` must first have been `OPEN`ed as a serial port.
 * Unlike other MMBasic platforms MMB4L cannot use `XMODEM` to send to or receive from the console.

## 8. Limitations

 * No high-resolution graphics commands/functions.
 * No sound commands/functions.
 * No GPIO commands/functions; this is only applicable to Raspberry Pi.
 * Supports `SETTICK` but not `SETTICK FAST`.
 * Since Linux is not a Real Time Operating System all timing commands such as `PAUSE` and `SETTICK` are subject to more error and variation than on microcontroller MMBasic implementations.
 * Paths are limited to 255 characters.
 * Arbitrary limit of 0.5 MB of program code and 1 MB of variable/other RAM.
 * Other limitations the alpha testers haven't told me about yet ...

## 9. How do I build MMB4L from source ?

 1. Clone the repository
    ```
    git clone git@github.com:thwill1000/mmb4l.git
    ```
 2. Initialise and update the "sptools/" submodule
    ```
    cd mmb4l/sptools
    git submodule init
    git submodule update
    cd ..
    ```
 3. Configure a build directory
    ```
    mkdir build-debug
    cd build-debug
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    ```
 4. Run make
    ```
    make
    ```
 5. Run unit-tests:
    ```
    ctest
    ```
 6. Run standard integration-tests:
    ```
    ./mmbasic ../tests/run_tests
    ```
 7. Run "sptools" integration-tests:
    ```
    cd ../sptools
    ../build-debug/mmbasic sptest
    ```

## 10. Credits

Obviously MMB4L would not have been possible without the work and generosity of Geoff Graham and Peter Mather.

The code was originally ported and is maintained by Thomas Hugo Williams.

Credit is also due to the denizens of [The Back Shed](https://www.thebackshed.com/forum/ViewForum.php?FID=16) forum including, but not limited to:
 * @Volhout - for test code for the MATH command/function.
 * @Mixtel90 - for coining the name MMB4L.

## 11. FAQ

**1. I heard rumours of Android and Chrome OS versions, do they exist ? can I have them ?**

I have compiled MMB4L to run under [Termux](https://termux.com/) and [Crostini](https://chromeos.dev/en/linux) but I am not providing pre-build distributions for these platforms at this time so I can concentrate on resolving issues with the mainline Linux builds.

**2. How do I contact the maintainer ?**

Thomas "Tom" Hugo Williams can be contacted via:
 - https://github.com and as user "thwill1000"
 - [https://www.thebackshed.com](https://www.thebackshed.com/forum/ViewForum.php?FID=16) as user "thwill"
