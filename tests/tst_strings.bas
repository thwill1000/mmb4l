' Copyright (c) 2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07.01
'
' Includes with permission tests derived from code by @Volhout on
' https://www.thebackshed.com

Option Explicit On
Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1") > 0

#Include "../basic-src/splib/system.inc"
#Include "../basic-src/splib/array.inc"
#Include "../basic-src/splib/list.inc"
#Include "../basic-src/splib/string.inc"
#Include "../basic-src/splib/file.inc"
#Include "../basic-src/splib/vt100.inc"
#Include "../basic-src/sptest/unittest.inc"

Const BASE% = Mm.Info(Option Base)

add_test("test_chr_ascii")
add_test("test_chr_utf8")
add_test("test_mid_function")
add_test("test_mid_command")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_chr_ascii()
  assert_string_equals("1", Chr$(49))
  assert_string_equals("f", Chr$(102))
  assert_string_equals("Z", Chr$(90))
  assert_string_equals("(", Chr$(40))
  Local s$ = Chr$(255)
  assert_int_equals(1, Peek(Byte Peek(VarAddr s$)))
  assert_int_equals(255, Peek(Byte Peek(VarAddr s$) + 1))
End Sub

Sub test_chr_utf8()
  ' 1-byte unicode = ASCII
  Local s$ = Chr$(Utf8 102)
  assert_int_equals(1, Peek(Byte Peek(VarAddr s$)))
  assert_int_equals(102, Peek(Byte Peek(VarAddr s$) + 1))

  ' 2-byte unicode
  s$ = Chr$(Utf8 &h00A3)
  assert_int_equals(2, Peek(Byte Peek(VarAddr s$)))
  assert_int_equals(&hC2, Peek(Byte Peek(VarAddr s$) + 1))
  assert_int_equals(&hA3, Peek(Byte Peek(VarAddr s$) + 2))

  ' 3-byte unicode
  s$ = Chr$(Utf8 &hFB40)
  assert_int_equals(3, Peek(Byte Peek(VarAddr s$)))
  assert_int_equals(&hEF, Peek(Byte Peek(VarAddr s$) + 1))
  assert_int_equals(&hAD, Peek(Byte Peek(VarAddr s$) + 2))
  assert_int_equals(&h80, Peek(Byte Peek(VarAddr s$) + 3))

  ' 4-byte unicode
  s$ = Chr$(Utf8 &h12013)
  assert_int_equals(4, Peek(Byte Peek(VarAddr s$)))
  assert_int_equals(&hF0, Peek(Byte Peek(VarAddr s$) + 1))
  assert_int_equals(&h92, Peek(Byte Peek(VarAddr s$) + 2))
  assert_int_equals(&h80, Peek(Byte Peek(VarAddr s$) + 3))
  assert_int_equals(&h93, Peek(Byte Peek(VarAddr s$) + 4))
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
