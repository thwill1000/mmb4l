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

Const base% = Mm.Info(Option Base)

Dim expected_key%
Dim actual_key%

add_test("test_on_key")
add_test("test_on_key_ascii")

run_tests()
'If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_on_key()
  Print
  Do While Inkey$ <> "" : Loop
  
  On Key on_key_handler()

  Local keys$(9) = ("q", "w", "e", "r", "t", "y", "ESC", "F3", "PAGE UP", "RIGHT")
  Local codes%(9) = (Asc("q"), Asc("w"), Asc("e"), Asc("r"), Asc("t"), Asc("y"), &h1B, &h93, &h88, &h83)
  Local i%
  For i% = 0 To 9
    Print "Press key [" keys$(i%) "]"
    expected_key% = codes%(i%)
    actual_key% = 0
    Do While actual_key% = 0 : Loop
    assert_int_equals(expected_key%, actual_key%)
  Next

  On Key 0
End Sub

Sub on_key_handler()
  Local k$ = Inkey$
  actual_key% = Asc(k$)
End Sub

Sub test_on_key_ascii()
  Print
  Do While Inkey$ <> "" : Loop

  On Key Asc("q"), on_key_q_handler()

  Local k$
  Print "Press key [q]"
  actual_key% = 0
  Do
    k$ = Inkey$
  Loop Until k$ <> "" Or actual_key% <> 0
  assert_int_equals(1, actual_key%)
  assert_string_equals("", k$)

  Print "Press key [w]"
  actual_key% = 0
  Do
    k$ = Inkey$
  Loop Until k$ <> "" Or actual_key% <> 0
  assert_string_equals("w", k$)
  assert_int_equals(0, actual_key%)

  On Key Asc("q"), 0
End Sub

Sub on_key_q_handler()
  actual_key% = 1
End Sub
