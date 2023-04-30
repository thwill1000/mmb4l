' Copyright (c) 2021-2023 Thomas Hugo Williams
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

add_test("test_call_integer_fun")
add_test("test_call_float_fun")
add_test("test_call_string_fun")
add_test("test_call_sub")
add_test("test_call_fun_as_sub")
add_test("test_call_sub_as_fun")
add_test("test_call_label_as_fun")
add_test("test_call_label_as_sub")
add_test("test_call_fun_max_name")
add_test("test_call_fun_too_long_name")
add_test("test_call_sub_max_name")
add_test("test_call_sub_too_long_name")
add_test("test_call_missing_fun")
add_test("test_call_missing_sub")
add_test("test_arg_list_bug")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Function int_fn%()
  int_fn% = 42
End Function

Function float_fn!()
  float_fn! = 3.12
End Function

Function string_fn$()
  string_fn$ = "bar"
End Function

Sub foo(i%, f!, s$)
  i% = 42
  f! = 3.12
  s$ = "bar"
End Sub

Function fun_with_max_length_name_6789012%()
  fun_with_max_length_name_6789012% = 42
End Function

Sub sub_with_max_length_name_6789012(i%)
  i% = 42
End Sub

Sub test_call_integer_fun()
  assert_int_equals(42, Call("int_fn%"))
End Sub

Sub test_call_float_fun()
  assert_float_equals(3.12, Call("float_fn!"))
End Sub

Sub test_call_string_fun()
  assert_string_equals("bar", Call("string_fn$"))
End Sub

wombat_label:
  Print "Should never see this"
  Return

Sub test_call_sub()
  Local i%, f!, s$
  Call "foo", i%, f!, s$
  assert_int_equals(42, i%)
  assert_float_equals(3.12, f!)
  assert_string_equals("bar", s$)
End Sub

Sub test_call_fun_as_sub()
  Local i%
  On Error Skip
  Call "int_fn", i%
  If sys.is_device%("mmb4l") Then
    assert_raw_error("Not a subroutine")
  ElseIf sys.is_device%("pmvga") Then
    assert_raw_error("Unknown user subroutine")
  Else
    assert_raw_error("Type specification is invalid")
  EndIf
End Sub

Sub test_call_sub_as_fun()
  If Not sys.is_device%("mmb4l") Then Exit Sub
  Local i%
  On Error Skip
  i% = Call("foo", i%)
  assert_raw_error("Not a function")
End Sub

Sub test_call_label_as_fun()
  Local i%
  On Error Skip
  i% = Call("wombat_label", i%)
  If sys.is_device%("mmb4l") Then
    assert_raw_error("Function not found")
  Else
    assert_raw_error("Unknown user function")
  EndIf
End Sub

Sub test_call_label_as_sub()
  Local i%
  On Error Skip
  Call "wombat_label", i%
  If sys.is_device%("mmb4l") Then
    assert_raw_error("Subroutine not found")
  Else
    assert_raw_error("Unknown user subroutine")
  EndIf
End Sub

Sub test_call_fun_max_name()
  If Not sys.is_device%("mmb4l") Then Exit Sub
  Local i%
  i% = Call("fun_with_max_length_name_6789012%")
  assert_int_equals(42, i%)
End Sub

Sub test_call_fun_too_long_name()
  Local i%
  On Error Skip
  i% = Call("fun_with_too_long_name_4567890123%", i%)
  If sys.is_device%("mmb4l") Then
    assert_raw_error("Function name too long")
  ElseIf sys.is_device%("pmvga") Then
    assert_raw_error("Unknown user function")
  Else
    assert_raw_error("Variable name too long")
  EndIf
End Sub

Sub test_call_sub_max_name()
  Local i%
  Call "sub_with_max_length_name_6789012", i%
  assert_int_equals(42, i%)
End Sub

Sub test_call_sub_too_long_name()
  Local i%
  On Error Skip
  Call "sub_with_too_long_name_4567890123", i%
  If sys.is_device%("mmb4l") Then
    assert_raw_error("Subroutine name too long")
  ElseIf sys.is_device%("pmvga") Then
    assert_raw_error("Unknown user subroutine")
  Else
    assert_raw_error("Variable name too long")
  EndIf
End Sub

Sub test_call_missing_fun()
  Local i%
  On Error Skip
  i% = Call("missing_fun", i%)
  If sys.is_device%("mmb4l") Then
    assert_raw_error("Function not found")
  Else
    assert_raw_error("Unknown user function")
  EndIf
End Sub

Sub test_call_missing_sub()
  Local i%
  On Error Skip
  Call "missing_sub", i%
  If sys.is_device%("mmb4l") Then
    assert_raw_error("Subroutine not found")
  Else
    assert_raw_error("Unknown user subroutine")
  EndIf
End Sub

' Prior to 5.07.01 this would report an "Argument List" error from the Print #1 statement.
Sub test_arg_list_bug()
  MkDir TMPDIR$

  Const f$ = TMPDIR$ + "/test_arg_list_bug"
  Open f$ For Output As #1
  Print #1, Call("int_fn%"), Call("float_fn!"), Call("string_fn$")
  Close #1

  Open f$ For Input As #1
  Local s$
  Line Input #1, s$

  ' The difference seems to be something to do with how tabs are handled.
  If sys.is_device%("mmb4l", "mmb4w", "pm*") Then
    assert_string_equals(" 42  3.12   bar", s$)
  Else
    assert_string_equals(" 42  3.12 bar", s$)
  EndIf

  assert_int_equals(1, Eof(#1))
  Close #1
End Sub
