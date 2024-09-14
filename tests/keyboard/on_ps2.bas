Option Base 0
Option Default None
Option Explicit

#Include "../../sptools/src/splib/system.inc"
#Include "../../sptools/src/splib/ctrl.inc"

Dim ctrl.scan_map%(31)
Restore ctrl.scan_map_data
Dim i%
For i% = Bound(ctrl.scan_map%(), 0) To Bound(ctrl.scan_map%(), 1)
  Read ctrl.scan_map%(i%)
Next

Graphics Window 0, 640, 400
Graphics Write 0
Text 320, 200, "With window focused press any key", CM, 4

On Ps2 ps2_interrupt
Do : Loop

Sub ps2_interrupt()
  Const ps2% = Mm.Info(PS2)
  Local key% = 0
  Const key_up% = ((ps2% And &hFFFF00) = &hE0F000) Or ((ps2% And &hFF00) = &hF000)

  Select Case ps2%
    Case < &hE000 : key% = Peek(Var ctrl.scan_map%(), ps2% And &hFF)
    Case < &hF000 : key% = Peek(Var ctrl.scan_map%(), (ps2% And &hFF) + &h80)
    Case < &hE0F000 : key% = Peek(Var ctrl.scan_map%(), ps2% And &hFF)
    Case Else : key% = Peek(Var ctrl.scan_map%(), (ps2% And &hFF) + &h80)
  End Select

  Local key_string$ = Chr$(key%)
  If key% < 32 Or key% > 127 Then key_string$ = "0x" + Hex$(key%, 2)
  Select Case key%
    Case &h08: key_string$ = "BACKSPACE"
    Case &h09: key_string$ = "TAB"
    Case &h0A: key_string$ = "ENTER"
    Case &h1B: key_string$ = "ESCAPE"
    Case &h20: key_string$ = "SPACE"
    Case &h7F: key_string$ = "DELETE"
    Case &h84: key_string$ = "INSERT"
    Case &h80: key_string$ = "UP"
    Case &h81: key_string$ = "DOWN"
    Case &h82: key_string$ = "LEFT"
    Case &h83: key_string$ = "RIGHT"
    Case &h86: key_string$ = "HOME"
    Case &h87: key_string$ = "END"
    Case &h88: key_string$ = "PAGE UP"
    Case &h89: key_string$ = "PAGE DOWN"
    Case &h8B: key_string$ = "ALT"
    Case &h8C: key_string$ = "SCROLL LOCK"
    Case &h91: key_string$ = "F1"
    Case &h92: key_string$ = "F2"
    Case &h93: key_string$ = "F3"
    Case &h94: key_string$ = "F4"
    Case &h95: key_string$ = "F5"
    Case &h96: key_string$ = "F6"
    Case &h97: key_string$ = "F7"
    Case &h98: key_string$ = "F8"
    Case &h99: key_string$ = "F9"
    Case &h9a: key_string$ = "F10"
    Case &h9b: key_string$ = "F11"
    Case &h9c: key_string$ = "F12"
  End Select

  ' PS2 codes not in the "scan map".
  Select Case ps2%
    Case &h58, &hF058: key_string$ = "CAPS LOCK"
    Case &h14, &hF014: key_string$ = "LCTRL"
    Case &h12, &hF012: key_string$ = "LSHIFT"
    Case &h77, &hF077: key_string$ = "NUM LOCK"
    Case &hE11477E1F014E077: key_string$ = "PAUSE"
    Case &hE014, &hE0F014: key_string$ = "RCTRL"
    Case &h59, &hF059: key_string$ = "RSHIFT"
  End Select

  ' MMBasic maps both ALT keys to the same code.
  If key_string$ = "ALT" Then
    Select Case ps2%
      Case &hE011, &hE0F011: key_string$ = "RALT"
      Case &h011, &hF011: key_string$ = "LALT"
      Case Else
        Error "Invalid state"
    End Select
  EndIf

  ? "PS2 Interrupt fired: 0x" + Hex$(ps2%, 8) + " " + key_string$ + " - " + Choice(key_up%, "keyup", "keydown")
End Sub
