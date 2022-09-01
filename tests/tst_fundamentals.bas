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
add_test("test_inv")
add_test("test_unary_minus")
add_test("test_unary_plus")
add_test("test_error_correct_after_goto")
add_test("test_error_correct_after_gosub")
add_test("test_dim_with_same_name_as_sub_reports_error", "test_dim_with_same_name")
add_test("test_local_with_same_name_as_fun_reports_error", "test_local_with_same_name")


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
  assert_raw_error("Syntax")

  On Error Skip 1
  Erase _32_chars_long_67890123456789012%
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", "Cannot find global variable _32_CHARS_LONG_67890123456789012", "Cannot find _32_CHARS_LONG_67890123456789012"))

  On Error Skip 1
  Erase _33_chars_long_678901234567890123%
  assert_raw_error("Name too long")
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
  Local base_line% = 167
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
  Local base_line% = 183
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
  assert_raw_error("A sub/fun has the same name: SUB_A")
End Sub

Sub sub_a()
End Sub

Sub test_local_with_same_name()
  On Error Skip 1
  Local fun_b%
  assert_raw_error("A sub/fun has the same name: FUN_B")
End Sub

Function fun_b%()
End Function
