' Copyright (c) 2020-2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

Option Explicit On
Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1") > 0

#Include "../../basic-src/splib/system.inc"
#Include "../../basic-src/splib/array.inc"
#Include "../../basic-src/splib/list.inc"
#Include "../../basic-src/splib/string.inc"
#Include "../../basic-src/splib/file.inc"
#Include "../../basic-src/splib/vt100.inc"
#Include "../../basic-src/sptest/unittest.inc"
#Include "../console.inc"

Const BASE% = Mm.Info(Option Base)

add_test("test_history_put")
add_test("test_history_get")
add_test("test_history_given_overflow")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_history_put()
  Local h%(array.new%(128))
  Local h_addr% = Peek(VarAddr h%())

  con.history_put(h%(), "foo")

  assert_int_equals(3,        Peek(Byte h_addr% + 0)))
  assert_int_equals(Asc("f"), Peek(Byte h_addr% + 1)))
  assert_int_equals(Asc("o"), Peek(Byte h_addr% + 2)))
  assert_int_equals(Asc("o"), Peek(Byte h_addr% + 3)))
  assert_int_equals(0,        Peek(Byte h_addr% + 4)))

  con.history_put(h%(), "snafu")

  assert_int_equals(5, Peek(Byte h_addr% + 0)))
  assert_int_equals(Asc("s"), Peek(Byte h_addr% + 1)))
  assert_int_equals(Asc("n"), Peek(Byte h_addr% + 2)))
  assert_int_equals(Asc("a"), Peek(Byte h_addr% + 3)))
  assert_int_equals(Asc("f"), Peek(Byte h_addr% + 4)))
  assert_int_equals(Asc("u"), Peek(Byte h_addr% + 5)))
  assert_int_equals(3,        Peek(Byte h_addr% + 6)))
  assert_int_equals(Asc("f"), Peek(Byte h_addr% + 7)))
  assert_int_equals(Asc("o"), Peek(Byte h_addr% + 8)))
  assert_int_equals(Asc("o"), Peek(Byte h_addr% + 9)))

  Local i%
  For i% = 10 To 1023
    If Peek(Byte h_addr% + i%) <> 0 Then
      assert_fail("Assert failed, byte " + Str$(i%) + " of h%() is non-zero")
      Exit For
    EndIf
  Next
End Sub

Sub test_history_get()
  Local h%(array.new%(128))
  Local h_addr% = Peek(VarAddr h%())

  con.history_put(h%(), "foo")
  con.history_put(h%(), "bar")
  con.history_put(h%(), "snafu")
  con.history_put(h%(), "wombat")

  assert_string_equals("wombat", con.history_get$(h%(), 0))
  assert_string_equals("snafu",  con.history_get$(h%(), 1))
  assert_string_equals("bar",    con.history_get$(h%(), 2))
  assert_string_equals("foo",    con.history_get$(h%(), 3))
  assert_string_equals("",       con.history_get$(h%(), 4))

  On Error Ignore
  Local s$ = con.history_get$(h%(), -1)
  assert_raw_error("index out of bounds: -1")
  On Error Abort
End Sub

Sub test_history_given_overflow()
  Local i%
  Local h%(array.new%(2)) ' 16 bytes
  Local h_addr% = Peek(VarAddr h%())
  Local check% = Peek(Byte h_addr% + 16) ' The byte one beyond the end of the array

  con.history_put(h%(), "foo")
  con.history_put(h%(), "bar")
  con.history_put(h%(), "snafu")
  con.history_put(h%(), "wombat")

  ' Array should have the values:
  '   <6>wombat<5>snafu<3>ba

  assert_int_equals(6,        Peek(Byte h_addr% + 0))
  assert_int_equals(Asc("u"), Peek(Byte h_addr% + 12))
  assert_int_equals(3,        Peek(Byte h_addr% + 13))
  assert_int_equals(Asc("b"), Peek(Byte h_addr% + 14))
  assert_int_equals(Asc("a"), Peek(Byte h_addr% + 15))
  assert_int_equals(check%,   Peek(Byte h_addr% + 16))

  assert_string_equals("wombat", con.history_get$(h%(), 0))
  assert_string_equals("snafu",  con.history_get$(h%(), 1))
  assert_string_equals("",       con.history_get$(h%(), 2))

  con.history_put(h%(), "a")

  assert_string_equals("a",      con.history_get$(h%(), 0))
  assert_string_equals("wombat", con.history_get$(h%(), 1))
  assert_string_equals("snafu",  con.history_get$(h%(), 2))
  assert_string_equals("",       con.history_get$(h%(), 3))
End Sub
