' Copyright (c) 2025 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>

Option Explicit On

Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1")  > 0

#Include "../sptools/src/splib/system.inc"
#Include "../sptools/src/splib/array.inc"
#Include "../sptools/src/splib/list.inc"
#Include "../sptools/src/splib/string.inc"
#Include "../sptools/src/splib/file.inc"
#Include "../sptools/src/splib/vt100.inc"
#Include "../sptools/src/sptest/unittest.inc"

Const BASE% = Mm.Info(Option Base)

add_test("test_select_case")
add_test("test_errors")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub test_select_case()
  assert_string_equals("DD", select_case$(0))
  assert_string_equals("AA", select_case$(1))
  assert_string_equals("BB", select_case$(2))
  assert_string_equals("BB", select_case$(3))
  assert_string_equals("BB", select_case$(4))
  assert_string_equals("CC", select_case$(5))
  assert_string_equals("CC", select_case$(6))
  assert_string_equals("CC", select_case$(7))
  assert_string_equals("DD", select_case$(8))
  assert_string_equals("DD", select_case$(9))
  assert_string_equals("EE", select_case$(10))
  assert_string_equals("FF", select_case$(11))
  assert_string_equals("GG", select_case$(12))
  assert_string_equals("HH", select_case$(13))
  assert_string_equals("HH", select_case$(14))
  assert_string_equals("II", select_case$(15))
End Sub

Function select_case$(i%)
  Select Case i%
    Case 1
      select_case$ = "AA"
    Case 2 To 4
      select_case$ = "BB"
    Case 5, 6, 7
      select_case$ = "CC"
    Case Is < 10
      select_case$ = "DD"
    Case 10 To 14
      Select Case i%
        Case 10: select_case$ = "EE"
        Case 11: select_case$ = "FF"
        Case < 13 : select_case$ = "GG"
        Case Else : select_case$ = "HH"
      End Select
    Case Else
      select_case$ = "II"
  End Select
End Function

Sub test_errors()
  On Error Ignore
  Select Case 5
  assert_raw_error("No matching END SELECT")
  On Error Abort
End Sub
