Option Base 0
Option Default None
Option Explicit

Dim cmd$, id% ' id% = Wii i2c channel
Dim gamepad_present%(3)
Dim gamepad_interrupt%(3)

Option Simulate "Colour Maximite 2"

Cls

For id% = 1 To 3
  cmd$ = "Controller Classic Open id%, on_gamepad" + Str$(id%)
  On Error Skip
  Execute cmd$
  If Not Mm.ErrNo Then gamepad_present%(id%) = 1
  print_gamepad(id%)
Next

Print
Print "X - Interrupt cursors, Left - Interrupt all"

Do
  Console HideCursor
  For id% = 1 To 3
    update_gamepad(id%)
  Next
  Pause 100
  Console ShowCursor
Loop

Sub print_gamepad(id%)
  Const y% = (id% - 1) * 8
  Print @(0, y%) "Wii Classic I2C" + Str$(id%) + " - " + Choice(gamepad_present%(id%), "PRESENT", "NOT PRESENT")
  Print "Buttons:                  ########"
  Print "Left  Analog Button:      ########"
  Print "Right Analog Button:      ########"
  Print "Left  Analog Stick:   x = ########, y = ########"
  Print "Right Analog Stick:   x = ########, y = ########"
End Sub

Sub update_gamepad(id%)
  If Not gamepad_present%(id%) Then Exit Sub

  Const y% = (id% - 1) * 8

  Const btn% = Classic(B, id%)
  Const b$ = str.rpad$(buttons_as_string$(btn%), 40)
  Const l$ = str.lpad$(Str$(Classic(L, id%)), 8)
  Const lx$ = str.lpad$(Str$(Classic(LX, id%)), 8)
  Const ly$ = str.lpad$(Str$(Classic(LY, id%)), 8)
  Const r$ = str.lpad$(Str$(Classic(R, id%)), 8)
  Const rx$ = str.lpad$(Str$(Classic(RX, id%)), 8)
  Const ry$ = str.lpad$(Str$(Classic(RY, id%)), 8)

  Print @(22, y% + 1) b$
  Print @(26, y% + 2) l$
  Print @(26, y% + 3) r$
  Print @(26, y% + 4) lx$
  Print @(40, y% + 4) ly$
  Print @(26, y% + 5) rx$
  Print @(40, y% + 5) ry$
  Print @(0, y% + 6) Choice(gamepad_interrupt%(id%), "*INTERRUPT*", "           ")

  Local cmd$
  Select Case btn%
    Case &h100 ' Left
      Controller Classic Close id%
      cmd$ = "Controller Classic Open id%, on_gamepad" + Str$(id%)
      On Error Skip
      Execute cmd$
    Case &h400 ' X
      Controller Classic Close id%
      cmd$ = "Controller Classic Open id%, on_gamepad" + Str$(id%) + ", &b000000011110000"
      On Error Skip
      Execute cmd$

    ' Case &h800 ' A
    '   Device Gamepad Vibrate id%, &hFFFF, &hFFFF, 10000
    ' Case &h2000 ' B
    '   Device Gamepad Vibrate id%, Off
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
  gamepad_interrupt%(id%) = Not gamepad_interrupt%(id%)
End Sub

Sub on_gamepad1()
  on_gamepad(1)
End Sub

Sub on_gamepad2()
  on_gamepad(2)
End Sub

Sub on_gamepad3()
  on_gamepad(3)
End Sub
