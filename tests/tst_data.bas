' Copyright (c) 2021-2022 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 5.07

Option Explicit On
Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1")  > 0

#Include "../basic-src/splib/system.inc"
#Include "../basic-src/splib/array.inc"
#Include "../basic-src/splib/list.inc"
#Include "../basic-src/splib/string.inc"
#Include "../basic-src/splib/file.inc"
#Include "../basic-src/splib/vt100.inc"
#Include "../basic-src/sptest/unittest.inc"

Const BASE% = Mm.Info(Option Base)

add_test("test_base2")
add_test("test_base8")
add_test("test_base10")
add_test("test_base16")
add_test("test_floats")
add_test("test_strings")
add_test("test_mixed_types")
add_test("test_restore_with_label")
add_test("test_data_save_and_restore")
add_test("test_data_save_and_restore_errors", "test_data_save_and_restore_err")

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
  Local i%, f!, s$
  Restore mixed_types_data

  Read i%
  assert_int_equals(12, i%)

  Read i%
  assert_int_equals(668, i%)

  Read i%
  assert_int_equals(-376, i%)

  Read i%
  assert_int_equals(43981, i%)

  Read f!
  assert_float_equals(3.142, f!)

  Read s$
  assert_string_equals("foobar", s$)
End Sub

mixed_types_data:
Data &b1100, &o1234, -376, &hABCD, 3.142, "foobar"

Sub test_restore_with_label()
  Local i%, f!, s$

  Restore strings_data
  Read s$ : Read s$ : Read s$ ' s$ = 3rd item of data
  assert_string_equals("bar", s$)

  Restore floats_data
  Read f! : Read f! : Read f! ' f! = 3rd item of data
  assert_float_equals(-1.1, f!)

  Restore base16_data
  Read i% : Read i% : Read i% ' i% = 3rd item of data
  assert_int_equals(4294901760, i%)

  Restore base10_data
  Read i% : Read i% : Read i% ' i% = 3rd item of data
  assert_int_equals(-1, i%)

  Restore base8_data
  Read i% : Read i% : Read i% ' i% = 3rd item of data
  assert_int_equals(5349, i%)

  Restore base2_data
  Read i% : Read i% : Read i% : Read i% ' i% = 4th item of data
  assert_int_equals(255, i%)
End Sub

Sub test_data_save_and_restore()
  If Mm.Device$ <> "MMBasic for Windows" Then Exit Sub

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
  If Mm.Device$ <> "MMBasic for Windows" Then Exit Sub

  ' Pop when the stack is empty.
  On Error Skip
  Read Restore
  assert_raw_error("Nothing to restore")

  Local i%
  For i% = 1 To 49 : Read Save : Next

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
