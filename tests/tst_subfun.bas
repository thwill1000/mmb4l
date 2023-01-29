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

add_test("test_call_fn_as_sub")
add_test("test_call_sub_as_fn")
add_test("test_call_fn_with_wrong_type")
add_test("test_assign_fn_to_wrong_type")
add_test("test_fun_max_length_name")
add_test("test_fun_too_long_name")
add_test("test_sub_max_length_name")
add_test("test_sub_too_long_name")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub sub_a()
End Sub

Function fun_b%()
End Function

Sub test_call_fn_as_sub()
  Local expected$ = Choice(Mm.Device$ = "MMB4L", "Not a subroutine", "Type specification is invalid: %")
  On Error Skip 1
  fun_b%()
  assert_raw_error(expected$)

  On Error Skip 1
  fun_b()
  assert_raw_error(expected$)

  On Error Skip 1
  fun_b!()
  assert_raw_error(expected$)
End Sub

Sub test_call_sub_as_fn()
  ' Does not pass on non-MMB4L;
  ' reports error "Nothing to return to" when we END SUB.
  If Mm.Device$ <> "MMB4L" Then Exit Sub

  On Error Skip 1
  Local a% = sub_a()
  assert_raw_error("Not a function")
End Sub

Sub test_call_fn_with_wrong_type()
  On Error Skip 1
  Local i% = fun_b$()
  assert_raw_error("Inconsistent type suffix")
End Sub

Sub test_assign_fn_to_wrong_type()
  On Error Skip 2
  Local s$ = fun_b%()
  assert_raw_error("Expected a string")
End Sub

Sub test_fun_max_length_name()
  ' Does not pass on non-MMB4L;
  ' reports error "Variable name too long" when we call function.
  If Mm.Device$ <> "MMB4L" Then Exit Sub
  Local i%
  i% = fun_with_max_length_name_6789012%()
  assert_int_equals(42, i%)
End Sub

Function fun_with_max_length_name_6789012%()
  fun_with_max_length_name_6789012% = 42
End Function

Sub test_fun_too_long_name()
  Local i%
  On Error Skip
  i% = fun_with_too_long_name_4567890123%()
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", "Function/subroutine name too long", "Variable name too long"))
End Sub

Sub test_sub_max_length_name()
  Local i%
  sub_with_max_length_name_6789012(i%)
  assert_int_equals(42, i%)
End Sub

Sub sub_with_max_length_name_6789012(i%)
  i% = 42
End Sub

Sub test_sub_too_long_name()
  On Error Skip
  sub_with_too_long_name_4567890123(i%)
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", "Function/subroutine name too long", "Variable name too long"))
End Sub
