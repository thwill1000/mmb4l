' Copyright (c) 2021-2023 Thomas Hugo Williams
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

If sys.is_device%("pm*") Then Goto skip_tests

Const BASE% = Mm.Info(Option Base)

add_test("test_json")

skip_tests:

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub test_json()
  If sys.is_device%("pm*") Then Exit Sub

  MkDir TMPDIR$
  Const f$ = TMPDIR$ + "/test_json.json"
  ut.write_data_file(f$, "data_test_json")

  Local data%(1000), s$
  Open f$ For Input As #1
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
  If sys.is_device%("mmb4l") Then
    assert_string_equals("<null>", Json$(data%(), "null-value", &b01))
    assert_string_equals("",       Json$(data%(), "null-value", &b10))
    assert_string_equals("<null>", Json$(data%(), "null-value", &b11))
  EndIf

  ' Test keys that do not exist.
  assert_string_equals("",          Json$(data%(), "does-not-exist"))
  If sys.is_device%("mmb4l") Then
    assert_string_equals("",          Json$(data%(), "does-not-exist", &b01))
    assert_string_equals("<missing>", Json$(data%(), "does-not-exist", &b10))
    assert_string_equals("<missing>", Json$(data%(), "does-not-exist", &b11))
    assert_string_equals("",          Json$(data%(), "resolutions[3].width"))
    assert_string_equals("",          Json$(data%(), "resolutions[3].width", &b01))
    assert_string_equals("<missing>", Json$(data%(), "resolutions[3].width", &b10))
    assert_string_equals("<missing>", Json$(data%(), "resolutions[3].width", &b11))
  EndIf
End Sub

data_test_json:
Data "text/json"
Data "{"
Data "    'name': 'Awesome 4K',"
Data "    'resolutions': ["
Data "        {"
Data "            'width': 1280,"
Data "            'height': 720"
Data "        },"
Data "        {"
Data "            'width': 1920,"
Data "            'height': 1080"
Data "        },"
Data "        {"
Data "            'width': 3840,"
Data "            'height': 2160"
Data "        }"
Data "    ],"
Data "    'empty-string': '',"
Data "    'null-value': null"
Data "}"
Data "<EOF>"
