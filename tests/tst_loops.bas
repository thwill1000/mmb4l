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

add_test("test_do_loop")
add_test("test_do_loop_until")
add_test("test_do_loop_while")
add_test("test_do_until_loop")
add_test("test_do_until_wend")
add_test("test_do_while_loop")
add_test("test_do_while_loop_until")
add_test("test_do_while_loop_while")
add_test("test_do_while_wend")
add_test("test_while_loop")
add_test("test_while_wend")
add_test("test_for")
add_test("test_for_given_at_limit")
add_test("test_for_given_beyond_limit")
add_test("test_for_given_positive_step")
add_test("test_for_given_negative_step")
add_test("test_for_given_floats")
add_test("test_for_given_errors")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub test_do_loop()
  Local i% = 0
  Do
    Inc i%
    If i% = 10 Then Exit Do
  Loop

  assert_int_equals(10, i%)
End Sub

Sub test_do_loop_until()
  Local i% = 0
  Do
    Inc i%
  Loop Until i% = 10

  assert_int_equals(10, i%)
End Sub

Sub test_do_loop_while()
  Local i% = 0
  Do
    Inc i%
  Loop While i% < 10

  assert_int_equals(10, i%)
End Sub

Sub test_do_until_loop()
  Local i% = 0

  On Error Ignore
  Do Until i% = 10
    If Mm.ErrNo Then Goto skipA
    If i% = 11 Then Exit Do
    Inc i%
  Loop

skipA:
  assert_raw_error("DO has an UNTIL test")
  assert_int_equals(0, i%)
End Sub

Sub test_do_until_wend()
  Local i% = 0

  On Error Ignore
  Do Until i% = 10
    If Mm.ErrNo Then Goto skipB
    If i% = 11 Then Exit Do
    Inc i%
  Wend

skipB:
  assert_raw_error("DO has an UNTIL test")
  assert_int_equals(0, i%)
End Sub

Sub test_do_while_loop()
  Local i% = 0

  Do While i% < 10
    If i% = 11 Then Exit Do
    Inc i%
  Loop

  assert_int_equals(10, i%)
End Sub

Sub test_do_while_loop_until()
  Local i% = 0

  On Error Ignore
  Do While i% < 10
    If Mm.ErrNo Then Goto skipC
    If i% = 11 Then Exit Do
    Inc i%
  Loop Until 1

skipC:
  assert_raw_error("LOOP has an UNTIL test")
  assert_int_equals(0, i%)
End Sub

Sub test_do_while_loop_while()
  Local i% = 0

  On Error Ignore
  Do While i% < 10
    If Mm.ErrNo Then Goto skipD
    If i% = 11 Then Exit Do
    Inc i%
  Loop While 1

skipD:
  assert_raw_error("LOOP has a WHILE test")
  assert_int_equals(0, i%)
End Sub

Sub test_do_while_wend()
  Local i% = 0

  On Error Ignore
  Do While i% < 10
    If Mm.ErrNo Then Goto skipE
    If i% = 11 Then Exit Do
    Inc i%
  Wend

skipE:
  ' Possibly should be "No matching LOOP"
  assert_raw_error("WEND without a matching WHILE")
  assert_int_equals(1, i%)
End Sub

Sub test_while_loop()
  Local i% = 0

  On Error Ignore
  While i% < 10
    If Mm.ErrNo Then Goto skipF
    If i% = 11 Then Exit Do
    Inc i%
  Loop

skipF:
  assert_raw_error("No matching WEND")
  assert_int_equals(0, i%)
End Sub

Sub test_while_wend()
  Local i% = 0

  While i% < 10
    If i% = 11 Then Exit Do
    Inc i%
  Wend

  assert_int_equals(10, i%)
End Sub

Sub test_for()
  Local i%, j%
  For i% = 1 To 10
    Inc j%, 2
  Next

  assert_int_equals(11, i%)
  assert_int_equals(20, j%)
End Sub

Sub test_for_given_at_limit()
  Local i%, j%
  For i% = 5 To 5
    Inc j%, 2
  Next

  assert_int_equals(6, i%)
  assert_int_equals(2, j%)
End Sub

Sub test_for_given_beyond_limit()
  Local i%, j%
  For i% = 6 To 5
    Inc j%, 2
  Next

  assert_int_equals(6, i%)
  assert_int_equals(0, j%)
End Sub

Sub test_for_given_positive_step()
  Local i%, j%
  For i% = 1 To 10 Step 2
    Inc j%, 2
  Next

  assert_int_equals(11, i%)
  assert_int_equals(10, j%)
End Sub

Sub test_for_given_negative_step()
  Local i%, j%
  For i% = 12 To 0 Step -3
    Inc j%, 2
  Next

  assert_int_equals(-3, i%)
  assert_int_equals(10, j%)
End Sub

Sub test_for_given_floats()
  Local f!, j%
  For f! = 3.14 To 4.14 Step 0.5
    Inc j%, 2
  Next

  assert_float_equals(4.14, f!, 1e-8)
  assert_int_equals(4, j%)
End Sub

Sub test_for_given_errors()
  Local i%

  On Error Ignore
  For i% = 12
  assert_raw_error("FOR with misplaced = or TO")
  On Error Abort

  On Error Ignore
  For i% To 12
  assert_raw_error("FOR with misplaced = or TO")
  On Error Abort

  On Error Ignore
  For i% To 1 = 12
  assert_raw_error("FOR with misplaced = or TO")
  On Error Abort

  On Error Ignore
  For i% = 1 To 12 To 6
  assert_raw_error("Syntax")
  On Error Abort

  Const ci% = 5
  On Error Ignore
  For ci% = 1 To 10
  assert_raw_error("Cannot change a constant")
  On Error Abort

  Local s$
  On Error Ignore
  For s$ = "foo" To "bar"
  assert_raw_error("Invalid variable")
  On Error Abort

  On Error Ignore
  For i% = 1 To 10
  assert_raw_error("No matching NEXT")
  On Error Abort
End Sub
