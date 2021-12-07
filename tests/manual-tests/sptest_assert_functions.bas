Option Base InStr(Mm.CmdLine$, "--base=1")  > 0
Option Default None
Option Explicit On

#include "../../basic-src/splib/system.inc"
#include "../../basic-src/splib/array.inc"
#include "../../basic-src/splib/file.inc"
#include "../../basic-src/splib/list.inc"
#include "../../basic-src/splib/string.inc"
#include "../../basic-src/splib/vt100.inc"
#include "../../basic-src/sptest/unittest.inc"

Const BASE% = Mm.Info(Option Base)

add_test("test_diff_int_array_size")
add_test("test_diff_float_array_size")
add_test("test_diff_string_array_size")
add_test("test_assert_int_array_equals")
add_test("test_assert_float_array_equals")
add_test("test_assert_string_array_equals")
add_test("test_assert_int_array_max")
add_test("test_assert_float_array_max")
add_test("test_assert_string_array_max")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_diff_int_array_size()
  Local one%(array.new%(10)) = (1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
  Local two%(array.new%(8)) = (1, 2, 3, 4, 5, 6, 7, 8)

  assert_int_array_equals(one%(), two%())
End Sub

Sub test_diff_float_array_size()
  Local one!(array.new%(10)) = (1.5, 2.6, 3.7, 4.8, 5.9, 6.10, 7.11, 8.12, 9.13, 10.14)
  Local two!(array.new%(8)) = (1.5, 2.6, 3.7, 4.8, 5.9, 6.10, 7.11, 8.12)
  Local delta!(array.new%(10))
  Memory Set Float Peek(VarAddr delta!()), 1e-5, 10 ' Fill array with 1e-5

  assert_float_array_equals(one!(), two!(), delta!())
End Sub

Sub test_diff_string_array_size()
  Local one$(array.new%(6)) = ("one", "two", "three", "four", "five", "six")
  Local two$(array.new%(8)) = ("one", "two", "three", "four", "five", "six", "seven", "eight")

  assert_string_array_equals(one$(), two$())
End Sub

Sub test_assert_int_array_equals()
  Local one%(array.new%(10)) = (1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
  Local two%(array.new%(10)) = (5, 6, 3, 4, 7, 6, 8, 9, 9, 10)

  assert_int_array_equals(one%(), two%())
End Sub

Sub test_assert_float_array_equals()
  Local one!(array.new%(10)) = (1.5, 2.6, 3.7, 4.8, 5.9, 6.10, 7.11, 8.12, 9.13, 10.14)
  Local two!(array.new%(10)) = (2.5, 2.6, 3.7, 6.8, 5.9, 6.10, 7.11, 20.12, 21.13, 22.14)
  Local delta!(array.new%(10))
  Memory Set Float Peek(VarAddr delta!()), 1e-5, 10 ' Fill array with 1e-5

  assert_float_array_equals(one!(), two!(), delta!())
End Sub

Sub test_assert_string_array_equals()
  Local one$(array.new%(8)) = ("one", "two", "three", "four", "five", "six", "seven", "eight")
  Local two$(array.new%(8)) = ("one", "owt", "eerht", "four", "evif", "xis", "seven", "thgie")

  assert_string_array_equals(one$(), two$())
End Sub

Sub test_assert_int_array_max()
  Local one%(array.new%(20))
  Local two%(array.new%(20))
  Local i%
  For i% = BASE% To BASE% + 19
    one%(i%) = i%
    two%(i%) = i% + 1
  Next

  assert_int_array_equals(one%(), two%())
End Sub

Sub test_assert_float_array_max()
  Local one!(array.new%(20))
  Local two!(array.new%(20))
  Local i%
  For i% = BASE% To BASE% + 19
    one!(i%) = 0.33 * i%
    two!(i%) = 0.33 * i% + 1
  Next
  Local delta!(array.new%(20))
  Memory Set Float Peek(VarAddr delta!()), 1e-5, 20 ' Fill array with 1e-5

  assert_float_array_equals(one!(), two!(), delta!())
End Sub

Sub test_assert_string_array_max()
  Local one$(array.new%(20))
  Local two$(array.new%(20))
  Local i%
  For i% = BASE% To BASE% + 19
    one$(i%) = Chr$(33 + i%) + Chr$(34 + i%)
    two$(i%) = Chr$(34 + i%) + Chr$(35 + i%)
  Next

  assert_string_array_equals(one$(), two$())
End Sub
