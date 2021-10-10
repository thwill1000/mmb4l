' Copyright (c) 2021 Thomas Hugo Williams
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

add_test("test_static_given_fn_value")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_static_given_fn_value()
  assert_string_equals("foo", static1$("foo"))
  assert_string_equals("foo", static1$())

  ' There is/was a firmware bug where if the input parameter to static2$() is
  ' the return value of another function then subsequent calls do not return
  ' the value of the static variable.
  assert_string_equals("foo", static2$(foo$()))
  assert_string_equals("foo", static2$())
End Sub

Function foo$()
  foo$ = "foo"
End Function

Function static1$(s$)
  Static a$ = s$
  static1$ = a$
End Function

Function static2$(s$)
  Static a$ = s$
  static2$ = a$
End Function

