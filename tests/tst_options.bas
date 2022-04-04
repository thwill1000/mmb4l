' Copyright (c) 2022 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 5.07

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
Const TMP$ = sys.string_prop$("tmpdir")
Dim fn_backup$(12)

If Mm.Device$ = "MMB4L" Then
add_test("test_option_load")
add_test("test_option_load_given_directory")
add_test("test_option_load_given_file_does_not_exist", "test_option_load_given_not_exist")
add_test("test_option_load_given_invalid_format", "test_option_load_given_invalid")
add_test("test_option_save")
add_test("test_option_save_given_invalid_path", "test_option_save_given_invalid")
add_test("test_option_save_given_directory", "test_option_save_given_directory")
EndIf

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
  fn_backup$(1) = Mm.Info$(Option F1)
  fn_backup$(2) = Mm.Info$(Option F2)
  fn_backup$(3) = Mm.Info$(Option F3)
  fn_backup$(4) = Mm.Info$(Option F4)
  fn_backup$(5) = Mm.Info$(Option F5)
  fn_backup$(6) = Mm.Info$(Option F6)
  fn_backup$(7) = Mm.Info$(Option F7)
  fn_backup$(8) = Mm.Info$(Option F8)
  fn_backup$(9) = Mm.Info$(Option F9)
  fn_backup$(10) = Mm.Info$(Option F10)
  fn_backup$(11) = Mm.Info$(Option F11)
  fn_backup$(12) = Mm.Info$(Option F12)
End Sub

Sub teardown_test()
  Option F1 fn_backup$(1)
  Option F2 fn_backup$(2)
  Option F3 fn_backup$(3)
  Option F4 fn_backup$(4)
  Option F5 fn_backup$(5)
  Option F6 fn_backup$(6)
  Option F7 fn_backup$(7)
  Option F8 fn_backup$(8)
  Option F9 fn_backup$(9)
  Option F10 fn_backup$(10)
  Option F11 fn_backup$(11)
  Option F12 fn_backup$(12)
End Sub

Sub test_option_load()
  Local filename$ = TMP$ + "/test_option_load"
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
  On Error Skip
  Option Load "/usr/bin"
  assert_raw_error("Is a directory")
End Sub

Sub test_option_load_given_not_exist()
  On Error Skip
  Option Load TMP$ + "/does_not_exist"
  assert_raw_error("No such file or directory")
End Sub

Sub test_option_load_given_invalid()
  Local filename$ = TMP$ + "/test_option_load_given_invalid"
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
  Local filename$ = TMP$ + "/test_option_save"
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
  Option Save TMP$ + "/invalid/path"
  assert_raw_error("No such file or directory")
End Sub

Sub test_option_save_given_directory()
  On Error Skip
  Option Save TMP$
  assert_raw_error("Is a directory")
End Sub
