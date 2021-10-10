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

add_test("test_set")
add_test("test_set_byte")
add_test("test_set_short")
' add_test("test_set_word")
' add_test("test_set_integer")
' add_test("test_set_float")
add_test("test_copy")
' add_test("test_copy_byte")
' add_test("test_copy_short")
' add_test("test_copy_word")
' add_test("test_copy_integer")
' add_test("test_copy_float")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_set()
  Local buf%(32) ' 256-bytes
  Local addr% = Peek(VarAddr buf%())

  Memory Set addr% + 1, &hCD, 254

  assert_hex_equals(&h00, Peek(Byte addr%))
  Local i%
  For i% = 1 To 254
    assert_hex_equals(&hCD, Peek(Byte addr% + i%))
  Next
  assert_hex_equals(&h00, Peek(Byte addr% + 255))
End Sub

Sub test_set_byte()
  Local buf%(32) ' 256-bytes
  Local addr% = Peek(VarAddr buf%())

  Memory Set Byte addr% + 1, &hAB, 254

  assert_hex_equals(&h00, Peek(Byte addr%))
  Local i%
  For i% = 1 To 254
    assert_hex_equals(&hAB, Peek(Byte addr% + i%))
  Next
  assert_hex_equals(&h00, Peek(Byte addr% + 255))
End Sub

Sub test_set_short()
  Local buf%(32) ' 256-bytes
  Local addr% = Peek(VarAddr buf%())

  Memory Set Short addr% + 2, &hABCD, 126

  assert_hex_equals(&h0000, Peek(Short addr%))
  Local i%
  For i% = 2 To 252 Step 2
    assert_hex_equals(&hABCD, Peek(Short addr% + i%))
  Next
  assert_hex_equals(&h0000, Peek(Short addr% + 254))
End Sub

Sub test_copy()
  Local dst%(array.new%(32)), src%(array.new%(32)) ' 256-bytes
  Local dst_addr% = Peek(VarAddr dst%())
  Local src_addr% = Peek(VarAddr src%())
  Local i%
  For i% = Bound(src%(), 0) To Bound(src%(), 1)
    src%(i%) = &h0102030405060708
  Next

  Memory Copy src_addr%, dst_addr%, 256

  assert_int_array_equals(src%(), dst%())
End Sub
