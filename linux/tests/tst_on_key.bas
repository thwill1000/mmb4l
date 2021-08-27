Option Explicit On
Option Default None
Option Base 0

Dim expected_key%
Dim actual_key%

Sub run_tests()
  test_on_key()
  test_on_key_ascii()
End Sub

Sub test_on_key()
  Local keys$(9) = ("q", "w", "e", "r", "t", "y", "ESC", "F3", "PAGE UP", "RIGHT")
  Local codes%(9) = (Asc("q"), Asc("w"), Asc("e"), Asc("r"), Asc("t"), Asc("y"), &h1B, &h93, &h88, &h83)
  On Key on_key_handler()
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
  Local k$

  Do While Inkey$ <> "" : Loop

  On Key Asc("q"), on_key_q_handler()
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

ut.init()
run_tests()
ut.report()

Sub ut.init()
  Dim ut.asserts_count%
  Dim ut.asserts_fails%
End Sub

Sub assert_int_equals(expected%, actual%)
  Inc ut.asserts_count%
  If expected% <> actual% Then
    Local s$ = "Assert equals failed, expected " + Str$(expected%)
    s$ = s$ + " but actually " + Str$(actual%)
    ut.add_failure(s$)
  EndIf
End Sub

Sub assert_string_equals(expected_$, actual_$)
  Inc ut.asserts_count%
  If expected_$ <> actual_$ Then
    Local expected$ = Chr$(34) + expected_$ + Chr$(34)
    Local actual$ = Chr$(34) + actual_$ + Chr$(34)
    If Len(expected_$) = 1 Then expected$ = "Chr$(" + Str$(Asc(expected_$)) + ")"
    If Len(actual_$) = 1 Then actual$ = "Chr$(" + Str$(Asc(actual_$)) + ")"

    Local s$ = "Assert equals failed, expected " + ut.sanitise_string$(expected$)
    Inc s$, " but actually " + ut.sanitise_string$(actual$)
    ut.add_failure(s$)
  EndIf
End Sub

Function ut.sanitise_string$(s$)
  Local c%, i%, s2$
  For i% = 1 To Len(s$)
    c% = Peek(Var s$, i%)
    Inc s2$, Choice(c% < 32 Or c% > 126, "<" + Str$(c%) + ">", Chr$(c%))
  Next
  ut.sanitise_string$ = s2$
End Function

Sub assert_true(z%, msg$)
  Inc ut.asserts_count%
  If Not z% Then ut.add_failure(Choice(msg$ = "", "assert_true() failed", msg$))
End Sub

Sub ut.add_failure(msg$)
  Inc ut.asserts_fails%
  Print "FAILURE [" Str$(ut.asserts_count%) "] " msg$
End Sub

Sub ut.report()
  Print "Num successes: " Str$(ut.asserts_count% - ut.asserts_fails%)
  Print "Num failures:  " Str$(ut.asserts_fails%)
  Print Choice(ut.asserts_fails% = 0, "[OK]", "[FAIL]")
End Sub
