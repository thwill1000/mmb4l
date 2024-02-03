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
Const NOT_FOUND$ = Choice(Mm.Device$ = "MMB4L", "Label not found", "Cannot find label")
Dim global_x%

add_test("test_goto_label")
add_test("test_goto_case_insensitive")
add_test("test_goto_partial_match")
add_test("test_goto_non_existent_label")
add_test("test_goto_max_length_label")
add_test("test_goto_too_long_label")
add_test("test_goto_sub")
add_test("test_goto_function")

add_test("test_gosub_label")
add_test("test_gosub_case_insensitive")
add_test("test_gosub_partial_match")
add_test("test_gosub_non_existent_label")
add_test("test_gosub_max_length_label")
add_test("test_gosub_too_long_label")
add_test("test_gosub_sub")
add_test("test_gosub_function")

add_test("test_restore_label")
add_test("test_restore_case_insensitive")
add_test("test_restore_partial_match")
add_test("test_restore_non_existent_label")
add_test("test_restore_max_length_label")
add_test("test_restore_too_long_label")
add_test("test_restore_sub")
add_test("test_restore_function")

add_test("test_call_label_as_sub")
add_test("test_call_label_as_function")
add_test("test_duplicate_label_and_sub")
add_test("test_duplicate_label_and_fun")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

' Dummy data that would be read without a RESTORE statement.
Data 0, 0, 0

Sub test_goto_label()
  Local x% = 1
  Goto goto_label
  assert_fail("Should never happen")
goto_label:
  assert_int_equals(1, x%)
End Sub

Sub test_goto_case_insensitive()
  Local x% = 1
  Goto GOTO_CASE_INSENSITIVE_LC
  assert_fail("Should never happen")
goto_case_insensitive_lc:
  Goto goto_case_insensitive_uc
  assert_fail("Should never happen")
GOTO_CASE_INSENSITIVE_UC:
  assert_int_equals(1, x%)
End Sub

Sub test_goto_partial_match()
  On Error Skip 1
  Goto goto_partial
  assert_raw_error(NOT_FOUND$)

  On Error Skip 1
  Goto partial_match
  assert_raw_error(NOT_FOUND$)

  Exit Sub
goto_partial_match:
  assert_fail("Should never happen")
End Sub

Sub test_goto_non_existent_label()
  On Error Skip 1
  Goto no_such_label
  assert_raw_error(NOT_FOUND$)
End Sub

Sub test_goto_max_length_label()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  Local x% = 1
  Goto goto_max_length_label_3456789012
  assert_fail("Should never happen")
goto_max_length_label_3456789012:
  assert_int_equals(1, x%)
End Sub

Sub test_goto_too_long_label()
  On Error Skip 1
  Goto thirty_three_characters_567890123
  assert_raw_error("Label too long")
End Sub

Sub test_goto_sub()
  If sys.is_platform%("pm*") Then Exit Sub

  On Error Skip 1
  Goto my_sub
  assert_raw_error(Choice(sys.is_platform%("mmb4l"), "Not a label", NOT_FOUND$))
End Sub

Sub my_sub()
  assert_fail("Should never happen")
End Sub

Sub test_goto_function()
  If sys.is_platform%("pm*") Then Exit Sub

  On Error Skip 1
  Goto my_function
  assert_raw_error(Choice(sys.is_platform%("mmb4l"), "Not a label", NOT_FOUND$))
End Sub

Function my_function%()
  assert_fail("Should never happen")
End Function

Sub test_gosub_label()
  global_x% = 1
  If Mm.Device$ = "MMB4L" Then assert_int_equals(3, Mm.Info(CallDepth))
  Gosub gosub_label
  If Mm.Device$ = "MMB4L" Then assert_int_equals(3, Mm.Info(CallDepth))
  assert_int_equals(2, global_x%)
End Sub

gosub_label:
  ' Strangely (?) the call depth is increased when GOSUB is called.
  If Mm.Device$ = "MMB4L" Then assert_int_equals(4, Mm.Info(CallDepth))
  Inc global_x%
  Return

Sub test_gosub_case_insensitive()
  global_x% = 1
  Gosub GOSUB_CASE_INSENSITIVE_LC
  assert_int_equals(3, global_x%)
  Gosub gosub_case_insensitive_uc
  assert_int_equals(7, global_x%)
End Sub

gosub_case_insensitive_lc:
  Inc global_x%, 2
  Return

GOSUB_CASE_INSENSITIVE_UC:
  Inc global_x%, 4
  Return

Sub test_gosub_partial_match()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  On Error Skip 1
  Gosub gosub_partial
  assert_raw_error(NOT_FOUND$)

  On Error Skip 1
  Gosub partial_match
  assert_raw_error(NOT_FOUND$)
End Sub

gosub_partial_match:
  assert_fail("Should never happen")

Sub test_gosub_non_existent_label()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  On Error Skip 1
  Gosub no_such_label
  assert_raw_error(NOT_FOUND$)
End Sub

Sub test_gosub_max_length_label()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  global_x% = 1
  Gosub gosub_max_length_label_456789012
  assert_int_equals(2, global_x%)
End Sub

gosub_max_length_label_456789012:
  Inc global_x%
  Return

Sub test_gosub_too_long_label()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  On Error Skip 1
  Gosub thirty_three_characters_567890123
  assert_raw_error("Label too long")
End Sub

Sub test_gosub_sub()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  On Error Skip 1
  Gosub my_sub
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", "Not a label", NOT_FOUND$))
End Sub

Sub test_gosub_function()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  On Error Skip 1
  Gosub my_function
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", "Not a label", NOT_FOUND$))
End Sub

Sub test_restore_label()
  Local x%, y%, z%
  Restore restore_label
  Read x%, y%, z%
  assert_int_equals(4, x%)
  assert_int_equals(5, y%)
  assert_int_equals(6, z%)
End Sub

restore_label:
  Data 4, 5, 6, 7

Sub test_restore_case_insensitive()
  Local x%, y%, z%

  Restore RESTORE_CASE_INSENSITIVE_LC
  Read x%, y%, z%
  assert_int_equals(7, x%)
  assert_int_equals(5, y%)
  assert_int_equals(3, z%)

  Restore restore_case_insensitive_uc
  Read x%, y%, z%
  assert_int_equals(12, x%)
  assert_int_equals(14, y%)
  assert_int_equals(16, z%)
End Sub

restore_case_insensitive_lc:
  Data 7, 5, 3

RESTORE_CASE_INSENSITIVE_UC:
  Data 12, 14, 16

Sub test_restore_partial_match()
  On Error Skip 1
  Restore restore_partial
  assert_raw_error(NOT_FOUND$)

  On Error Skip 1
  Restore partial_match
  assert_raw_error(NOT_FOUND$)
End Sub

restore_partial_match:
  Data 21, 23, 25

Sub test_restore_non_existent_label()
  On Error Skip 1
  Restore no_such_label
  assert_raw_error(NOT_FOUND$)
End Sub

Sub test_restore_max_length_label()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  Local x%, y%, z%
  Restore restore_max_length_label_6789012
  Read x%, y%, z%
  assert_int_equals(8, x%)
  assert_int_equals(9, y%)
  assert_int_equals(10, z%)
End Sub

restore_max_length_label_6789012:
  Data 8, 9, 10

Sub test_restore_too_long_label()
  On Error Skip 1
  Restore thirty_three_characters_567890123
  ' Note RESTORE can work with variables as well as labels,
  ' hence this error message.
  assert_raw_error("Variable name too long")
End Sub

Sub test_restore_sub()
  If sys.is_platform%("pm*") Then Exit Sub

  On Error Skip 1
  Restore my_sub
  assert_raw_error(Choice(sys.is_platform%("mmb4l"), "Not a label", NOT_FOUND$))
End Sub

Sub test_restore_function()
  If sys.is_platform%("pm*") Then Exit Sub

  On Error Skip 1
  Restore my_function
  assert_raw_error(Choice(sys.is_platform%("mmb4l"), "Not a label", NOT_FOUND$))
End Sub

Sub test_call_label_as_sub()
  On Error Skip 1
  my_label()
  assert_raw_error("Unknown command")
End Sub

my_label:
  assert_fail("Should never happen")

Sub test_call_label_as_function()
  On Error Skip 1
  Local x% = my_label(2)
  assert_raw_error("MY_LABEL is not declared")
End Sub

Sub test_duplicate_label_and_sub()
  If sys.is_platform%("pm*") Then Exit Sub

  global_x% = 0
  duplicate_label_and_sub()
  assert_int_equals(1, global_x%)
  Gosub duplicate_label_and_sub
  assert_int_equals(6, global_x%)
End Sub

Sub duplicate_label_and_sub()
  Inc global_x%, 1
End Sub

duplicate_label_and_sub:
  Inc global_x%, 5
  Return

Sub test_duplicate_label_and_fun()
  If sys.is_platform%("pm*") Then Exit Sub

  global_x% = 0
  Inc global_x%, duplicate_label_and_fun%()
  assert_int_equals(3, global_x%)
  Gosub duplicate_label_and_fun
  assert_int_equals(13, global_x%)
End Sub

Function duplicate_label_and_fun%()
  duplicate_label_and_fun% = 3
End Function

duplicate_label_and_fun:
  Inc global_x%, 10
  Return
