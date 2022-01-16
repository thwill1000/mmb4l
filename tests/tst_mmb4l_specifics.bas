' Copyright (c) 2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

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
If Mm.Device$ <> "MMB4L" Then End

'add_test("test_system_no_capture")
add_test("test_system_string_capture")
add_test("test_system_long_string_capture")
add_test("test_system_given_too_long")
add_test("test_system_given_errors")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_system_no_capture()
  System "echo 'foo bar'"
End Sub

Sub test_system_string_capture()
  Local out$

  System "echo 'foo bar'", out$

  assert_string_equals("foo bar", out$)
End Sub

Sub test_system_long_string_capture()
  Local out%(array.new%(32));

  System "echo 'foo bar'", out%()

  assert_string_equals("foo bar", LGetStr$(out%(), 1, LLen(out%())))

  Local short%(array.new%(2))

  System "echo '0123456789'", short%()

  assert_string_equals("01234567", LGetStr$(short%(), 1, LLen(short%())))
End Sub

Sub test_system_given_too_long()
  Local out%(array.new%(2));

  System "echo '0123456789'", out%()

  assert_string_equals("01234567", LGetStr$(out%(), 1, LLen(out%())))
End Sub

Sub test_system_given_errors()
  Local i%, f!, s$(10), ia%(10, 10)

  On Error Skip 1
  System "echo 'foo bar'", i%
  assert_raw_error("Invalid 2nd argument; expected string or long string")

  On Error Skip 1
  System "echo 'foo bar'", f!
  assert_raw_error("Invalid 2nd argument; expected string or long string")

  On Error Skip 1
  System "echo 'foo bar'", s$()
  assert_raw_error("Invalid 2nd argument; expected string or long string")

  On Error Skip 1
  System "echo 'foo bar'", ia%()
  assert_raw_error("Invalid 2nd argument; expected string or long string")

  On Error Skip 1
  System "echo 'foo bar'",
  assert_raw_error("Syntax")
End Sub
