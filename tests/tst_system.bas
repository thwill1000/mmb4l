' Copyright (c) 2021-2026 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 6

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

If sys.is_platform%("cmm2*", "pm*") Then Goto skip_tests

Const BASE% = Mm.Info(Option Base)
Const DEVICE$ = Choice(Mm.Device$ = "MMB4L", Mm.Device$ + " - " + Mm.Info$(Arch), Mm.Device$)
Const IS_WINDOWS% = sys.is_windows%()
Const IS_ANDROID% = (DEVICE$ = "MMB4L - Android aarch64")

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

skip_tests:

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub test_system_no_capture()
  System "echo 'foo bar'"
  assert_no_error()
End Sub

Sub test_system_string_capture()
  Local out$

  System "echo 'foo bar'", out$

  Const expected$ = Choice(IS_WINDOWS%, "'foo bar'", "foo bar")
  assert_string_equals(expected$, out$)
End Sub

Sub test_system_longstring_capture()
  Local out%(array.new%(32));

  System "echo 'foo bar'", out%()

  Const expected$ = Choice(IS_WINDOWS%, "'foo bar'", "foo bar")
  assert_string_equals(expected$, LGetStr$(out%(), 1, LLen(out%())))
End Sub

Sub test_system_given_too_long()
  Local out%(array.new%(2));

  System "echo '0123456789'", out%()

  ' Captured output is truncated to 8 characters.
  Const expected$ = Choice(IS_WINDOWS%, "'0123456", "01234567")
  assert_string_equals(expected$, LGetStr$(out%(), 1, LLen(out%())))
End Sub

Sub test_system_given_invalid_syntax()
  Local i%, f!, s$(10), ia%(10, 10)

  On Error Skip 1
  System "echo 'foo bar'", i%
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", "Invalid 2nd argument; expected STRING or LONGSTRING", "Invalid variable"))

  On Error Skip 1
  System "echo 'foo bar'", f!
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", "Invalid 2nd argument; expected STRING or LONGSTRING", "Syntax"))

  On Error Skip 1
  System "echo 'foo bar'", s$()
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", "Invalid 2nd argument; expected STRING or LONGSTRING", "Invalid variable"))

  On Error Skip 1
  System "echo 'foo bar'", ia%()
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", "Invalid 2nd argument; expected STRING or LONGSTRING", "Invalid variable"))

  On Error Skip 1
  System "echo 'foo bar'",
  assert_raw_error("Syntax")
End Sub

Sub test_system_exit_status_arg()
  If Mm.Device$ <> "MMB4L" Then Exit Sub

  Local s$, exit_status%

  System "echo 'foo'", s$, exit_status%
  assert_string_equals(Choice(IS_WINDOWS%, "'foo'", "foo"), s$)
  assert_int_equals(0, exit_status%)

  If IS_WINDOWS% Then
    System "dir /b C:\does-not-exist", s$, exit_status%
    assert_string_equals("File Not Found", s$)
    assert_int_equals(1, exit_status%)
  Else
    System "ls /does-not-exist", s$, exit_status%
    assert_string_equals("ls: cannot access '/does-not-exist': No such file or directory", s$)
    assert_int_equals(2, exit_status%)
  EndIf
End Sub

Sub test_system_command_not_found()
  Local s$, exit_status%

  If Mm.Device$ = "MMB4L" Then
    On Error Skip 1
    System "foo", s$, exit_status%
    If Not IS_WINDOWS% Then
      assert_raw_error("Unknown system command")
    EndIf
    Local expected$
    If IS_WINDOWS% Then
      expected$ = "'foo' is not recognized as an internal or external command," + Chr$(10) + "operable program or batch file"
    ElseIf IS_ANDROID% Then
      expected$ = "sh: foo: not found"
    Else
      expected$ = "sh: 1: foo: not found"
    EndIf
    assert_int_equals(Choice(IS_WINDOWS%, 1, 127), exit_status%)
  Else
    On Error Skip 1
    System "foo", s$
    assert_raw_error("")
    assert_string_equals("", s$)
  EndIf
End Sub

Sub test_system_getenv()
  If Mm.Device$ <> "MMB4L" Then Exit Sub

  ' use SYSTEM command to get current username.
  Local whoami$
  If IS_WINDOWS% Then
    System "echo %USERNAME%", whoami$
  Else
    System "whoami", whoami$
  EndIf

  Const expected$ = Choice(IS_WINDOWS%, "C:\Users\", "/home/") + whoami$
  Const varname$ = Choice(IS_WINDOWS%, "USERPROFILE", "HOME")
  Local i%, value$, value_ls%(32)

  ' Given name is STRING literal and value is STRING variable.
  value$ = ""
  If IS_WINDOWS% Then
    System GetEnv "USERPROFILE", value$
  Else
    System GetEnv "HOME", value$
  EndIf
  assert_string_equals(expected$, value$)

  ' Given name is STRING literal and value is is LONGSTRING variable.
  LongString Clear value_ls%()
  If IS_WINDOWS% Then
    System GetEnv "USERPROFILE", value_ls%()
  Else
    System GetEnv "HOME", value_ls%()
  EndIf
  assert_string_equals(expected$, LGetStr$(value_ls%(), 1, LLen(value_ls%())))

  ' Given name is STRING variable and value is STRING variable.
  value$ = ""
  System GetEnv varname$, value$
  assert_string_equals(expected$, value$)

  ' Given name is STRING variable and value is LONGSTRING variable.
  LongString Clear value_ls%()
  System GetEnv varname$, value_ls%()
  assert_string_equals(expected$, LGetStr$(value_ls%(), 1, LLen(value_ls%())))

  ' Given name is LONGSTRING variable and value is STRING variable.
  LongString Clear value_ls%()
  LongString Append value_ls%(), "HOME"
  On Error Skip 1
  System GetEnv value_ls%(), value$
  assert_raw_error("Dimensions")

  ' Given value is STRING literal.
  On Error Skip 1
  System GetEnv "HOME", "FOO"
  assert_raw_error("Invalid variable name")

  ' Given value is INTEGER.
  On Error Skip 1
  System GetEnv "HOME", i%
  assert_raw_error("Invalid 2nd argument; expected STRING or LONGSTRING")
End Sub

Sub test_system_setenv()
  If Mm.Device$ <> "MMB4L" Then Exit Sub

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
  If Mm.Device$ <> "MMB4L" Then Exit Sub

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
