Option Base 0
Option Default None
Option Explicit

Option Simulate "MMBasic for Windows"

Dim cmd$
Dim gamepad_present%(1)
Dim gamepad_interrupt%(1)

Cls

cmd$ = "Gamepad On on_gamepad1"
On Error Skip
Execute cmd$
If Not Mm.ErrNo Then gamepad_present%(1) = 1
print_gamepad(1)

Print
Print "A - Start rumble, B - Stop rumble, X - Interrupt cursors, Left - Interrupt all"

Do
  Console HideCursor
  update_gamepad(1)
  Pause 100
  Console ShowCursor
Loop

Sub print_gamepad(id%)
  Const y% = (id% - 1) * 8
  Print @(0, y%) "Gamepad " + Str$(id%) + " - " + Choice(gamepad_present%(id%), "PRESENT", "NOT PRESENT")
  Print "Buttons:                  ########"
  Print "Left  Analog Button:      ########"
  Print "Right Analog Button:      ########"
  Print "Left  Analog Stick:   x = ########, y = ########"
  Print "Right Analog Stick:   x = ########, y = ########"
End Sub

Sub update_gamepad(id%)
  If Not gamepad_present%(id%) Then Exit Sub

  Const y% = (id% - 1) * 8

  Const btn% = Gamepad(B)
  Const b$ = str.rpad$(buttons_as_string$(btn%), 40)
  Const l$ = str.lpad$(Str$(Gamepad(L)), 8)
  Const lx$ = str.lpad$(Str$(Gamepad(LX)), 8)
  Const ly$ = str.lpad$(Str$(Gamepad(LY)), 8)
  Const r$ = str.lpad$(Str$(Gamepad(R)), 8)
  Const rx$ = str.lpad$(Str$(Gamepad(RX)), 8)
  Const ry$ = str.lpad$(Str$(Gamepad(RY)), 8)

  Print @(22, y% + 1) b$
  Print @(26, y% + 2) l$
  Print @(26, y% + 3) r$
  Print @(26, y% + 4) lx$
  Print @(40, y% + 4) ly$
  Print @(26, y% + 5) rx$
  Print @(40, y% + 5) ry$
  Print @(0, y% + 6) Choice(gamepad_interrupt%(id%), "*INTERRUPT*", "           ")
  If gamepad_interrupt%(id%) > 0 Then Inc gamepad_interrupt%(id%), -1

  Local cmd$
  Select Case btn%
    Case &h100 ' Left
      Gamepad Off
      cmd$ = "Gamepad On on_gamepad" + Str$(id%)
      On Error Skip
      Execute cmd$
    Case &h400 ' X
      Gamepad Off
      cmd$ = "Gamepad On on_gamepad" + Str$(id%) + ", &b000000011110000"
      On Error Skip
      Execute cmd$
    Case &h800 ' A
      Gamepad Vibrate
    Case &h2000 ' B
      Gamepad Stop
  End Select
End Sub

Function str.lpad$(s$, x%)
  str.lpad$ = s$
  If Len(s$) < x% Then str.lpad$ = Space$(x% - Len(s$)) + s$
End Function

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
  buttons_as_string$ = s$
End Function

Sub on_gamepad(id%)
  gamepad_interrupt%(id%) = 10
End Sub

Sub on_gamepad1()
  on_gamepad(1)
End Sub
