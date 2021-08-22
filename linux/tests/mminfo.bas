Option Explicit On
Option Default None
Option Base 0

Sub run_tests()
  test_mm_device()
End Sub

Sub test_mm_device()
  assert_string_equals("Linux x86_64", Mm.Device$)
End Sub

ut.init()
run_tests()
ut.report()

Sub ut.init()
  Dim ut.asserts_count%
  Dim ut.asserts_fails%
End Sub

Sub assert_hex_equals(expected%, actual%, chars%)
  Inc ut.asserts_count%
  If expected% <> actual% Then
    Local s$ = "Assert equals failed, expected &h" + Hex$(expected%, chars%)
    s$ = s$ + " but actually &h" + Hex$(actual%, chars%)
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

Sub ut.add_failure(msg$)
  Inc ut.asserts_fails%
  Print "FAILURE [" Str$(ut.asserts_count%) "] " msg$
End Sub

Sub ut.report()
  Print "Num successes: " Str$(ut.asserts_count% - ut.asserts_fails%)
  Print "Num failures:  " Str$(ut.asserts_fails%)
  Print Choice(ut.asserts_fails% = 0, "[OK]", "[FAIL]")
End Sub
