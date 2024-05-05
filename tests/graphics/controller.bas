Option Base 0
Option Default None
Option Explicit

Const GAMEPAD_ID = 2
Dim b$, l$, lx$, ly$, r$, rx$, ry$

Device Gamepad Open GAMEPAD_ID
Cls

Do
  b$ = str.lpad$(Str$(Device(Gamepad GAMEPAD_ID, B)), 6)
  l$ = str.lpad$(Str$(Device(Gamepad GAMEPAD_ID, L)), 6)
  lx$ = str.lpad$(Str$(Device(Gamepad GAMEPAD_ID, LX)), 6)
  ly$ = str.lpad$(Str$(Device(Gamepad GAMEPAD_ID, LY)), 6)
  r$ = str.lpad$(Str$(Device(Gamepad GAMEPAD_ID, R)), 6)
  rx$ = str.lpad$(Str$(Device(Gamepad GAMEPAD_ID, RX)), 6)
  ry$ = str.lpad$(Str$(Device(Gamepad GAMEPAD_ID, RY)), 6)

  Console Home
  Print "Buttons:                  "; b$
  Print "Left  Analog Button:      "; l$
  Print "Right Analog Button:      "; r$
  Print "Left  Analog Stick:   x = "; lx$; " y = "; ly$
  Print "Right Analog Stick:   x = "; rx$; " y = "; ry$

  Pause 100
Loop

Function str.lpad$(s$, x%)
  str.lpad$ = s$
  If Len(s$) < x% Then str.lpad$ = Space$(x% - Len(s$)) + s$
End Function
