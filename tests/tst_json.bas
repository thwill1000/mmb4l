' Copyright (c) 2021-2022 Thomas Hugo Williams
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

Const BASE% = Mm.Info(Option Base)
Const RESOURCES_DIR$ = Mm.Info$(Path) + "/resources"

add_test("test_json")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_json()
  Local data%(1000), s$
  Open RESOURCES_DIR$ + "/test_json.json" For Input As #1
  Do While Not Eof(#1)
    Line Input #1, s$
    LongString Append data%(), s$
  Loop
  Close #1

  assert_string_equals("Awesome 4K", Json$(data%(), "name"))
  assert_string_equals("1280", Json$(data%(), "resolutions[0].width"))
  assert_string_equals("720",  Json$(data%(), "resolutions[0].height"))
  assert_string_equals("1920", Json$(data%(), "resolutions[1].width"))
  assert_string_equals("1080", Json$(data%(), "resolutions[1].height"))
  assert_string_equals("3840", Json$(data%(), "resolutions[2].width"))
  assert_string_equals("2160", Json$(data%(), "resolutions[2].height"))

  ' Test explicit empty string value.
  assert_string_equals("", Json$(data%(), "empty-string"))

  ' Test explicit null value.
  assert_string_equals("",       Json$(data%(), "null-value"))
  If Mm.Device$ = "MMB4L" Then
    assert_string_equals("<null>", Json$(data%(), "null-value", &b01))
    assert_string_equals("",       Json$(data%(), "null-value", &b10))
    assert_string_equals("<null>", Json$(data%(), "null-value", &b11))
  EndIf

  ' Test keys that do not exist.
  assert_string_equals("",          Json$(data%(), "does-not-exist"))
  If Mm.Device$ = "MMB4L" Then
    assert_string_equals("",          Json$(data%(), "does-not-exist", &b01))
    assert_string_equals("<missing>", Json$(data%(), "does-not-exist", &b10))
    assert_string_equals("<missing>", Json$(data%(), "does-not-exist", &b11))
    assert_string_equals("",          Json$(data%(), "resolutions[3].width"))
    assert_string_equals("",          Json$(data%(), "resolutions[3].width", &b01))
    assert_string_equals("<missing>", Json$(data%(), "resolutions[3].width", &b10))
    assert_string_equals("<missing>", Json$(data%(), "resolutions[3].width", &b11))
  EndIf
End Sub
