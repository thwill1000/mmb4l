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

Const EXPECTED_HOME$ = "/home/thwill"

add_test("test_system_no_capture")
add_test("test_system_string_capture")
add_test("test_system_longstring_capture")
add_test("test_system_longstring_capture_given_too_long", "test_system_given_too_long")
add_test("test_system_given_invalid_syntax")
add_test("test_system_given_exit_status_arg", "test_system_exit_status_arg")
add_test("test_system_given_command_not_found", "test_system_command_not_found");
add_test("test_system_getenv")
add_test("test_system_setenv")
add_test("test_system_setenv_given_longstring", "test_system_setenv_given_ls")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_system_no_capture()
  System "echo 'foo bar'"
  assert_no_error()
End Sub

Sub test_system_string_capture()
  Local out$

  System "echo 'foo bar'", out$

  assert_string_equals("foo bar", out$)
End Sub

Sub test_system_longstring_capture()
  Local out%(array.new%(32));

  System "echo 'foo bar'", out%()

  assert_string_equals("foo bar", LGetStr$(out%(), 1, LLen(out%())))
End Sub

Sub test_system_given_too_long()
  Local out%(array.new%(2));

  System "echo '0123456789'", out%()

  ' Captured output is truncated.
  assert_string_equals("01234567", LGetStr$(out%(), 1, LLen(out%())))
End Sub

Sub test_system_given_invalid_syntax()
  Local i%, f!, s$(10), ia%(10, 10)

  On Error Skip 1
  System "echo 'foo bar'", i%
  assert_raw_error("Invalid 2nd argument; expected STRING or LONGSTRING")

  On Error Skip 1
  System "echo 'foo bar'", f!
  assert_raw_error("Invalid 2nd argument; expected STRING or LONGSTRING")

  On Error Skip 1
  System "echo 'foo bar'", s$()
  assert_raw_error("Invalid 2nd argument; expected STRING or LONGSTRING")

  On Error Skip 1
  System "echo 'foo bar'", ia%()
  assert_raw_error("Invalid 2nd argument; expected STRING or LONGSTRING")

  On Error Skip 1
  System "echo 'foo bar'",
  assert_raw_error("Syntax")
End Sub

Sub test_system_exit_status_arg()
  Local s$, exit_status%

  System "echo 'foo'", s$, exit_status%
  assert_string_equals("foo", s$)
  assert_int_equals(0, exit_status%)

  System "ls /does-not-exist", s$, exit_status%
  assert_string_equals("ls: cannot access '/does-not-exist': No such file or directory", s$)
  assert_int_equals(2, exit_status%)

  System "ls /does-not-exist", , exit_status%
  assert_int_equals(2, exit_status%)
End Sub

Sub test_system_command_not_found()
  Local s$, exit_status%

  On Error Skip 1
  System "foo", s$, exit_status%
  assert_raw_error("Unknown system command")
  assert_string_equals("sh: 1: foo: not found", s$)
  assert_int_equals(127, exit_status%)
End Sub

Sub test_system_getenv()
  Local i%, name$ = "HOME", value$, value_ls%(32)

  ' Given name is STRING literal and value is STRING variable.
  value$ = ""
  System GetEnv "HOME", value$
  assert_string_equals(EXPECTED_HOME$, value$)

  ' Given name is STRING literal and value is is LONGSTRING variable.
  LongString Clear value_ls%()
  System GetEnv "HOME", value_ls%()
  assert_string_equals(EXPECTED_HOME$, LGetStr$(value_ls%(), 1, LLen(value_ls%())))

  ' Given name is STRING variable and value is STRING variable.
  value$ = ""
  System GetEnv name$, value$
  assert_string_equals(EXPECTED_HOME$, value$)

  ' Given name is STRING variable and value is LONGSTRING variable.
  LongString Clear value_ls%()
  System GetEnv name$, value_ls%()
  assert_string_equals(EXPECTED_HOME$, LGetStr$(value_ls%(), 1, LLen(value_ls%())))

  ' Given name is LONGSTRING variable and value is STRING variable.
  LongString Clear value_ls%()
  LongString Append value_ls%(), "HOME"
  On Error Skip 1
  System GetEnv value_ls%(), value$
  assert_raw_error("Dimensions")

  ' Given value is STRING literal.
  On Error Skip 1
  System GetEnv "HOME", "FOO"
  assert_raw_error("Variable name")

  ' Given value is INTEGER.
  On Error Skip 1
  System GetEnv "HOME", i%
  assert_raw_error("Invalid 2nd argument; expected STRING or LONGSTRING")
End Sub

Sub test_system_setenv()
  Local i%, ls%(32), s$, value$

  ' Given name is STRING literal and value is STRING literal.
  value$ = ""
  System SetEnv "FOO", "bar"
  System GetEnv "FOO", value$
  assert_string_equals("bar", value$)

  ' Given name is STRING literal and value is STRING variable.
  s$ = "bar"
  value$ = ""
  System SetEnv "FOO", s$
  System GetEnv "FOO", value$
  assert_string_equals("bar", value$)

  ' Given name is STRING literal and value is STRING expression.
  s$ = "wom"
  value$ = ""
  System SetEnv "FOO", s$ + "bat"
  System GetEnv "FOO", value$
  assert_string_equals("wombat", value$)

  ' Given name is STRING variable and value is STRING literal.
  s$ = "FOO"
  value$ = ""
  System SetEnv s$, "bar"
  System GetEnv "FOO", value$
  assert_string_equals("bar", value$)

  ' Given name is STRING expression and value is STRING literal.
  s$ = "FO"
  value$ = ""
  System SetEnv s$ + "O", "bar"
  System GetEnv "FOO", value$
  assert_string_equals("bar", value$)

  ' Given name is STRING literal and value is LONGSTRING variable.
  LongString Clear ls%()
  LongString Append ls%(), "bar"
  value$ = ""
  System SetEnv "FOO", ls%()
  System GetEnv "FOO", value$
  assert_string_equals("bar", value$)

  ' Given name is STRING literal and value is a non-existing LONGSTRING variable.
  On Error Skip 1
  System SetEnv "FOO", ls2%()
  assert_raw_error("LS2 is not declared")

  ' Given name is STRING literal and value is an INTEGER function.
  '  - error message isn't great, but could be worse.
  On Error Skip 1
  System SetEnv "FOO", int_function%()
  assert_raw_error("INT_FUNCTION is not declared")
End Sub

Function int_function%()
  int_function% = 1
End Function

Sub test_system_setenv_given_ls()
  Local i%, ls_in%(100), ls_out%(100), s$
  For i% = 0 To 10
    LongString Append ls_in%(), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
  Next

  assert_int_equals(682, LLen(ls_in%()))

  System SetEnv "FOO", ls_in%()
  System GetEnv "FOO", ls_out%()

  assert_int_equals(682, LLen(ls_out%()))
  assert_int_array_equals(ls_in%(), ls_out%())

  On Error Skip 1
  System GetEnv "FOO", s$
  assert_raw_error("Environment variable value too long")
End Sub
