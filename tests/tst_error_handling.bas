' Copyright (c) 2022-2023 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 5.07

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
Const EXPECTED_ERROR_CODE% = Choice(sys.is_device%("mmb4l"), 256, 16)

Dim interrupt_called% = 0

If sys.is_device%("cmm2*") Then Goto skip_tests

add_test("test_system_error")
add_test("test_user_error")
add_test("Error in main thread not visible in interrupt", "test_error_in_normal")
add_test("Error in interrupt not visible in main thread", "test_error_in_interrupt")
add_test("test_error_given_pipe")
add_test("Skip in normal thread not swallowed by interrupt", "test_interrupt_not_swallow")
add_test("test_on_error_skip_2")
' Can't keep these tests enabled as they are designed to throw uncaught ERRORs.
' add_test("test_interrupt_does_not_ignore", "test_interrupt_not_ignore")
' add_test("test_editor_opens_correctly")
add_test("test_error_skip_with_if_block")

skip_tests:

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub test_system_error()
  Const BASE_LINE% = Val(Field$(Mm.Info$(Line), 1, ","))
  On Error Skip 1
  Local x% = 1 / 0
  If sys.is_device%("pm*") Then
    ' Error messages on the PicoMite do not include the line number,
    ' instead they offending line (with number) is printed to the console beforehand.
    assert_string_equals("Divide by zero", Mm.ErrMsg$)
  Else
    assert_string_equals(expected_error$(BASE_LINE% + 2, "Divide by zero"), Mm.ErrMsg$)
  EndIf
End Sub

Function expected_error$(line%, msg$)
  If sys.is_device%("pm*") Then
    expected_error$ = msg$
  Else
    expected_error$ = "Error in line " + Str$(line%) + ": " + msg$
  EndIf
End Function

Sub test_user_error()
  Const BASE_LINE% = Val(Field$(Mm.Info$(Line), 1, ","))
  On Error Skip 1
  Error "foo"
  assert_string_equals(expected_error$(BASE_LINE% + 2, "foo"), Mm.ErrMsg$)
End Sub

' Test that an error thrown in the normal thread of execution is not visible
' in an interrupt.
Sub test_error_in_normal()
  Const BASE_LINE% = Val(Field$(Mm.Info$(Line), 1, ","))
  interrupt_called% = 0
  On Error Skip 1
  Error "foo"
  SetTick 10, interrupt1
  Pause 20

  assert_true(interrupt_called%)
  assert_int_equals(EXPECTED_ERROR_CODE%, Mm.ErrNo)
  assert_string_equals(expected_error$(BASE_LINE% + 3, "foo"), Mm.ErrMsg$)
End Sub

Sub interrupt1()
  interrupt_called% = 1
  assert_int_equals(0, Mm.ErrNo)
  assert_string_equals("", Mm.ErrMsg$)
  SetTick 0, interrupt1
End Sub

' Test that an error thrown in an interrupt is not visible in the normal
' thread of execution.
Sub test_error_in_interrupt()
  On Error Clear
  interrupt_called% = 0
  SetTick 10, interrupt2
  Pause 20
  assert_true(interrupt_called%)
  assert_int_equals(0, Mm.ErrNo)
  assert_string_equals("", Mm.ErrMsg$)
End Sub

Sub interrupt2()
  Const BASE_LINE% = Val(Field$(Mm.Info$(Line), 1, ","))
  interrupt_called% = 1
  On Error Skip 1
  Error "foo"
  assert_int_equals(EXPECTED_ERROR_CODE%, Mm.ErrNo)
  assert_string_equals(expected_error$(BASE_LINE% + 3, "foo"), Mm.ErrMsg$)
  SetTick 0, interrupt2
End Sub

Sub test_error_given_pipe()
  Const BASE_LINE% = Val(Field$(Mm.Info$(Line), 1, ","))
  On Error Skip 2
  Local s$ = "Hello|World" : Error "foo"
  assert_int_equals(EXPECTED_ERROR_CODE%, Mm.ErrNo)
  assert_string_equals(expected_error$(BASE_LINE% + 2, "foo"), Mm.ErrMsg$)
End Sub

' Test that a skip in the normal thread of execution is not swallowed by
' an interrupt.
Sub test_interrupt_not_swallow()
  Const BASE_LINE% = Val(Field$(Mm.Info$(Line), 1, ","))
  SetTick 1, interrupt3
  Local i%
  For i% = 1 To 1000
    On Error Skip 1
    Error "foo" ' Should always be skipped
  Next
  assert_int_equals(EXPECTED_ERROR_CODE%, Mm.ErrNo)
  assert_string_equals(expected_error$(BASE_LINE% + 5, "foo"), Mm.ErrMsg$)
  SetTick 0, interrupt3
End Sub

Sub interrupt3()
  Local z% = 0 ' Any single statement
End Sub

' Test that an ignore in an interrupt does not skip an error in the normal thread
' of execution.
Sub test_interrupt_not_ignore()
  interrupt_called% = 0
  SetTick 10, interrupt4
  Do While Not interrupt_called% : Loop
  Error "foo"
  On Error Clear
End Sub

Sub interrupt4()
  interrupt_called% = 1
  SetTick 0, interrupt4
  On Error Ignore
End Sub

Sub test_on_error_skip_2()
  Const BASE_LINE% = Val(Field$(Mm.Info$(Line), 1, ","))
  SetTick 1, interrupt3
  Local i%
  For i% = 1 To 1000
    On Error Skip 2
    Error "foo" ' Should always be skipped
    Error "bar" ' Should always be skipped
    ' Error "wombat"
  Next
  assert_int_equals(EXPECTED_ERROR_CODE%, Mm.ErrNo)
  assert_string_equals(expected_error$(BASE_LINE% + 6, "bar"), Mm.ErrMsg$)
  SetTick 0, interrupt3
End Sub

Sub test_editor_opens_correctly()
'  On Error Skip 1
'  Error "test_editor_opens_correctly"
'  Print Mm.ErrMsg$
'  End
  interrupt_called% = 0
  SetTick 10, interrupt5
  Do While Not interrupt_called% : Loop
End Sub

Sub interrupt5()
  interrupt_called% = 1
  Error "interrupt5"
End Sub

Sub test_error_skip_with_if_block()
  Local a%, even%, z%

  a% = 4
  On Error Skip 5
  If a% Mod 2 = 0 Then
    even% = true
    z% = 1 ' deliberate extra command.
  Else
    even% = false
  EndIf
  z% = 1 / 0
  assert_raw_error("Divide by zero")

  a% = 5
  On Error Skip 4 ' one less command to skip.
  If a% Mod 2 = 0 Then
    even% = true
    z% = 1 ' deliberate extra command.
  Else
    even% = false
  EndIf
  z% = 1 / 0
  assert_raw_error("Divide by zero")
End Sub
