Option Explicit On
Option Default None
Option Base 0

Const BASE% = Mm.Info(Option Base)

Sub run_tests()
  test_peek_byte()
  test_peek_float()
  test_peek_integer()
  test_peek_short()
  test_peek_var()
  test_peek_word()
End Sub

Sub test_peek_byte()
  Local num% = &h0102030405060708
  Local num_addr% = Peek(VarAddr num%)

  assert_hex_equals(&h08, Peek(Byte num_addr% + 0), 2)
  assert_hex_equals(&h07, Peek(Byte num_addr% + 1), 2)
  assert_hex_equals(&h06, Peek(Byte num_addr% + 2), 2)
  assert_hex_equals(&h05, Peek(Byte num_addr% + 3), 2)
  assert_hex_equals(&h04, Peek(Byte num_addr% + 4), 2)
  assert_hex_equals(&h03, Peek(Byte num_addr% + 5), 2)
  assert_hex_equals(&h02, Peek(Byte num_addr% + 6), 2)
  assert_hex_equals(&h01, Peek(Byte num_addr% + 7), 2)
End Sub

Sub test_peek_float()
  Local arr!(BASE% + 1) = (3.142, -3.142e23)
  Local arr_addr% = Peek(VarAddr arr!())

  ' Peek(Float addr%) rounds addess down to 64-bit boundary.

  assert_float_equals(3.142, Peek(Float arr_addr% + 0), 16)
  assert_float_equals(3.142, Peek(Float arr_addr% + 1), 16)
  assert_float_equals(3.142, Peek(Float arr_addr% + 2), 16)
  assert_float_equals(3.142, Peek(Float arr_addr% + 3), 16)
  assert_float_equals(3.142, Peek(Float arr_addr% + 4), 16)
  assert_float_equals(3.142, Peek(Float arr_addr% + 5), 16)
  assert_float_equals(3.142, Peek(Float arr_addr% + 6), 16)
  assert_float_equals(3.142, Peek(Float arr_addr% + 7), 16)

  assert_float_equals(-3.142e23, Peek(Float arr_addr% + 8), 16)
  assert_float_equals(-3.142e23, Peek(Float arr_addr% + 9), 16)
  assert_float_equals(-3.142e23, Peek(Float arr_addr% + 10), 16)
  assert_float_equals(-3.142e23, Peek(Float arr_addr% + 11), 16)
  assert_float_equals(-3.142e23, Peek(Float arr_addr% + 12), 16)
  assert_float_equals(-3.142e23, Peek(Float arr_addr% + 13), 16)
  assert_float_equals(-3.142e23, Peek(Float arr_addr% + 14), 16)
  assert_float_equals(-3.142e23, Peek(Float arr_addr% + 15), 16)
End Sub

Sub test_peek_integer()
  Local arr%(BASE% + 1) = (&h0102030405060708, &h090a0b0c0d0e0f10)
  Local arr_addr% = Peek(VarAddr arr%())

  ' Peek(Integer addr%) rounds address down to 64-bit boundary.

  assert_hex_equals(&h0102030405060708, Peek(Integer arr_addr% + 0), 16)
  assert_hex_equals(&h0102030405060708, Peek(Integer arr_addr% + 1), 16)
  assert_hex_equals(&h0102030405060708, Peek(Integer arr_addr% + 2), 16)
  assert_hex_equals(&h0102030405060708, Peek(Integer arr_addr% + 3), 16)
  assert_hex_equals(&h0102030405060708, Peek(Integer arr_addr% + 4), 16)
  assert_hex_equals(&h0102030405060708, Peek(Integer arr_addr% + 5), 16)
  assert_hex_equals(&h0102030405060708, Peek(Integer arr_addr% + 6), 16)
  assert_hex_equals(&h0102030405060708, Peek(Integer arr_addr% + 7), 16)

  assert_hex_equals(&h090a0b0c0d0e0f10, Peek(Integer arr_addr% + 8), 16)
  assert_hex_equals(&h090a0b0c0d0e0f10, Peek(Integer arr_addr% + 9), 16)
  assert_hex_equals(&h090a0b0c0d0e0f10, Peek(Integer arr_addr% + 10), 16)
  assert_hex_equals(&h090a0b0c0d0e0f10, Peek(Integer arr_addr% + 11), 16)
  assert_hex_equals(&h090a0b0c0d0e0f10, Peek(Integer arr_addr% + 12), 16)
  assert_hex_equals(&h090a0b0c0d0e0f10, Peek(Integer arr_addr% + 13), 16)
  assert_hex_equals(&h090a0b0c0d0e0f10, Peek(Integer arr_addr% + 14), 16)
  assert_hex_equals(&h090a0b0c0d0e0f10, Peek(Integer arr_addr% + 15), 16)
End Sub

Sub test_peek_short()
  Local num% = &h0102030405060708
  Local num_addr% = Peek(VarAddr num%)

  ' Peek(Short addr%) rounds address down to 16-bit boundary.

  assert_hex_equals(&h0708, Peek(Short num_addr% + 0), 4)
  assert_hex_equals(&h0708, Peek(Short num_addr% + 1), 4)
  assert_hex_equals(&h0506, Peek(Short num_addr% + 2), 4)
  assert_hex_equals(&h0506, Peek(Short num_addr% + 3), 4)
  assert_hex_equals(&h0304, Peek(Short num_addr% + 4), 4)
  assert_hex_equals(&h0304, Peek(Short num_addr% + 5), 4)
  assert_hex_equals(&h0102, Peek(Short num_addr% + 6), 4)
  assert_hex_equals(&h0102, Peek(Short num_addr% + 7), 4)
End Sub

Sub test_peek_var()
  Local num% = &h0102030405060708

  'assert_hex_equals(&h08, Peek(Var num%), 2) ' TODO: is this not supported on CMM2 ?
  assert_hex_equals(&h08, Peek(Var num%, 0), 2)
  assert_hex_equals(&h07, Peek(Var num%, 1), 2)
  assert_hex_equals(&h06, Peek(Var num%, 2), 2)
  assert_hex_equals(&h05, Peek(Var num%, 3), 2)
  assert_hex_equals(&h04, Peek(Var num%, 4), 2)
  assert_hex_equals(&h03, Peek(Var num%, 5), 2)
  assert_hex_equals(&h02, Peek(Var num%, 6), 2)
  assert_hex_equals(&h01, Peek(Var num%, 7), 2)
End Sub

Sub test_peek_word()
  Local num% = &h0102030405060708
  Local num_addr% = Peek(VarAddr num%)

  ' Peek(Word addr%) rounds address down to 32-bit boundary.

  assert_hex_equals(&h05060708, Peek(Word num_addr% + 0), 8)
  assert_hex_equals(&h05060708, Peek(Word num_addr% + 1), 8)
  assert_hex_equals(&h05060708, Peek(Word num_addr% + 2), 8)
  assert_hex_equals(&h05060708, Peek(Word num_addr% + 3), 8)

  assert_hex_equals(&h01020304, Peek(Word num_addr% + 4), 8)
  assert_hex_equals(&h01020304, Peek(Word num_addr% + 5), 8)
  assert_hex_equals(&h01020304, Peek(Word num_addr% + 6), 8)
  assert_hex_equals(&h01020304, Peek(Word num_addr% + 7), 8)
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

Sub ut.add_failure(msg$)
  Inc ut.asserts_fails%
  Print "FAILURE [" Str$(ut.asserts_count%) "] " msg$
End Sub

Sub ut.report()
  Print "Num successes: " Str$(ut.asserts_count% - ut.asserts_fails%)
  Print "Num failures:  " Str$(ut.asserts_fails%)
  Print Choice(ut.asserts_fails% = 0, "[OK]", "[FAIL]")
End Sub
