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
Const TMPDIR$ = sys.string_prop$("tmpdir")

add_test("test_call_1")
add_test("test_call_2")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_call_1()
  assert_int_equals(3, Call("int_fn%"))
  assert_float_equals(3.12, Call("float_fn!"))
  assert_string_equals("foo", Call("string_fn$"))
End Sub

Function int_fn%()
  int_fn% = 3
End Function

Function float_fn!()
  float_fn! = 3.12
End Function

Function string_fn$()
  string_fn$ = "foo"
End Function

' Prior to 5.07.01 this would report an "Argument List" error from the Print #1 statement.
Sub test_call_2()
  Open TMPDIR$ + "/tst_call.tmp" For Output As #1
  Print #1, Call("int_fn%"), Call("float_fn!"), Call("string_fn$")
  Close #1

  Open TMPDIR$ + "/tst_call.tmp" For Input As #1
  Local s$
  Line Input #1, s$

  ' The difference seems to be something to do with how tabs are handled.
  If Mm.Device$ = "MMB4L" Or Mm.Device$ = "MMBasic for Windows" Then
    assert_string_equals(" 3   3.12   foo", s$)
  Else
    assert_string_equals(" 3   3.12 foo", s$)
  EndIf

  assert_int_equals(1, Eof(#1))
  Close #1
End Sub
