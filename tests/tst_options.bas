' Copyright (c) 2022-2024 Thomas Hugo Williams
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

If Not sys.is_platform%("mmb4l") Then Goto skip_tests

Const BASE% = Mm.Info(Option Base)
Const CRLF$ = Chr$(13) + Chr$(10)
Const HOME$ = sys.HOME$()
Const IS_ANDROID% = Mm.Info$(Arch) = "Android aarch64"

add_test("test_option_load")
add_test("test_option_load_given_directory")
add_test("test_option_load_given_file_does_not_exist", "test_option_load_given_not_exist")
add_test("test_option_load_given_invalid_format", "test_option_load_given_invalid")
add_test("test_option_save")
add_test("test_option_save_given_invalid_path", "test_option_save_given_invalid")
add_test("test_option_save_given_directory", "test_option_save_given_directory")
add_test("test_option_reset")
add_test("test_option_reset_given_non_persistent", "test_option_reset_non_persistent")
add_test("test_option_reset_given_unknown")
add_test("test_option_reset_all")
add_test("test_option_reset_syntax_errors")
add_test("test_option_simulate")

skip_tests:

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
  ' Only saves options that are not at their defaults.
  MkDir TMPDIR$
  Option Save TMPDIR$ + "/mmbasic.options.bak"
End Sub

Sub teardown_test()
  Option Reset All
  Option Load TMPDIR$ + "/mmbasic.options.bak"
End Sub

Sub test_option_load()
  Local filename$ = TMPDIR$ + "/test_option_load"
  Open filename$ For Output As #1
  Print #1, "f1 = " + str.quote$("foo")
  Print #1, "f2 = " + str.quote$("bar")
  Print #1, "f3 = " + str.quote$("wom")
  Print #1, "f4 = " + str.quote$("bat")
  Close #1

  Option Load filename$

  assert_string_equals("foo", Mm.Info$(Option F1))
  assert_string_equals("bar", Mm.Info$(Option F2))
  assert_string_equals("wom", Mm.Info$(Option F3))
  assert_string_equals("bat", Mm.Info$(Option F4))
End Sub

Sub test_option_load_given_directory()
  Local dir$ = Choice(IS_ANDROID%, "/data/data/com.termux/files/usr/bin", "/usr/bin")

  On Error Skip
  Option Load dir$
  assert_raw_error("Is a directory")
End Sub

Sub test_option_load_given_not_exist()
  On Error Skip
  Option Load TMPDIR$ + "/does_not_exist"
  assert_raw_error("No such file or directory")
End Sub

Sub test_option_load_given_invalid()
  Local filename$ = TMPDIR$ + "/test_option_load_given_invalid"
  Open filename$ For Output As #1
  Print #1, "foo"
  Print #1, "bar"
  Close #1

  On Error Skip
  Option Load filename$
  assert_raw_error("Invalid format")
End Sub

Sub test_option_save()
  Local count%, s$

  Option F1 "foo"
  Option F2 "bar"
  Option F3 "wom"
  Option F4 "bat"
  Local filename$ = TMPDIR$ + "/test_option_save"
  Option Save filename$

  Open filename$ For Input As #1
  Do While Not Eof(#1)
    Line Input #1, s$
    If InStr(s$, "f1 = ") = 1 Then
      assert_string_equals("f1 = " + str.quote$("foo"), s$)
      Inc count%
    End If
    If InStr(s$, "f2 = ") = 1 Then
      assert_string_equals("f2 = " + str.quote$("bar"), s$)
      Inc count%
    End If
    If InStr(s$, "f3 = ") = 1 Then
      assert_string_equals("f3 = " + str.quote$("wom"), s$)
      Inc count%
    End If
    If InStr(s$, "f4 = ") = 1 Then
      assert_string_equals("f4 = " + str.quote$("bat"), s$)
      Inc count%
    End If
  Loop
  Close #1

  assert_int_equals(4, count%)
End Sub

Sub test_option_save_given_invalid()
  On Error Skip
  Option Save TMPDIR$ + "/invalid/path"
  assert_raw_error("No such file or directory")
End Sub

Sub test_option_save_given_directory()
  On Error Skip
  Option Save TMPDIR$
  assert_raw_error("Is a directory")
End Sub

Sub test_option_reset()
  Option Case "Upper"
  Option Editor "Vi"
  Option F4 "four"
  Option Search Path "~"
  Option Tab 8

  assert_string_equals("Upper",        Mm.Info$(Option Case))
  assert_string_equals("Vi",           Mm.Info$(Option Editor))
  assert_string_equals("four",         Mm.Info$(Option F4))
  assert_string_equals(HOME$,          Mm.Info$(Option Search Path))
  assert_int_equals(8,                 Mm.Info(Option Tab))

  Option Reset Case

  assert_string_equals("Title",        Mm.Info$(Option Case))
  assert_string_equals("Vi",           Mm.Info$(Option Editor))
  assert_string_equals("four",         Mm.Info$(Option F4))
  assert_string_equals(HOME$,          Mm.Info$(Option Search Path))
  assert_int_equals(8,                 Mm.Info(Option Tab))

  Option Reset Editor

  assert_string_equals("Title",        Mm.Info$(Option Case))
  assert_string_equals("Nano",         Mm.Info$(Option Editor))
  assert_string_equals("four",         Mm.Info$(Option F4))
  assert_string_equals(HOME$,          Mm.Info$(Option Search Path))
  assert_int_equals(8,                 Mm.Info(Option Tab))

  Option Reset F4

  assert_string_equals("Title",        Mm.Info$(Option Case))
  assert_string_equals("Nano",         Mm.Info$(Option Editor))
  assert_string_equals("EDIT" + CRLF$, Mm.Info$(Option F4))
  assert_string_equals(HOME$,          Mm.Info$(Option Search Path))
  assert_int_equals(8,                 Mm.Info(Option Tab))

  Option Reset Search Path

  assert_string_equals("Title",        Mm.Info$(Option Case))
  assert_string_equals("Nano",         Mm.Info$(Option Editor))
  assert_string_equals("EDIT" + CRLF$, Mm.Info$(Option F4))
  assert_string_equals("",             Mm.Info$(Option Search Path))
  assert_int_equals(8,                 Mm.Info(Option Tab))

  Option Reset Tab

  assert_string_equals("Title",        Mm.Info$(Option Case))
  assert_string_equals("Nano",         Mm.Info$(Option Editor))
  assert_string_equals("EDIT" + CRLF$, Mm.Info$(Option F4))
  assert_string_equals("",             Mm.Info$(Option Search Path))
  assert_int_equals(4,                 Mm.Info(Option Tab))
End Sub

Sub test_option_reset_non_persistent()
  On Error Skip
  Option Reset Break
  assert_raw_error("Invalid for non-persistent option")
End Sub

Sub test_option_reset_given_unknown()
  On Error Skip
  Option Reset Unknown
  assert_raw_error("Unknown option")
End Sub

Sub test_option_reset_all()
  Option Case "Upper"
  Option Editor "Vi"
  Option F1 "one"
  Option F2 "two"
  Option F3 "three"
  Option F4 "four"
  Option F5 "five"
  Option F6 "six"
  Option F7 "seven"
  Option F8 "eight"
  Option F9 "nine"
  Option F10 "ten"
  Option F11 "eleven"
  Option F12 "twelve"
  Option Search Path "~"
  Option Tab 8

  Option Reset All

  Const quotes$ = " " + Chr$(34) + Chr$(34) + Chr$(130)
  assert_string_equals("Title",                    Mm.Info$(Option Case))
  assert_string_equals("Nano",                     Mm.Info$(Option Editor))
  assert_string_equals("FILES" + CRLF$,            Mm.Info$(Option F1))
  assert_string_equals("RUN" + CRLF$,              Mm.Info$(Option F2))
  assert_string_equals("LIST" + CRLF$,             Mm.Info$(Option F3))
  assert_string_equals("EDIT" + CRLF$,             Mm.Info$(Option F4))
  assert_string_equals("AUTOSAVE" + quotes$,       Mm.Info$(Option F5))
  assert_string_equals("XMODEM RECEIVE" + quotes$, Mm.Info$(Option F6))
  assert_string_equals("XMODEM SEND" + quotes$,    Mm.Info$(Option F7))
  assert_string_equals("EDIT" + quotes$,           Mm.Info$(Option F8))
  assert_string_equals("LIST" + quotes$,           Mm.Info$(Option F9))
  assert_string_equals("RUN" + quotes$,            Mm.Info$(Option F10))
  assert_string_equals("",                         Mm.Info$(Option F11))
  assert_string_equals("",                         Mm.Info$(Option F12))
  assert_string_equals("",                         Mm.Info$(Option Search Path))
  assert_int_equals(4,                             Mm.Info(Option Tab))

  ' Non-persistent options should not have been reset.
  assert_int_equals(BASE%,                         Mm.Info(Option Base))
End Sub

Sub test_option_reset_syntax_errors()
  On Error Skip
  Option Reset All Case
  assert_raw_error("Syntax")

  On Error Skip
  Option Reset Case Editor
  assert_raw_error("Syntax")

  On Error Skip
  Option Reset Search Path Editor
  assert_raw_error("Syntax")

  On Error Skip
  Option Reset
  assert_raw_error("Syntax")
End Sub

Sub test_option_simulate()
  Option Simulate "Colour Maximite 2"

  assert_string_equals("Colour Maximite 2", Mm.Device$)
  assert_string_equals("Colour Maximite 2", Mm.Info$(Device))
  assert_string_equals("", Mm.Info$(Platform))

  Option Simulate "PicoMiteVGA"

  assert_string_equals("PicoMiteVGA", Mm.Device$)
  assert_string_equals("PicoMiteVGA", Mm.Info$(Device))
  assert_string_equals("", Mm.Info$(Platform))

  Option Simulate "Game*Mite"

  assert_string_equals("PicoMite", Mm.Device$)
  assert_string_equals("PicoMite", Mm.Info$(Device))
  assert_string_equals("Game*Mite", Mm.Info$(Platform))

  On Error Skip
  Option Simulate "FooBar"
  assert_raw_error("Invalid value")

  Option Simulate "MMB4L"

  assert_string_equals("MMB4L", Mm.Device$)
  assert_string_equals("MMB4L", Mm.Info$(Device))
  assert_string_equals("", Mm.Info$(Platform))
End Sub
