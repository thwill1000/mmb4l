' Copyright (c) 2024 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 5.07

Option Explicit On
Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1") > 0

#Include "../sptools/src/splib/system.inc"
#Include "../sptools/src/splib/array.inc"
#Include "../sptools/src/splib/list.inc"
#Include "../sptools/src/splib/string.inc"
#Include "../sptools/src/splib/file.inc"
#Include "../sptools/src/splib/vt100.inc"
#Include "../sptools/src/sptest/unittest.inc"

Const base% = Mm.Info(Option Base)

add_test("test_eval_simple_int")
add_test("test_eval_simple_string")
add_test("test_eval_user_function")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub test_eval_simple_int()
  assert_int_equals(7, eval("3 + 4"))
End Sub

Sub test_eval_simple_string()
  assert_string_equals("foobar", eval(str.quote$("foo") + "+" + str.quote$("bar")))
End Sub

Sub test_eval_user_function()
  assert_int_equals(42, eval("forty_two%()"))
End Sub

Function forty_two%()
  forty_two% = 42
End Function
