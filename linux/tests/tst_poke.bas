Option Explicit On
Option Default None
Option Base 0

Const BASE% = Mm.Info(Option Base)

Sub run_tests()
  test_poke_byte()
  test_poke_float()
  test_poke_integer()
  test_poke_short()
  test_poke_var()
  test_poke_word()
End Sub

Sub test_poke_byte()
  Local num% = &h0102030405060708
  Local num_addr% = Peek(VarAddr num%)

  Poke Byte num_addr% + 0, &h09
  Poke Byte num_addr% + 1, &h0A
  Poke Byte num_addr% + 2, &h0B
  Poke Byte num_addr% + 3, &h0C
  Poke Byte num_addr% + 4, &h0D
  Poke Byte num_addr% + 5, &h0E
  Poke Byte num_addr% + 6, &h0F
  Poke Byte num_addr% + 7, &h10

  assert_hex_equals(&h100F0E0D0C0B0A09, num%)
End Sub

Sub test_poke_float()
  Local arr!(BASE% + 1) = (3.142, -3.142e23)
  Local arr_addr% = Peek(VarAddr arr!())

  Poke Float arr_addr%, 42.013
  Poke Float arr_addr% + 8, -2.1e51

  assert_float_equals(42.013, arr!(BASE% + 0))
  assert_float_equals(-2.1e51, arr!(BASE% + 1))

  On Error Skip 1
  Poke Float arr_addr% + 1, 0.0
  assert_raw_error("Address not divisible by 8")
End Sub

Sub test_poke_integer()
  Local arr%(BASE% + 1) = (&h0102030405060708, &h090a0b0c0d0e0f10)
  Local arr_addr% = Peek(VarAddr arr%())

  Poke Integer arr_addr%, &h0807060504030201
  Poke Integer arr_addr% + 8, &h100F0E0D0C0B0A09

  assert_hex_equals(&h0807060504030201, arr%(BASE% + 0), 16)
  assert_hex_equals(&h100F0E0D0C0B0A09, arr%(BASE% + 1), 16)

  On Error Skip 1
  Poke Integer arr_addr% + 1, 42
  assert_raw_error("Address not divisible by 8")
End Sub

Sub test_poke_short()
  Local num% = &h0102030405060708
  Local num_addr% = Peek(VarAddr num%)

  Poke Short num_addr% + 0, &h0A09
  Poke Short num_addr% + 2, &h0C0B
  Poke Short num_addr% + 4, &h0E0D
  Poke Short num_addr% + 6, &h100F

  assert_hex_equals(&h100F0E0D0C0B0A09, num%)

  On Error Skip 1
  Poke Short num_addr% + 1, &h0000
  assert_raw_error("Address not divisible by 2")
End Sub

Sub test_poke_var()
  Local num% = &h0102030405060708

  Poke Var num%, 0, &h09
  Poke Var num%, 1, &h0A
  Poke Var num%, 2, &h0B
  Poke Var num%, 3, &h0C
  Poke Var num%, 4, &h0D
  Poke Var num%, 5, &h0E
  Poke Var num%, 6, &h0F
  Poke Var num%, 7, &h10

  assert_hex_equals(&h100F0E0D0C0B0A09, num%)
End Sub

Sub test_poke_word()
  Local num% = &h0102030405060708
  Local num_addr% = Peek(VarAddr num%)

  Poke Word num_addr% + 0, &h0C0B0A09
  Poke Word num_addr% + 4, &h100F0E0D
  assert_hex_equals(&h100F0E0D0C0B0A09, num%)

  On Error Skip 1
  Poke Word num_addr% + 1, 42
  assert_raw_error("Address not divisible by 4")
End Sub

ut.init()
run_tests()
ut.report()

Sub ut.init()
  Dim ut.asserts_count%
  Dim ut.asserts_fails%
End Sub

Sub assert_float_equals(expected!, actual!, delta!)
  Inc ut.asserts_count%
  If Not equals_float%(expected!, actual!, delta!) Then
    Local s$ = "Assert equals failed, expected " + Str$(expected!)
    s$ = s$ + " but actually " + Str$(actual!)
    ut.add_failure(s$)
  EndIf
End Sub

Function equals_float%(a!, b!, delta!)
  equals_float% = (a! >= b! - delta!) And (a! <= b! + delta!)
End Function

Sub assert_hex_equals(expected%, actual%, chars%)
  Inc ut.asserts_count%
  If expected% <> actual% Then
    Local s$ = "Assert equals failed, expected &h" + Hex$(expected%, chars%)
    s$ = s$ + " but actually &h" + Hex$(actual%, chars%)
    ut.add_failure(s$)
  EndIf
End Sub

Sub assert_raw_error(expected$)
  Inc ut.asserts_count%
  If InStr(Mm.ErrMsg$, expected$) < 1 Then
    Local s$ = "Expected Error " + Chr$(34) + expected$ + Chr$(34)
    Cat s$, ", but actually " + Chr$(34) + Mm.ErrMsg$ + Chr$(34)
    ut.add_failure(s$)
  EndIf
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
