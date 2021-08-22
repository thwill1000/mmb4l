Option Explicit On
Option Default None
Option Base 0

Sub run_tests()
  test_pause_and_timer()
End Sub

Sub test_pause_and_timer()
  Timer = 0
  Pause 10
  assert_true(Abs(Timer - 10) < 2);

  Timer = 0
  Pause 100
  assert_true(Abs(Timer - 100) < 2);

  Timer = 0
  Pause 1000
  assert_true(Abs(Timer - 1000) < 2);
End Sub

ut.init()
run_tests()
ut.report()

Sub ut.init()
  Dim ut.asserts_count%
  Dim ut.asserts_fails%
End Sub

Sub assert_int_equals(expected%, actual%, chars%)
  Inc ut.asserts_count%
  If expected% <> actual% Then
    Local s$ = "Assert equals failed, expected " + Str$(expected%)
    s$ = s$ + " but actually " + Str$(actual%)
    ut.add_failure(s$)
  EndIf
End Sub

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
