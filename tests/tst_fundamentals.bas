' Copyright (c) 2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

Option Explicit On
Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1")  > 0

#Include "splib/system.inc"
#Include "splib/array.inc"
#Include "splib/list.inc"
#Include "splib/string.inc"
#Include "splib/file.inc"
#Include "splib/vt100.inc"
#Include "sptest/unittest.inc"

Const BASE% = Mm.Info(Option Base)

add_test("test_unary_minus")
add_test("test_unary_plus")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
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
