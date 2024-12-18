# TODO

 - Add MATH AES128 command from PicoMite
 - Add extended END command from PicoMite
 - Look at initialising audio/graphics/gamepads all together and inserting a pause after audio initialisation to see if that helps with audio glitches; beware of messing up TICK interrupt timing, perhaps need an explicit GFX INIT command
 - Add functionality to allow Gamepad compatibility to be configured, e.g. SNES vs. NES vs. Atari JS for PicoMiteVGA
 - Change program_apply_replacement() so that it does not replace within comments (single + multi-line) or within quoted strings
 - Correctly support sprites on multiple different surfaces, e.g.
     - restore background on correct surface
     - collisions should only occur between sprites on same surface
 - Use a bigger sample buffer (currently only 1 sample) in "audio.c"
 - Add ability to direct ``TRACE`` to a file
 - Add ability to call native "C" shared objects
 - Support "long lines" at the MMBasic prompt
 - Add PicoMite EDITor implementation
     - and add support for "long lines" to it
 - Add '.' command that behaves like a combination of '*' and '!', first looking for a .bas file to run and then trying a ``SYSTEM`` command.
 - Don't include line number and filename in error message string; still show them at prompt where appropriate and make them available via ``MM.INFO()``
 - Make default output from ``FILES`` more like that of CMM2
 - (?) Implement ``SAVE`` command
 - (?) Don't fake DOS drive specifications, e.g. A:/
 - (?) Remove use of 'goto' in 'path.c'
 - (?) Remove ``SETTITLE`` and ``CURSOR`` commands
 - (?) Rewrite "tools/glibc_check.sh" in MMBasic
 - Find fix for this bug:
```
    ' This will sometimes run forever,
    ' and occasionally do nothing because the interrupt never fires.

    Option Console Both
    Option Base 0
    Option Default None
    Option Explicit Off

    Timer = 0

    SetTick 5, my_interrupt
    Pause 10
    End

    Sub my_interrupt()
      ? "Interrupt entered"
      Print Timer
      Local i%
      For i% = 1 To 100000 : Next
      ? "Interrupt exited"
    End Sub
```
 - Remove 32K array size limit.
 - Reorder interrupt processing to match PicoMite, add this as a comment to the code:
      1. ON KEY individual
      2. ON KEY general
      3. GUI Int Down (not PicoMiteVGA)
      4. GUI Int Up (not PicoMiteVGA)
      5. ADC completion
      6. I2C Slave Rx
      7. I2C Slave Tx
      8. I2C2 Slave Rx
      9. I2C2 Slave Tx
      10. WAV Finished
      11. COM1: Serial Port
      12. COM2: Serial Port
      13. IR Receive
      14. Keypad
      15. Interrupt command/CSub Interrupt
      16. I/O Pin Interrupts in order of definition
      17. Tick Interrupts (1 to 4 in that order)
  - Consider options for handling of control characters in ``MMgetline()``
  - Read from serial ports directly into RxBuf
      - i.e. not via a temporary buffer
  - Buffer serial port writes
  - Long running commands (``AUTOSAVE``, ``XMODEM``) should call background processing
  - Perform background processing in different thread
  - Implement ``LOAD FONT``
  - Tidy up LED snake example
  - Implement ``CONSOLE BLINK``, ``BOLD``, ``FAINT``, ``ITALIC``, ``REVERSE``, ``STRIKE``, ``UNDERLINE``
  - Write a proper lexer and ensure all Peter's "preprocessor tricks" work in both
      - programs, and
      - the command-line
  - Completely overhaul 'mmbasic.syntax.nanorc'
      - can we fix syntax highlighting for "strings" inside comments ?
  - Add option to ``PAUSE`` controlling whether it nanosleeps or not
      - or an ``OPTION`` to MMBasic controlling all sleep behaviour
  - Have another look at the other settings recommended for raw console mode
  - Work through other MMBasic manuals implementing missing commands/functions
  - Review all uses of void*
  - Implement safety checks on ``PEEK``/``POKE`` calls
      - should these be optional ?
      - should address alignment of ``PEEK``/``POKE`` be optional ?
  - Implement ``FORK``, ``WAIT`` and friends
  - Build on macOS
  - Extend preprocessor
  - Implement ``SWAP``
  - Integrate 'pigpio' with Raspberry Pi build
  - Fix bug where ``TRACE`` will not log a line that you ``GOTO``/``GOSUB`` using a line number, but will if you use a label
  - Can ``PRINT @(x,y)`` be implemented without using a function slot for @ ?
  - Implement line continuation character _
  - Implement exception handling ``TRY`` | ``CATCH`` | ``FINALLY`` | ``END TRY``
