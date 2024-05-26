Option Base 0
Option Default None
Option Explicit

Option Simulate "PicoMiteVGA"

Const ctrl.OPEN = -1
Const ctrl.CLOSE = -2
Const ctrl.SOFT_CLOSE = -3
Const ctrl.PULSE = 0.001
Const ctrl.R      = &h01
Const ctrl.START  = &h02
Const ctrl.SELECT = &h08
Const ctrl.L      = &h10
Const ctrl.DOWN   = &h20
Const ctrl.RIGHT  = &h40
Const ctrl.UP     = &h80
Const ctrl.LEFT   = &h100
Const ctrl.X      = &h400
Const ctrl.A      = &h800
Const ctrl.Y      = &h1000
Const ctrl.B      = &h2000

Dim cmd$
Dim gamepad_present%(2)

Console Clear

On Error Skip
snes_a(ctrl.OPEN)
If Not Mm.ErrNo Then gamepad_present%(1) = 1
print_gamepad(1)

On Error Skip
snes_b(ctrl.OPEN)
If Not Mm.ErrNo Then gamepad_present%(2) = 1
print_gamepad(2)

Do
  Console HideCursor
  update_gamepad(1)
  update_gamepad(2)
  Pause 100
  Console ShowCursor
Loop

Sub print_gamepad(id%)
  Const y% = (id% - 1) * 4
  Const controller$ = "PicoMiteVGA SNES " + Choice(id% = 1, "A", "B")
  Print @(0, y%) controller$ + " - " + Choice(gamepad_present%(id%), "PRESENT", "NOT PRESENT")
  Print "Buttons: ########"
End Sub

Sub update_gamepad(id%)
  If Not gamepad_present%(id%) Then Exit Sub

  Const y% = (id% - 1) * 4
  Local btn%
  If id% = 1 Then
    snes_a(btn%)
  Else
    snes_b(btn%)
  EndIf
  Const b$ = str.rpad$(buttons_as_string$(btn%), 40)

  Print @(9, y% + 1) b$
End Sub

Function str.rpad$(s$, x%)
  str.rpad$ = s$
  If Len(s$) < x% Then str.rpad$ = s$ + Space$(x% - Len(s$))
End Function

Function buttons_as_string$(btn%)
  Local s$
  If btn% And &h01 Then Cat s$, "R, "
  If btn% And &h02 Then Cat s$, "Start, "
  If btn% And &h04 Then Cat s$, "Home, "
  If btn% And &h08 Then Cat s$, "Select, "
  If btn% And &h10 Then Cat s$, "L, "
  If btn% And &h20 Then Cat s$, "Down, "
  If btn% And &h40 Then Cat s$, "Right, "
  If btn% And &h80 Then Cat s$, "Up, "
  If btn% And &h100 Then Cat s$, "Left, "
  If btn% And &h200 Then Cat s$, "ZR, "
  If btn% And &h400 Then Cat s$, "X, "
  If btn% And &h800 Then Cat s$, "A, "
  If btn% And &h1000 Then Cat s$, "Y, "
  If btn% And &h2000 Then Cat s$, "B, "
  If btn% And &h4000 Then Cat s$, "ZL, "
  If Len(s$) Then s$ = Left$(s$, Len(s$) - 2)
  buttons_as_string$ = Choice(Len(s$), s$, "None")
End Function

Sub snes_a(x%)
  Select Case x%
    Case >= 0
      Pulse GP2, ctrl.PULSE
      x% =    Not Pin(GP1) * ctrl.B      : Pulse GP3, ctrl.PULSE
      Inc x%, Not Pin(GP1) * ctrl.Y      : Pulse GP3, ctrl.PULSE
      Inc x%, Not Pin(GP1) * ctrl.SELECT : Pulse GP3, ctrl.PULSE
      Inc x%, Not Pin(GP1) * ctrl.START  : Pulse GP3, ctrl.PULSE
      Inc x%, Not Pin(GP1) * ctrl.UP     : Pulse GP3, ctrl.PULSE
      Inc x%, Not Pin(GP1) * ctrl.DOWN   : Pulse GP3, ctrl.PULSE
      Inc x%, Not Pin(GP1) * ctrl.LEFT   : Pulse GP3, ctrl.PULSE
      Inc x%, Not Pin(GP1) * ctrl.RIGHT  : Pulse GP3, ctrl.PULSE
      Inc x%, Not Pin(GP1) * ctrl.A      : Pulse GP3, ctrl.PULSE
      Inc x%, Not Pin(GP1) * ctrl.X      : Pulse GP3, ctrl.PULSE
      Inc x%, Not Pin(GP1) * ctrl.L      : Pulse GP3, ctrl.PULSE
      Inc x%, Not Pin(GP1) * ctrl.R      : Pulse GP3, ctrl.PULSE
      Exit Sub
    Case ctrl.OPEN
      SetPin GP1, Din : SetPin GP2, Dout : SetPin GP3, Dout
      Pin(GP2) = 0 : Pin(GP3) = 0
      snes_a(0) ' Discard the first reading.
      ' ctrl.open_driver("snes_a")
    Case ctrl.CLOSE, ctrl.SOFT_CLOSE
      SetPin GP1, Off : SetPin GP2, Off : SetPin GP3, Off
      ' ctrl.close_driver("snes_a")
  End Select
End Sub

Sub snes_b(x%)
  Select Case x%
    Case >= 0
      Pulse GP5, ctrl.PULSE
      x% =    Not Pin(GP4) * ctrl.B      : Pulse GP22, ctrl.PULSE
      Inc x%, Not Pin(GP4) * ctrl.Y      : Pulse GP22, ctrl.PULSE
      Inc x%, Not Pin(GP4) * ctrl.SELECT : Pulse GP22, ctrl.PULSE
      Inc x%, Not Pin(GP4) * ctrl.START  : Pulse GP22, ctrl.PULSE
      Inc x%, Not Pin(GP4) * ctrl.UP     : Pulse GP22, ctrl.PULSE
      Inc x%, Not Pin(GP4) * ctrl.DOWN   : Pulse GP22, ctrl.PULSE
      Inc x%, Not Pin(GP4) * ctrl.LEFT   : Pulse GP22, ctrl.PULSE
      Inc x%, Not Pin(GP4) * ctrl.RIGHT  : Pulse GP22, ctrl.PULSE
      Inc x%, Not Pin(GP4) * ctrl.A      : Pulse GP22, ctrl.PULSE
      Inc x%, Not Pin(GP4) * ctrl.X      : Pulse GP22, ctrl.PULSE
      Inc x%, Not Pin(GP4) * ctrl.L      : Pulse GP22, ctrl.PULSE
      Inc x%, Not Pin(GP4) * ctrl.R      : Pulse GP22, ctrl.PULSE
      Exit Sub
    Case ctrl.OPEN
      SetPin GP4, Din : SetPin GP5, Dout : SetPin GP22, Dout
      Pin(GP5) = 0 : Pin(GP22) = 0
      snes_b(0) ' Discard the first reading.
      ' ctrl.open_driver("snes_b")
    Case ctrl.CLOSE, ctrl.SOFT_CLOSE
      SetPin GP4, Off : SetPin GP5, Off : SetPin GP22, Off
      ' ctrl.close_driver("snes_b")
  End Select
End Sub
