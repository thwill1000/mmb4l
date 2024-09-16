' Copyright (c) 2024 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMB4L 0.7.0

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

Select Case DEVICE$
  Case "MMB4L - Android aarch64"
    Const IS_ANDROID% = 1
    Const EXPECTED_HOME$ = "/data/data/com.termux/files/home"
  Case "MMB4L - Linux armv6l"
    Const IS_ANDROID% = 0
    Const EXPECTED_HOME$ = "/home/pi"
  Case Else
    Const IS_ANDROID% = 0
    Const EXPECTED_HOME$ = "/home/thwill"
End Select

add_test("test_setenv")
add_test("test_setenv_given_equals")
add_test("test_setenv_given_space_in_name")
add_test("test_setenv_given_space_in_value")
add_test("test_setenv_given_lower_case_name", "test_setenv_given_lc_name")
add_test("test_setenv_given_invalid_chars_in_name", "test_setenv_gvn_inv_chars_name")

skip_tests:

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub test_setenv()
  SetEnv "FOO", "bar"

  assert_string_equals("bar", Mm.Info$(EnvVar "FOO"))

  Local out$
  System "echo $FOO", out$
  assert_string_equals("bar", out$)
End Sub

Sub test_setenv_given_equals()
  SetEnv "FOO2" = "bar2"

  assert_string_equals("bar2", Mm.Info$(EnvVar "FOO2"))

  Local out$
  System "echo $FOO2", out$
  assert_string_equals("bar2", out$)
End Sub

Sub test_setenv_given_space_in_name()
  On Error Skip
  SetEnv "FOO BAR" = "bar"
  assert_raw_error("Invalid environment variable name")
End Sub

Sub test_setenv_given_space_in_value()
  SetEnv "FOO3" = "foo bar"

  assert_string_equals("foo bar", Mm.Info$(EnvVar "FOO3"))

  Local out$
  System "echo $FOO3", out$
  assert_string_equals("foo bar", out$)
End Sub

Sub test_setenv_given_lc_name()
  On Error Skip
  SetEnv "foo4" = "bar"
  assert_raw_error("Invalid environment variable name")
End Sub

Sub test_setenv_gvn_inv_chars_name()
  On Error Skip
  SetEnv "FOO5!" = "bar"
  assert_raw_error("Invalid environment variable name")
End Sub
