' Copyright (c) 2023 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 5.07

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

add_test("Test DIM var with same name as SUB", "test_dim_given_sub")
add_test("Test DIM var with same name as FUNCTION", "test_dim_given_function")
add_test("Test DIM var with same name as label", "test_dim_given_label")
add_test("Test LOCAL var with same name as SUB", "test_local_given_sub")
add_test("Test LOCAL var with same name as FUNCTION", "test_local_given_function")
add_test("Test LOCAL var with same name as label", "test_local_given_label")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub sub_foo()
End Sub

Function fun_foo%()
End Function

foo_label:

Sub test_dim_given_sub()
  On Error Skip 1
  Dim sub_foo%
  If Mm.Device$ = "MMB4L" Then
    assert_raw_error("A function/subroutine has the same name: SUB_FOO")
  Else
    assert_raw_error("A sub/fun has the same name: SUB_FOO")
  EndIf
End Sub

Sub test_dim_given_function()
  On Error Skip 1
  Dim fun_foo%
  If Mm.Device$ = "MMB4L" Then
    assert_raw_error("A function/subroutine has the same name: FUN_FOO")
  Else
    assert_raw_error("A sub/fun has the same name: FUN_FOO")
  EndIf
End Sub

Sub test_dim_given_label()
  Dim foo_label% = 42
  assert_int_equals(42, foo_label%)
End Sub

Sub test_local_given_sub()
  On Error Skip 1
  Local sub_foo%
  If Mm.Device$ = "MMB4L" Then
    assert_raw_error("A function/subroutine has the same name: SUB_FOO")
  Else
    assert_raw_error("A sub/fun has the same name: SUB_FOO")
  EndIf
End Sub

Sub test_local_given_function()
  On Error Skip 1
  Local fun_foo%
  If Mm.Device$ = "MMB4L" Then
    assert_raw_error("A function/subroutine has the same name: FUN_FOO")
  Else
    assert_raw_error("A sub/fun has the same name: FUN_FOO")
  EndIf
End Sub

Sub test_local_given_label()
  Local foo_label% = 42
  assert_int_equals(42, foo_label%)
End Sub
