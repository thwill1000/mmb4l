' Copyright (c) 2020-2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

Option Explicit On
Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1") > 0

#Include "../src/splib/system.inc"
#Include "../src/splib/array.inc"
#Include "../src/splib/list.inc"
#Include "../src/splib/string.inc"
#Include "../src/splib/file.inc"
#Include "../src/splib/vt100.inc"
#Include "../src/sptest/unittest.inc"

Const base% = Mm.Info(Option Base)

add_test("test_inc_scalar")
add_test("test_inc_1d_int_array")
add_test("test_inc_1d_float_array")
add_test("test_inc_1d_string_array")
add_test("test_inc_2d_int_array")
add_test("test_inc_2d_float_array")
add_test("test_inc_2d_string_array")
add_test("test_inc_given_string_too_long")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_inc_scalar()
  Local i%, f!, s$

  Inc i%
  assert_int_equals(1, i%)
  Inc i%, 2
  assert_int_equals(3, i%)
  Inc i%, -4
  assert_int_equals(-1, i%)

  Inc f!
  assert_float_equals(1.0, f!, 0.001)
  Inc f!, 3.21
  assert_float_equals(4.21, f!, 0.001)
  Inc f!, -5.19
  assert_float_equals(-0.98, f!, 0.001)

  Cat s$, "foo"
  assert_string_equals("foo", s$)
  Cat s$, "bar"
  assert_string_equals("foobar", s$)
End Sub

Sub test_inc_1d_int_array()
  Local a%(array.new%(5)) = (1, 2, 3, 4, 5)

  Local i%
  For i% = base% To base% + 4 : Inc a%(i%) : Next

  Local expected%(array.new%(5)) = (2, 3, 4, 5, 6)
  assert_int_array_equals(expected%(), a%())
End Sub

Sub test_inc_1d_float_array()
  Local a!(array.new%(5)) = (-73.2, -3.14, 0.0, 1000.992, 3.41)

  Local i%
  For i% = base% To base% + 4 : Inc a!(i%), 1.3 : Next

  Local expected!(array.new%(5)) = (-71.9, -1.84, 1.3, 1002.292, 4.71)
  Local deltas!(array.new%(5)) = (0.001, 0.001, 0.001, 0.001, 0.001)
  assert_float_array_equals(expected!(), a!(), deltas!())
End Sub

Sub test_inc_1d_string_array()
  Local a$(array.new%(5)) = ("one", "two", "three", "four", "five")

  Local i%
  For i% = base% To base% + 4 : Cat a$(i%), Str$(i% - base% + 1) : Next

  Local expected$(array.new%(5)) = ("one1", "two2", "three3", "four4", "five5")
  assert_string_array_equals(expected$(), a$())
End Sub

Sub test_inc_2d_int_array()
  Local a%(array.new%(3), array.new%(3)) = (-4, -3, -2, -1, 0, 1, 2, 3, 4)

  assert_int_equals(-4, a%(base% + 0, base% + 0))
  assert_int_equals(-3, a%(base% + 1, base% + 0))
  assert_int_equals(-2, a%(base% + 2, base% + 0))
  assert_int_equals(-1, a%(base% + 0, base% + 1))
  assert_int_equals( 0, a%(base% + 1, base% + 1))
  assert_int_equals( 1, a%(base% + 2, base% + 1))
  assert_int_equals( 2, a%(base% + 0, base% + 2))
  assert_int_equals( 3, a%(base% + 1, base% + 2))
  assert_int_equals( 4, a%(base% + 2, base% + 2))

  Local i%, j%
  For i% = base% To base% + 2
    For j% = base% To base% + 2
      Inc a%(i%, j%)
    Next j%
  Next i%

  assert_int_equals(-3, a%(base% + 0, base% + 0))
  assert_int_equals(-2, a%(base% + 1, base% + 0))
  assert_int_equals(-1, a%(base% + 2, base% + 0))
  assert_int_equals( 0, a%(base% + 0, base% + 1))
  assert_int_equals( 1, a%(base% + 1, base% + 1))
  assert_int_equals( 2, a%(base% + 2, base% + 1))
  assert_int_equals( 3, a%(base% + 0, base% + 2))
  assert_int_equals( 4, a%(base% + 1, base% + 2))
  assert_int_equals( 5, a%(base% + 2, base% + 2))
End Sub

Sub test_inc_2d_float_array()
  Local a!(array.new%(3), array.new%(3)) = (-0.4, -0.3, -0.2, -0.1, 0.0, 0.1, 0.2, 0.3, 0.4)

  assert_float_equals(-0.4, a!(base% + 0, base% + 0), 0.001)
  assert_float_equals(-0.3, a!(base% + 1, base% + 0), 0.001)
  assert_float_equals(-0.2, a!(base% + 2, base% + 0), 0.001)
  assert_float_equals(-0.1, a!(base% + 0, base% + 1), 0.001)
  assert_float_equals( 0.0, a!(base% + 1, base% + 1), 0.001)
  assert_float_equals( 0.1, a!(base% + 2, base% + 1), 0.001)
  assert_float_equals( 0.2, a!(base% + 0, base% + 2), 0.001)
  assert_float_equals( 0.3, a!(base% + 1, base% + 2), 0.001)
  assert_float_equals( 0.4, a!(base% + 2, base% + 2), 0.001)

  Local i%, j%
  For i% = base% To base% + 2
    For j% = base% To base% + 2
      Inc a!(i%, j%), 0.3
    Next j%
  Next i%

  assert_float_equals(-0.1, a!(base% + 0, base% + 0), 0.001)
  assert_float_equals(-0.0, a!(base% + 1, base% + 0), 0.001)
  assert_float_equals( 0.1, a!(base% + 2, base% + 0), 0.001)
  assert_float_equals( 0.2, a!(base% + 0, base% + 1), 0.001)
  assert_float_equals( 0.3, a!(base% + 1, base% + 1), 0.001)
  assert_float_equals( 0.4, a!(base% + 2, base% + 1), 0.001)
  assert_float_equals( 0.5, a!(base% + 0, base% + 2), 0.001)
  assert_float_equals( 0.6, a!(base% + 1, base% + 2), 0.001)
  assert_float_equals( 0.7, a!(base% + 2, base% + 2), 0.001)
End Sub

Sub test_inc_2d_string_array()
  Local a$(array.new%(3), array.new%(3)) = ("a", "b", "c", "d", "e", "f", "g", "h", "i")

  assert_string_equals("a", a$(base% + 0, base% + 0))
  assert_string_equals("b", a$(base% + 1, base% + 0))
  assert_string_equals("c", a$(base% + 2, base% + 0))
  assert_string_equals("d", a$(base% + 0, base% + 1))
  assert_string_equals("e", a$(base% + 1, base% + 1))
  assert_string_equals("f", a$(base% + 2, base% + 1))
  assert_string_equals("g", a$(base% + 0, base% + 2))
  assert_string_equals("h", a$(base% + 1, base% + 2))
  assert_string_equals("i", a$(base% + 2, base% + 2))

  Local i%, j%
  For i% = base% To base% + 2
    For j% = base% To base% + 2
      Cat a$(i%, j%), Str$(i% - base% + 1) + Str$(j% - base% + 1)
    Next j%
  Next i%

  assert_string_equals("a11", a$(base% + 0, base% + 0))
  assert_string_equals("b21", a$(base% + 1, base% + 0))
  assert_string_equals("c31", a$(base% + 2, base% + 0))
  assert_string_equals("d12", a$(base% + 0, base% + 1))
  assert_string_equals("e22", a$(base% + 1, base% + 1))
  assert_string_equals("f32", a$(base% + 2, base% + 1))
  assert_string_equals("g13", a$(base% + 0, base% + 2))
  assert_string_equals("h23", a$(base% + 1, base% + 2))
  assert_string_equals("i33", a$(base% + 2, base% + 2))
End Sub

Sub test_inc_given_string_too_long()
  Local a$ = String$(128, "a")
  Local b$ = String$(128, "b")

  On Error Skip
  a$ = a$ + b$
  assert_raw_error("String too long")

  a$ = String$(128, "a")
  On Error Skip
  Inc a$, b$
  assert_raw_error("String too long")
End Sub
