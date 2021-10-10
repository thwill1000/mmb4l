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

Const BASE% = Mm.Info(Option Base)

add_test("test_pause_and_timer");

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_pause_and_timer()
  ' Save current timer.
  Local t% = Timer

  Timer = 0
  Pause 10
  assert_true(Abs(Timer - 10) < 2);

  Timer = 0
  Pause 100
  assert_true(Abs(Timer - 100) < 2);

  Timer = 0
  Pause 1000
  assert_true(Abs(Timer - 1000) < 2);

  ' Restore timer - this test will be reported to have taken ~0s.
  Timer = t%
End Sub
