Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then Option Simulate "Game*Mite"

Const ctrl.OPEN = -1
Const ctrl.CLOSE = -2
Const ctrl.SOFT_CLOSE = -3

Dim cmd$
Dim gamepad_present%(1)

Console Clear
Cls ' Required to show the graphics window.

On Error Ignore
ctrl.gamemite(ctrl.OPEN)
If Not Mm.ErrNo Then gamepad_present%(1) = 1
On Error Abort
print_gamepad(1)

Do
  Console HideCursor
  update_gamepad(1)
  Pause 100
  Console ShowCursor
Loop

Sub print_gamepad(id%)
  Const y% = (id% - 1) * 8
  Print @(0, y%) "*** Focus must be be on Game*Mite window ***"
  Print
  Print "Game*Mite - " + Choice(gamepad_present%(id%), "PRESENT", "NOT PRESENT")
  Print "Buttons: ########"
End Sub

Sub update_gamepad(id%)
  If Not gamepad_present%(id%) Then Exit Sub

  Const y% = (id% - 1) * 8 + 1
  Local btn%
  ctrl.gamemite(btn%)
  Const b$ = str.rpad$(buttons_as_string$(btn%), 40)

  Print @(9, y% + 2) b$
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

Sub ctrl.gamemite(x%)
  Select Case x%
    Case >= 0
      x% = Port(GP12,2,GP11,2,GP8,1,GP8,1,GP11,1,GP10,1,GP9,1,GP13,3,GP13,3)
      x% = (x% Xor &h7FFF) And &h29EA
      Exit Sub
    Case ctrl.OPEN
      Local i%
      For i% = 8 To 15 : SetPin Mm.Info(PinNo "GP" + Str$(i%)), Din, PullUp : Next
      ' ctrl.open_driver("ctrl.gamemite")
    Case ctrl.CLOSE, ctrl.SOFT_CLOSE
      Local i%
      For i% = 8 To 15 : SetPin Mm.Info(PinNo "GP" + Str$(i%)), Off : Next
      ' ctrl.close_driver("ctrl.gamemite")
  End Select
End Sub
