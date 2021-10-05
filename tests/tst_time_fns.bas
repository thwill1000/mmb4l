' Copyright (c) 2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

Option Explicit On
Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1") > 0

#Include "splib/system.inc"
#Include "splib/array.inc"
#Include "splib/list.inc"
#Include "splib/string.inc"
#Include "splib/file.inc"
#Include "splib/vt100.inc"
#Include "sptest/unittest.inc"

Const BASE% = Mm.Info(Option Base)

add_test("test_pause_and_timer");

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_pause_and_timer()
  Local t% = Timer
  Pause 10
  assert_true(Abs(Timer - t% - 10) < 2)
'  assert_int_equals(Timer, t% + 10)

  t% = Timer
  Pause 100
  assert_true(Abs(Timer - t% - 100) < 2)
'  assert_int_equals(Timer, t% + 100)

  t% = Timer
  Pause 1000
  assert_true(Abs(Timer - t% - 1000) < 2)
'  assert_int_equals(Timer, t% + 1000)
End Sub
