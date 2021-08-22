Option Explicit On
Option Default None
Option Base 0

Dim ut.asserts_count%
Dim ut.asserts_fails%

test_peek_byte()
test_peek_var()
test_poke_byte()
test_poke_var()

Print "Num successes: " Str$(ut.asserts_count% - ut.asserts_fails%)
Print "Num failures:  " Str$(ut.asserts_fails%)
Print Choice(ut.asserts_fails% = 0, "[OK]", "[FAIL]")

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
