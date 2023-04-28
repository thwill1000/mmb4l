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

add_test("test_base2")
add_test("test_base8")
add_test("test_base10")
add_test("test_base16")
add_test("test_floats")
add_test("test_strings")
add_test("test_mixed_types")
add_test("test_restore_with_line_number",       "test_restore_lineno")
add_test("test_restore_with_string_literal",    "test_restore_strlit")
add_test("test_restore_with_string_expression", "test_restore_strexp")
add_test("test_restore_with_int_expression",    "test_restore_intexp")
add_test("test_restore_with_float_expression",  "test_restore_floatexp")
add_test("test_data_save_and_restore")
add_test("test_data_save_and_restore_errors", "test_data_save_and_restore_err")
add_test("test_peek_poke_dataptr")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_base2()
  Local actual%(array.new%(5))
  Local i%
  Restore base2_data
  For i% = 0 To 4
    Read actual%(BASE% + i%)
  Next

  Local expected%(array.new%(5)) = (52, 2, 0, 255, 0)
  assert_int_array_equals(expected%(), actual%())
End Sub

base2_data:
Data &B0110100, &b10, &b0, &B11111111, &B00000000

Sub test_base8()
  Local actual%(array.new%(5))
  Local i%
  Restore base8_data
  For i% = 0 To 4
    Read actual%(BASE% + i%)
  Next

  Local expected%(array.new%(5)) = (0, 19471, 5349, 0, 16777215)
  assert_int_array_equals(expected%(), actual%())
End Sub

base8_data:
Data &O0, &O46017, &o12345, &o00000000, &o77777777

Sub test_base10()
  Local actual%(array.new%(5))
  Local i%
  Restore base10_data
  For i% = 0 To 4
    Read actual%(BASE% + i%)
  Next

  Local expected%(array.new%(5)) = (0, 1, -1, 9223372036854775807, -9223372036854775808)
  assert_int_array_equals(expected%(), actual%())
End Sub

base10_data:
Data 0, 1, -1, 9223372036854775807, -9223372036854775808

End Sub

Sub test_base16()
  Local actual%(array.new%(5))
  Local i%
  Restore base16_data
  For i% = 0 To 4
    Read actual%(BASE% + i%)
  Next

  Local expected%(array.new%(5)) = (0, 4294967295, 4294901760, 1229782938247303441, 81985529216486895)
  assert_int_array_equals(expected%(), actual%())
End Sub

base16_data:
Data &h0, &hFFFFFFFF, &HFFFF0000, &h1111111111111111, &h0123456789ABCDEF

Sub test_floats()
  Local actual!(array.new%(5))
  Local i%
  Restore floats_data
  For i% = 0 To 4
    Read actual!(BASE% + i%)
  Next

  Local expected!(array.new%(5)) = (0, 1.1, -1.1, 1.7e308, 1.0e-38)
  Local delta!(array.new%(5))
  assert_float_array_equals(expected!(), actual!(), delta!())
End Sub

floats_data:
Data 0.0, 1.1, -1.1, 1.7e308, 1.0e-38)

Sub test_strings()
  Local actual$(array.new%(5))
  Local i%
  Restore strings_data
  For i% = 0 To 4
    Read actual$(BASE% + i%)
  Next

  Local expected$(array.new%(5)) = ("", "foo", "bar", "wombat", "snafu")
  assert_string_array_equals(expected$(), actual$())
End Sub

strings_data:
Data "", "foo", "bar", "wombat", "snafu"

Sub test_mixed_types()
  Local i1%, i2%, i3%, i4%, f!, s$

  Restore mixed_types_data
  Read i1%, i2%, i3%, i4%, f!, s$

  assert_int_equals(12, i1%)
  assert_int_equals(668, i2%)
  assert_int_equals(-376, i3%)
  assert_int_equals(43981, i4%)
  assert_float_equals(3.142, f!)
  assert_string_equals("foobar", s$)
End Sub

mixed_types_data:
Data &b1100, &o1234, -376, &hABCD, 3.142, "foobar"

Sub test_restore_lineno()
  Local i%, f!, s$

  Restore 999
  Read i%, f!, s$

  assert_int_equals(42, i%)
  assert_float_equals(3.142, f!, 1e-6)
  assert_string_equals("Hello World", s$)

  If Mm.Device$ = "MMB4L" Then

    Restore &h3E7
    Read i%, f!, s$

    assert_int_equals(42, i%)
    assert_float_equals(3.142, f!, 1e-6)
    assert_string_equals("Hello World", s$)

    Restore &o1747
    Read i%, f!, s$

    assert_int_equals(42, i%)
    assert_float_equals(3.142, f!, 1e-6)
    assert_string_equals("Hello World", s$)

    Restore &b1111100111
    Read i%, f!, s$

  EndIf

  assert_int_equals(42, i%)
  assert_float_equals(3.142, f!, 1e-6)
  assert_string_equals("Hello World", s$)
End Sub

999 Data 42, 3.142, "Hello World"

Sub test_restore_strlit()
  If InStr(Mm.Device$, "Colour Maximite 2") Then Exit Sub

  Local i1%, i2%, i3%, i4%, f!, s$

  Restore "mixed_types_data"
  Read i1%, i2%, i3%, i4%, f!, s$

  assert_int_equals(12, i1%)
  assert_int_equals(668, i2%)
  assert_int_equals(-376, i3%)
  assert_int_equals(43981, i4%)
  assert_float_equals(3.142, f!)
  assert_string_equals("foobar", s$)
End Sub

Sub test_restore_strexp()
  If InStr(Mm.Device$, "Colour Maximite 2") Then Exit Sub

  Local i1%, i2%, i3%, i4%, f!, s$

  s$ = "types"
  Restore "mixed_" + s$ + "_data"
  Read i1%, i2%, i3%, i4%, f!, s$

  assert_int_equals(12, i1%)
  assert_int_equals(668, i2%)
  assert_int_equals(-376, i3%)
  assert_int_equals(43981, i4%)
  assert_float_equals(3.142, f!)
  assert_string_equals("foobar", s$)

  s$ = "mixed_types"
  Restore s$ + "_data"
  Read i1%, i2%, i3%, i4%, f!, s$

  assert_int_equals(12, i1%)
  assert_int_equals(668, i2%)
  assert_int_equals(-376, i3%)
  assert_int_equals(43981, i4%)
  assert_float_equals(3.142, f!)
  assert_string_equals("foobar", s$)
End Sub

Sub test_restore_intexp()
  Local i%, f!, s$

  i% = 900
  Restore i% + 99
  Read i%, f!, s$

  assert_int_equals(42, i%)
  assert_float_equals(3.142, f!, 1e-6)
  assert_string_equals("Hello World", s$)

  i% = 900
  Restore 99 + i%
  Read i%, f!, s$

  assert_int_equals(42, i%)
  assert_float_equals(3.142, f!, 1e-6)
  assert_string_equals("Hello World", s$)

  If Mm.Device$ = "MMB4L" Then

    i% = 900
    Restore &h63 + i%
    Read i%, f!, s$

    assert_int_equals(42, i%)
    assert_float_equals(3.142, f!, 1e-6)
    assert_string_equals("Hello World", s$)

    i% = 900
    Restore &o143 + i%
    Read i%, f!, s$

    assert_int_equals(42, i%)
    assert_float_equals(3.142, f!, 1e-6)
    assert_string_equals("Hello World", s$)

    i% = 900
    Restore &b1100011 + i%
    Read i%, f!, s$

  EndIf

  assert_int_equals(42, i%)
  assert_float_equals(3.142, f!, 1e-6)
  assert_string_equals("Hello World", s$)
End Sub

Sub test_restore_floatexp()
  Local i%, f!, s$

  f! = 900.0
  Restore f! + 99.0
  Read i%, f!, s$

  assert_int_equals(42, i%)
  assert_float_equals(3.142, f!, 1e-6)
  assert_string_equals("Hello World", s$)

  f! = 900.0
  Restore 99.0 + f!
  Read i%, f!, s$

  assert_int_equals(42, i%)
  assert_float_equals(3.142, f!, 1e-6)
  assert_string_equals("Hello World", s$)
End Sub

Sub test_data_save_and_restore()
  If InStr(Mm.Device$, "Colour Maximite 2") Then Exit Sub

  Local actual$(BASE% + 15)
  Local expected$(BASE% + 15) = ("mm", "nn", "oo", "pp", "ii", "jj", "kk", "ll", "ee", "ff", "gg", "hh", "aa", "bb", "cc", "dd")
  Local i%, j%, idx% = BASE%

  For i% = 1 To 4
    Restore "save_and_restore_data_" + Str$(i%)
    If i% <> 4 Then Read Save
  Next

  For i% = 1 To 4
    For j% = 1 To 4
      Read actual$(idx%)
      Inc idx%
    Next
    If i% <> 4 Then Read Restore
  Next

  assert_string_array_equals(expected$(), actual$())
End Sub

Sub test_data_save_and_restore_err()
  If InStr(Mm.Device$, "Colour Maximite 2") Then Exit Sub

  ' Pop when the stack is empty.
  On Error Skip
  Read Restore
  assert_raw_error("Nothing to restore")

  Local i%
  If sys.is_device%("pm*") Then
    For i% = 1 To 15 : Read Save : Next
  Else
    For i% = 1 To 49 : Read Save : Next
  EndIf

  On Error Skip
  Read Save
  assert_raw_error("Too many saves")
End Sub

save_and_restore_data_1:
Data "aa", "bb", "cc", "dd"

save_and_restore_data_2:
Data "ee", "ff", "gg", "hh"

save_and_restore_data_3:
Data "ii", "jj", "kk", "ll"

save_and_restore_data_4:
Data "mm", "nn", "oo", "pp"

Sub test_peek_poke_dataptr()
  If Mm.Device$ <> "MMB4L" Then Exit Sub

  Local s1$, s2$

  Restore save_and_restore_data_1
  Read s1$, s2$
  assert_string_equals("aa", s1$)
  assert_string_equals("bb", s2$)

  Local ptr1% = Peek(DataPtr)

  Restore save_and_restore_data_2
  Read s1$, s2$
  assert_string_equals("ee", s1$)
  assert_string_equals("ff", s2$)

  Local ptr2% = Peek(DataPtr)

  Poke DataPtr ptr1%
  Read s1$, s2$
  assert_string_equals("cc", s1$)
  assert_string_equals("dd", s2$)

  Poke DataPtr ptr2%
  Read s1$, s2$
  assert_string_equals("gg", s1$)
  assert_string_equals("hh", s2$)
End Sub
