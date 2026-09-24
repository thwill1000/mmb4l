' Copyright (c) 2025 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>

Option Explicit On

Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1")  > 0

#Include "../sptools/src/splib/system.inc"
#Include "../sptools/src/splib/array.inc"
#Include "../sptools/src/splib/list.inc"
#Include "../sptools/src/splib/string.inc"
#Include "../sptools/src/splib/file.inc"
#Include "../sptools/src/splib/vt100.inc"
#Include "../sptools/src/sptest/unittest.inc"

Const BASE% = Mm.Info(Option Base)

add_test("test_on_goto")
add_test("test_on_gosub")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub test_on_goto()
  assert_int_equals(0, on_goto%(0))
  assert_int_equals(10, on_goto%(1))
  assert_int_equals(20, on_goto%(2))
  assert_int_equals(30, on_goto%(3))
  assert_int_equals(0, on_goto%(4))
End Sub

Function on_goto%(i%)
  On i% Goto 110,120,130
  Exit Function
110
  on_goto% = 10
  Exit Function
120
  on_goto% = 20
  Exit Function
130
  on_goto% = 30
  Exit Function
End Function

Sub test_on_gosub()
  Dim on_gosub_rval%
  assert_int_equals(0, on_gosub%(0))
  assert_int_equals(10, on_gosub%(1))
  assert_int_equals(20, on_gosub%(2))
  assert_int_equals(30, on_gosub%(3))
  assert_int_equals(0, on_gosub%(4))
End Sub

Function on_gosub%(i%)
  on_gosub_rval% = 0
  On i% Gosub 210,220,230
  on_gosub% = on_gosub_rval%
  Exit Function
210
  on_gosub_rval% = 10
  Return
220
  on_gosub_rval% = 20
  Return
230
  on_gosub_rval% = 30
  Return
End Function
