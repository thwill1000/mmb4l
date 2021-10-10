' Copyright (c) 2020-2021 Thomas Hugo Williams
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

add_test("test_peek_byte")
add_test("test_peek_float")
add_test("test_peek_integer")
add_test("test_peek_short")
add_test("test_peek_var")
add_test("test_peek_word")
add_test("test_peek_cfunaddr")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
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

Sub test_peek_cfunaddr()
  Local ad%, i%, offset%

  ad% = Peek(CFunAddr data1())
  offset% = 0
  For i% = 1 To &hC
    assert_hex_equals(i%, Peek(Word ad% + 4 * offset%))
    Inc offset%
  Next

  ad% = Peek(CFunAddr data2())
  offset% = 0
  For i% = &hD To &hA Step -1
    assert_hex_equals(i%, Peek(Word ad% + 4 * offset%))
    Inc offset%
  Next
End Sub

CSub data1()
  00000000
  00000001 00000002 00000003 00000004
  00000005 00000006 00000007 00000008
  00000009 0000000A 0000000B 0000000C
End CSub

CSub data2()
  00000002
  0000000F 0000000E 0000000D 0000000C
  0000000B 0000000A
End CSub
