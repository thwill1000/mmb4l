' Copyright (c) 2021-2022 Thomas Hugo Williams
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

add_test("test_erase")
add_test("test_erase_given_arrays")
add_test("test_erase_given_strings")
add_test("test_inv")
add_test("test_unary_minus")
add_test("test_unary_plus")
add_test("test_error_correct_after_goto")
add_test("test_error_correct_after_gosub")
add_test("test_dim_with_same_name_as_sub_reports_error", "test_dim_with_same_name")
add_test("test_local_with_same_name_as_fun_reports_error", "test_local_with_same_name")
add_test("test_call_fn_as_sub")
add_test("test_call_sub_as_fn")
add_test("test_call_fn_with_wrong_type")
add_test("test_assign_fn_to_wrong_type")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_erase()
  Dim foo%, bar!, wombat$

  foo% = 42
  bar! = 3.142
  wombat$ = "wombat"

  Erase foo%
  On Error Skip 1
  foo% = 42
  assert_raw_error("FOO is not declared")
  bar! = 3.142
  wombat$ = "wombat"

  Erase bar, wombat$
  On Error Skip 1
  bar! = 3.142
  assert_raw_error("BAR is not declared")
  On Error Skip 1
  wombat$ = "wombat"
  assert_raw_error("WOMBAT is not declared")

  On Error Skip 1
  Erase snafu!
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", "Cannot find global variable SNAFU", "Cannot find SNAFU"))

  On Error Skip 1
  Erase *invalid
  Select Case Mm.Device$
    Case "Colour Maximite 2", "Colour Maximite 2 G2"
      assert_raw_error("Unknown command")
    Case Else
      assert_raw_error("Invalid name")
  End Select

  On Error Skip 1
  Erase _32_chars_long_67890123456789012%
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", "Cannot find global variable _32_CHARS_LONG_67890123456789012", "Cannot find _32_CHARS_LONG_67890123456789012"))

  On Error Skip 1
  Erase _33_chars_long_678901234567890123%
  Select Case Mm.Device$
    Case "Colour Maximite 2", "Colour Maximite 2 G2"
      assert_raw_error("Cannot find _33_CHARS_LONG_678901234567890123")
    Case Else
      assert_raw_error("Name too long")
  End Select
End Sub

' There was a bug in MMB4W (and the PicoMite) where the heap memory
' used by arrays was not being released correctly and would eventually
' be exhausted.
Sub test_erase_given_arrays()
  If Mm.Device$ = "MMBasic for Windows" Then
    Local filler1%(15 * 1024 * 1024)
  Else
    ' TODO: remove MMB4L 32K array size limitation.
    Local filler1%(32 * 1024 - 1)
    Local filler2%(32 * 1024 - 1)
    Local filler3%(32 * 1024 - 1)
    Local filler4%(24 * 1024 - 1)
  EndIf
  Local i%
  For i% = 0 To 255
    Dim foo_array%(Mm.Info(Option Base) + 4095) ' 32K
    Erase foo_array%
  Next

  On Error Skip 1
  foo_array%(1) = 42
  assert_raw_error("FOO_ARRAY is not declared")
End Sub

' There was a bug in MMB4W (and the PicoMite) where the heap memory
' used by strings was not being released correctly and would eventually
' be exhausted.
Sub test_erase_given_strings()
  If Mm.Device$ = "MMBasic for Windows" Then
    Local filler1%(15 * 1024 * 1024)
  Else
    ' TODO: remove MMB4L 32K array size limitation.
    Local filler1%(32 * 1024 - 1)
    Local filler2%(32 * 1024 - 1)
    Local filler3%(32 * 1024 - 1)
    Local filler4%(24 * 1024 - 1)
  EndIf
  Local i%
  For i% = 0 To 32767
    Dim foo_string$
    Erase foo_string$
  Next

  On Error Skip 1
  foo_string$ = "foo"
  assert_raw_error("FOO_STRING is not declared")
End Sub

Sub test_inv()
  assert_hex_equals(&hFFFFFFFFFFFFFFFF, Inv(&h0))
  assert_hex_equals(&hFFFFFFFFFFFFFFFE, Inv(&h1))
  assert_hex_equals(&h0,                Inv(&hFFFFFFFFFFFFFFFF))
  assert_hex_equals(&h8000000000000000, Inv(&h7FFFFFFFFFFFFFFF))
  assert_hex_equals(&h7FFFFFFFFFFFFFFF, Inv(&h8000000000000000))
End Sub

Sub test_unary_minus()
  Local x% = 3
  assert_int_equals(-3, -x%)
  assert_int_equals(-3, -(3))
  assert_int_equals(-3, -(x%))
  assert_int_equals(-3, (-x%))
  assert_int_equals(-6, -(3 + 3))
  assert_int_equals(-6, -(x% + x%))

  x% = -3
  assert_int_equals(3, -x%)
  assert_int_equals(3, -(-3))
  assert_int_equals(3, -(x%))
  assert_int_equals(3, (-x%))
  assert_int_equals(6, -(-3 - 3))
  assert_int_equals(0, -(-3 - -3))
  assert_int_equals(6, -(x% + x%))

  Local y! = 3.2
  assert_float_equals(-3.2, -y!)
  assert_float_equals(-3.2, -(3.2))
  assert_float_equals(-3.2, -(y!))
  assert_float_equals(-3.2, (-y!))
  assert_float_equals(-6.4, -(3.2 + 3.2))
  assert_float_equals(-6.4, -(y! + y!))

  y! = -3.2
  assert_float_equals(3.2, -y!)
  assert_float_equals(3.2, -(-3.2))
  assert_float_equals(3.2, -(y!))
  assert_float_equals(3.2, (-y!))
  assert_float_equals(6.4, -(-3.2 - 3.2))
  assert_float_equals(6.4, -(y! + y!))
End Sub

Sub test_unary_plus()
  Local x% = 3
  assert_int_equals(3, +x%)
  assert_int_equals(3, +3)
  assert_int_equals(3, +(3))
  assert_int_equals(3, (+x%))
  assert_int_equals(3, +(x%))
  assert_int_equals(6, (+3 + 3))
  assert_int_equals(6, +(x% + x%))

  x% = -3
  assert_int_equals(-3, +x%)
  assert_int_equals(-3, -(3))
  assert_int_equals(-3, (+x%))
  assert_int_equals(-3, +(x%))
  assert_int_equals(-6, (-3 + -3))
  assert_int_equals(-6, +(x% + x%))

  Local y! = 3.2
  assert_float_equals(3.2, +y!)
  assert_float_equals(3.2, +3.2)
  assert_float_equals(3.2, +(3.2))
  assert_float_equals(3.2, (+y!))
  assert_float_equals(3.2, +(y!))
  assert_float_equals(6.4, (+3.2 + 3.2))
  assert_float_equals(6.4, +(y! + y!))

  y! = -3.2
  assert_float_equals(-3.2, +y!)
  assert_float_equals(-3.2, +(-3.2))
  assert_float_equals(-3.2, (+y!))
  assert_float_equals(-3.2, +(y!))
  assert_float_equals(-6.4, (-3.2 + -3.2))
  assert_float_equals(-6.4, +(y! + y!))
End Sub

Sub test_error_correct_after_goto()
  Local base_line% = 230
  Goto 30
test_goto_label_1:
  assert_raw_error("Error in line " + Str$(base_line% + 4) + ": foo1")
  Goto 40
test_goto_label_2:
  assert_raw_error("Error in line " + Str$(base_line% + 6) + ": foo2")
  On Error Skip
  Error "foo3"
  assert_raw_error("Error in line " + Str$(base_line%) + ": foo3")
End Sub

30 On Error Skip : Error "foo1" : Goto test_goto_label_1
40 On Error Skip
Error "foo2"
Goto test_goto_label_2

Sub test_error_correct_after_gosub()
  Local base_line% = 246
  GoSub 60
  assert_raw_error("Error in line " + Str$(base_line% + 4) + ": bar1")
  GoSub 70
  assert_raw_error("Error in line " + Str$(base_line% + 6) + ": bar2")
  On Error Skip
  Error "bar3"
  assert_raw_error("Error in line " + Str$(base_line%) + ": bar3")
End Sub

60 On Error Skip : Error "bar1" : Return
70 On Error Skip
Error "bar2"
Return

Sub test_dim_with_same_name()
  On Error Skip 1
  Dim sub_a%
  If Mm.Device$ = "MMB4L" Then
    assert_raw_error("A function/subroutine has the same name: SUB_A")
  Else
    assert_raw_error("A sub/fun has the same name: SUB_A")
  EndIf
End Sub

Sub sub_a()
End Sub

Sub test_local_with_same_name()
  On Error Skip 1
  Local fun_b%
  If Mm.Device$ = "MMB4L" Then
    assert_raw_error("A function/subroutine has the same name: FUN_B")
  Else
    assert_raw_error("A sub/fun has the same name: FUN_B")
  EndIf
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
  ' Can't test this on non-MMB4L;
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
