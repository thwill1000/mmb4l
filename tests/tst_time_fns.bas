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

Dim t1%, t2%, t3%, t4%

add_test("test_date")
add_test("test_datetime")
add_test("test_day")
add_test("test_epoch")
add_test("test_time")
add_test("test_timer")
add_test("test_pause")
add_test("test_settick")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_date()
  If Mm.Device$ = "MMB4L" Then
    System "date '+%d-%m-%Y' > /tmp/test_date.txt"
    Open "/tmp/test_date.txt" For Input As #1
    Local s$
    Line Input #1, s$
    Close #1
    assert_string_equals(s$, Date$)
  Else
    Local old_date$ = Date$

    Date$ = "01-02-24"
    assert_string_equals("01-02-2024", Date$)


    Date$ = "02-02-2024"
    assert_string_equals("02-02-2024", Date$)

    Date$ = "01/02/25"
    assert_string_equals("01-02-2025", Date$)

    Date$ = "02/02/2025"
    assert_string_equals("02-02-2025", Date$)

    Date$ = old_date$
  EndIf
End Sub

Sub test_datetime()
  If Mm.Device$ = "MMB4L" Then
    Local out%(32);
    System "date -u '+%d-%m-%Y %H:%M:%S'", out%()
    Local expected$ = LGetStr$(out%(), 1, 19)
    assert_string_equals(expected$, DateTime$(Now))
  Else
    Local t$ = Time$
    Local d$ = Date$
    Local dt$ = DateTime$(Now)
    If (d$ + " " + t$ <> dt$) Then
      t$ = Time$
      d$ = Date$
      dt$ = DateTime$(Now)
    EndIf
    assert_string_equals(d$ + " " + t$, dt$)
  EndIf

  assert_string_equals("01-01-1970 00:00:00", DateTime$(0))
  assert_string_equals("03-01-1974 19:36:36", DateTime$(126473796))
  assert_string_equals("01-01-2000 00:00:00", DateTime$(946684800))
  assert_string_equals("25-08-2015 12:12:12", DateTime$(1440504732))
  assert_string_equals("31-12-1969 23:43:20", DateTime$(-1000))
End Sub

Sub test_day()
  assert_string_equals("Sunday", Day$("01-02-1970"))
  assert_string_equals("Thursday", Day$("03-01-1974"))
  assert_string_equals("Saturday", Day$("01-01-2000"))
  assert_string_equals("Wednesday", Day$("31-12-1969"))

  ' Test different ordering in the date.
  assert_string_equals("Tuesday", Day$("25-08-2015"))
  assert_string_equals("Tuesday", Day$("25-08-15"))
  assert_string_equals("Tuesday", Day$("2015-08-25"))

  assert_string_equals(Day$(Now), Day$(Date$))
End Sub

Sub test_epoch()
  assert_int_equals(0, Epoch("01-01-1970 00:00:00"))
  assert_int_equals(126473796, Epoch("03-01-1974 19:36:36"))
  assert_int_equals(946684800, Epoch("01-01-2000 00:00:00"))
  assert_int_equals(1440504732, Epoch("25-08-2015 12:12:12"))
  assert_int_equals(-1000, Epoch("31-12-1969 23:43:20"))

  If Mm.Device$ = "MMB4L" Then
    System "date '+%s' > /tmp/test_epoch.txt"
    Open "/tmp/test_epoch.txt" For Input As #1
    Local s$
    Line Input #1, s$
    Close #1
    assert_int_equals(Val(s$), Epoch(Now))
  Else
    assert_int_equals(Epoch(DateTime$(Now)), Epoch(Now))
  EndIf

  On Error Skip
  Local e% = Epoch("01-01-1900 00:00:00")
  assert_raw_error("Invalid date")
End Sub

Sub test_time()
  If Mm.Device$ = "MMB4L" Then
    System "date '+%H:%M:%S' > /tmp/test_time.txt"
    Open "/tmp/test_time.txt" For Input As #1
    Local s$
    Line Input #1, s$
    Close #1
    assert_string_equals(s$, Time$)
  Else
    Local old_time$ = Time$

    Time$ = "12:13:14"
    assert_string_equals("12:13:14", Time$)

    Time$ = 5 ' Adds 5 seconds.
    assert_string_equals("12:13:19", Time$)

    Time$ = -10 ' Subtracts 10 seconds.
    assert_string_equals("12:13:09", Time$)

    Time$ = old_time$
  EndIf
End Sub

Sub test_timer()
  Local old_timer% = Timer

  Local i%
  For i% = 0 To 200 Step 10
    Timer = i%
    assert_float_equals(i%, Timer, 1)
  Next

  Timer = old_timer%
End Sub

Sub test_pause()
  Local t% = Timer
  Pause 10
  assert_float_equals(Timer, t% + 10, 2)

  t% = Timer
  Pause 100
  assert_float_equals(Timer, t% + 100, 5)

  t% = Timer
  Pause 1000
  assert_float_equals(Timer, t% + 1000, 10)
End Sub

Sub test_settick()
  SetTick 100, inc_t1, 1
  SetTick  50, inc_t2, 2
  SetTick  10, inc_t3, 3
  SetTick   2, inc_t4, 4
  Pause 1000
'  Local t% = Timer
'  Do While Timer - t% < 1000 : Loop
  SetTick 0, inc_t4, 4
  SetTick 0, inc_t3, 3
  SetTick 0, inc_t2, 2
  SetTick 0, inc_t1, 1

  assert_float_equals(10, t1%, 1)
  assert_float_equals(20, t2%, 2)
  assert_float_equals(100, t3%, 3)
  assert_float_equals(500, t4%, 4)
End Sub

Sub inc_t1()
  'Print "inc_t1()"
  Inc t1%
End Sub

Sub inc_t2()
  'Print "inc_t2()"
  Inc t2%
End Sub

Sub inc_t3()
  'Print "inc_t3()"
  Inc t3%
End Sub

Sub inc_t4()
  'Print "inc_t4()"
  Inc t4%
End Sub
