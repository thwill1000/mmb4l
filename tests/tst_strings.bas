' Copyright (c) 2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07.01
'
' Includes with permission tests derived from code by @Volhout on
' https://www.thebackshed.com

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

Const BASE% = Mm.Info(Option Base)

add_test("test_mid_function")
add_test("test_mid_command")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_mid_function()
  assert_string_equals("f", Mid$("foobar", 1, 1))
  assert_string_equals("oo", Mid$("foobar", 2, 2))
  assert_string_equals("bar", Mid$("foobar", 4, 10))
End Sub

Sub test_mid_command()
  Local a$ = "123456"

  Mid$(a$, 3, 3) = "aaa"
  assert_string_equals("12aaa6", a$)

  Mid$(a$, 1, 1) = "b"
  assert_string_equals("b2aaa6", a$)

  On Error Skip
  Mid$(a$, 1, 10) = "cccccccccc"
  Local expected$ = "10 is invalid (valid is 1 to 6)"
  assert_string_equals(expected$, Right$(Mm.ErrMsg$, Len(expected$)))
End Sub
