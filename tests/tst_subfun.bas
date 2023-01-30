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
add_test("test_fun_alt_type_syntax")
add_test("test_sub_alt_type_syntax")
add_test("test_fun_label_and_sub")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub sub_a(i%)
  i% = 42
End Sub

Function fun_b%()
  fun_b% = 42
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
  On Error Skip 3 ' Needs SKIP 3 because of lines to skip in function itself.
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
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", "Function name too long", "Variable name too long"))
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
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", "Subroutine name too long", "Variable name too long"))
End Sub

Function ifun(i As Integer) As Integer
  ifun = i
End Function

Sub test_fun_alt_type_syntax()
  assert_int_equals(42, ifun(42))
End Sub

Sub isub(a As Integer, b As Integer)
  b = a
End Sub

Sub test_sub_alt_type_syntax()
  Local c As Integer = 42
  Local Integer d
  isub(c, d)
  assert_int_equals(42, d)
End Sub

' Smoke test for mixing functions, labels and subroutines with same names.
Sub test_fun_label_and_sub()
  Local i%, s$

  Restore aaa
  Read s$
  assert_string_equals("aaa", s$)

  Restore bbb
  Read s$
  assert_string_equals("bbb", s$)

  aaa(i%, 3.14)
  assert_int_equals(3, i%)

  i% = bbb(42, 3.142)
  assert_int_equals(126, i%)
End Sub

zzz: Data "zzz"

aaa: Sub aaa(i As Integer, f As Float)
  i = Int(f)
End Sub

Data "aaa", "zzz"

bbb: Function bbb(i As Integer, f As Float) As Integer
  bbb = i * Int(f)
End Function

Data "bbb"
