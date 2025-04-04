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

add_test("test_if_with_line_num")
add_test("test_else_with_line_num")
add_test("test_if_goto")
add_test("test_if_then")
add_test("test_if_then_else")
add_test("test_if_then_if_then")
add_test("test_if_without_then")
add_test("test_if_with_errors")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub test_if_with_line_num()
  If 1 Then 100
  assert_fail("IF <condition> THEN <linenum> failed")
100
  assert_true(1)
End Sub

Sub test_else_with_line_num()
  Local fail% = 0
  If 0 Then fail% = 1 Else 110
  assert_fail("IF <condition> THEN <statement> ELSE <linenum> failed")
110
  assert_true(1)
End Sub

Sub test_if_goto()
  If 1 Goto 200
  assert_fail("IF <condition> GOTO <linenum> failed")
200
  assert_true(1)
End Sub

Sub test_if_then()
  ' Condition is TRUE.
  Local success% = 0
  If 1 Then success% = 1
  assert_true(success%, "IF <condition> THEN failed")

  ' Condition is FALSE.
  success% = 1
  If 0 Then success% = 0
  assert_true(success%, "IF <condition> THEN failed")
End Sub

Sub test_if_then_else()
  ' Condition is TRUE.
  Local success% = 0
  If 1 Then success% = 1 Else success% = 0
  assert_true(success%, "IF <condition> THEN <statement1> ELSE <statement2> failed")

  ' Condition is FALSE.
  success% = 0
  If 0 Then success% = 0 Else success% = 1
  assert_true(success%, "IF <condition> THEN <statement1> ELSE <statement2> failed")
End Sub

Sub test_if_then_if_then()
  If 1 Then If 1 Then assert_true(1) : Exit Sub
  assert_fail("IF <condition1> THEN IF <condition2> THEN failed")
End Sub

Sub test_if_without_then()
  On Error Ignore
  If 1 Print "Hello"
  assert_raw_error("IF without THEN")
  On Error Abort
End Sub

Sub test_if_with_errors()
  On Error Ignore
  If 0 Then Print "Hello" Else
  assert_raw_error("Syntax")
  On Error Abort

  On Error Ignore
  If 0 Then Print "Hello" Then Print "Goodbye"
  assert_raw_error("Syntax")
  On Error Abort
End Sub
