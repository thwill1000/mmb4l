' Copyright (c) 2021-2023 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 5.07

Option Explicit On
Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1") > 0

#Include "../sptools/src/splib/system.inc"
#Include "../sptools/src/splib/array.inc"
#Include "../sptools/src/splib/list.inc"
#Include "../sptools/src/splib/string.inc"
#Include "../sptools/src/splib/file.inc"
#Include "../sptools/src/splib/vt100.inc"
#Include "../sptools/src/sptest/unittest.inc"

Const BASE% = Mm.Info(Option Base)

add_test("test_set")
add_test("test_set_byte")
add_test("test_set_short")
add_test("test_set_word")
add_test("test_set_integer")
add_test("test_set_float")
add_test("test_copy")
add_test("test_copy_byte")
add_test("test_copy_short")
add_test("test_copy_word")
add_test("test_copy_integer")
add_test("test_copy_float")
add_test("test_copy_given_overlap")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_set()
  Local buf%(array.new%(32)) ' 256-bytes
  Const addr% = Peek(VarAddr buf%())

  Memory Set addr% + 1, &hCD, 254

  Local i%
  For i% = 0 To 255
    Select Case i%
      Case 0, 255
          assert_hex_equals(&h00, Peek(Byte addr% + i%))
      Case Else
          assert_hex_equals(&hCD, Peek(Byte addr% + i%))
    End Select
  Next
End Sub

Sub test_set_byte()
  Local buf%(array.new%(32)) ' 256-bytes
  Const addr% = Peek(VarAddr buf%())

  Memory Set Byte addr% + 1, &hAB, 254

  Local i%
  For i% = 0 To 255
    Select Case i%
      Case 0, 255
          assert_hex_equals(&h00, Peek(Byte addr% + i%))
      Case Else
          assert_hex_equals(&hAB, Peek(Byte addr% + i%))
    End Select
  Next
End Sub

Sub test_set_short()
  Local buf%(array.new%(32)) ' 256-bytes
  Const addr% = Peek(VarAddr buf%())

  Memory Set Short addr% + 2, &hABCD, 126

  Local i%
  For i% = 0 To 127
    Select Case i%
      Case 0, 127
          assert_hex_equals(&h0000, Peek(Short addr% + 2 * i%))
      Case Else
          assert_hex_equals(&hABCD, Peek(Short addr% + 2 * i%))
    End Select
  Next

  ' Test unaligned address.
  On Error Skip
  Memory Set Short addr% + 3, &hABCD, 1
  assert_raw_error("Address not divisible by 2")

  ' Test maximum value SET.
  Memory Set Short addr% + 2, &hFFFF, 1
  assert_hex_equals(&hFFFF, Peek(Short addr% + 2))
End Sub

Sub test_set_word()
  Local buf%(array.new%(32)) ' 256-bytes
  Const addr% = Peek(VarAddr buf%())

  Memory Set Word addr% + 4, &h12345678, 62

  Local i%
  For i% = 0 To 63
    Select Case i%
      Case 0, 63
          assert_hex_equals(&h00000000, Peek(Word addr% + 4 * i%))
      Case Else
          assert_hex_equals(&h12345678, Peek(Word addr% + 4 * i%))
    End Select
  Next

  ' Test unaligned address.
  On Error Skip
  Memory Set Word addr% + 3, &h12345678, 1
  assert_raw_error("Address not divisible by 4")

  ' Test maximum value SET.
  Memory Set Word addr% + 4, &hFFFFFFFF, 1
  assert_hex_equals(&hFFFFFFFF, Peek(Word addr% + 4))
End Sub

Sub test_set_integer()
  Local buf%(array.new%(32)) ' 256-bytes
  Const addr% = Peek(VarAddr buf%())

  Memory Set Integer addr% + 8, &h1234567890ABCDEF, 30

  Local i%
  For i% = 0 To 31
    Select Case i%
      Case 0, 31
          assert_hex_equals(&h0000000000000000, Peek(Integer addr% + 8 * i%))
      Case Else
          assert_hex_equals(&h1234567890ABCDEF, Peek(Integer addr% + 8 * i%))
    End Select
  Next

  ' Test unaligned address.
  On Error Skip
  Memory Set Integer addr% + 3, &h12345678, 1
  assert_raw_error("Address not divisible by 8")

  ' Test maximum value SET.
  Memory Set Integer addr% + 8, &hFFFFFFFFFFFFFFFF, 1
  assert_hex_equals(&hFFFFFFFFFFFFFFFF, Peek(Integer addr% + 8))
End Sub

Sub test_set_float()
  Local buf%(array.new%(32)) ' 256-bytes
  Const addr% = Peek(VarAddr buf%())

  Memory Set Float addr% + 8, 312.764, 30

  Local i%
  For i% = 0 To 31
    Select Case i%
      Case 0, 31
          assert_float_equals(0.0, Peek(Float addr% + 8 * i%))
      Case Else
          assert_float_equals(312.764, Peek(Float addr% + 8 * i%))
    End Select
  Next

  ' Test unaligned address.
  On Error Skip
  Memory Set Float addr% + 3, 312.764, 1
  assert_raw_error("Address not divisible by 8")

  ' Test "maximum" value SET, actually the value is NaN - TODO: is this true?
  If Not sys.is_device%("pm*") Then
    Local f!
    Poke Integer Peek(VarAddr f!), &hFFFFFFFFFFFFFFFF
    Memory Set Float addr% + 8, f!, 1
    assert_float_equals(f!, Peek(Float addr% + 8))
  EndIf
End Sub

Sub test_copy()
  Local dst%(array.new%(32)), src%(array.new%(32)) ' 256-bytes
  Const dst_addr% = Peek(VarAddr dst%())
  Const src_addr% = Peek(VarAddr src%())
  Local i%
  For i% = Bound(src%(), 0) + 1 To Bound(src%(), 1) - 1
    src%(i%) = &h0102030405060708
  Next

  Memory Copy src_addr%, dst_addr%, 256

  assert_int_array_equals(src%(), dst%())
  assert_hex_equals(0, Peek(Integer dst_addr%))
  assert_hex_equals(0, Peek(Integer dst_addr% + 248))
End Sub

Sub test_copy_byte()
  If Not sys.is_device%("mmb4l") Then Exit Sub

  Local dst%(array.new%(32)), src%(array.new%(32)) ' 256-bytes
  Const dst_addr% = Peek(VarAddr dst%())
  Const src_addr% = Peek(VarAddr src%())
  Local i%
  For i% = Bound(src%(), 0) + 1 To Bound(src%(), 1) - 1
    src%(i%) = &h0102030405060708
  Next

  Memory Copy Byte src_addr%, dst_addr%, 256

  assert_int_array_equals(src%(), dst%())
  assert_hex_equals(0, Peek(Integer dst_addr%))
  assert_hex_equals(0, Peek(Integer dst_addr% + 248))
End Sub

Sub test_copy_short()
  If Not sys.is_device%("mmb4l") Then Exit Sub

  Local dst%(array.new%(32)), src%(array.new%(32)) ' 256-bytes
  Const dst_addr% = Peek(VarAddr dst%())
  Const src_addr% = Peek(VarAddr src%())
  Local i%
  For i% = Bound(src%(), 0) + 1 To Bound(src%(), 1) - 1
    src%(i%) = &h0102030405060708
  Next

  Memory Copy Short src_addr%, dst_addr%, 256 / 2

  assert_int_array_equals(src%(), dst%())
  assert_hex_equals(0, Peek(Integer dst_addr%))
  assert_hex_equals(0, Peek(Integer dst_addr% + 248))

  ' Test unaligned source address.
  On Error Skip
  Memory Copy Short src_addr% + 3, dst_addr%, 1
  assert_raw_error("Source address not divisible by 2")

  ' Test unaligned destination address.
  On Error Skip
  Memory Copy Short src_addr%, dst_addr% + 3, 1
  assert_raw_error("Destination address not divisible by 2")
End Sub

Sub test_copy_word()
  If Not sys.is_device%("mmb4l") Then Exit Sub

  Local dst%(array.new%(32)), src%(array.new%(32)) ' 256-bytes
  Const dst_addr% = Peek(VarAddr dst%())
  Const src_addr% = Peek(VarAddr src%())
  Local i%
  For i% = Bound(src%(), 0) + 1 To Bound(src%(), 1) - 1
    src%(i%) = &h0102030405060708
  Next

  Memory Copy Word src_addr%, dst_addr%, 256 / 4

  assert_int_array_equals(src%(), dst%())
  assert_hex_equals(0, Peek(Integer dst_addr%))
  assert_hex_equals(0, Peek(Integer dst_addr% + 248))

  ' Test unaligned source address.
  On Error Skip
  Memory Copy Word src_addr% + 3, dst_addr%, 1
  assert_raw_error("Source address not divisible by 4")

  ' Test unaligned destination address.
  On Error Skip
  Memory Copy Word src_addr%, dst_addr% + 3, 1
  assert_raw_error("Destination address not divisible by 4")
End Sub

Sub test_copy_integer()
  Local dst%(array.new%(32)), src%(array.new%(32)) ' 256-bytes
  Const dst_addr% = Peek(VarAddr dst%())
  Const src_addr% = Peek(VarAddr src%())
  Local i%
  For i% = Bound(src%(), 0) + 1 To Bound(src%(), 1) - 1
    src%(i%) = &h0102030405060708
  Next

  Memory Copy Integer src_addr%, dst_addr%, 256 / 8

  assert_int_array_equals(src%(), dst%())
  assert_hex_equals(0, Peek(Integer dst_addr%))
  assert_hex_equals(0, Peek(Integer dst_addr% + 248))

  ' Test unaligned source address.
  On Error Skip
  Memory Copy Integer src_addr% + 3, dst_addr%, 1
  If sys.is_device%("mmb4l") Then
    assert_raw_error("Source address not divisible by 8")
  Else
    assert_raw_error("Address not divisible by 8")
  Endif

  ' Test unaligned destination address.
  On Error Skip
  Memory Copy Integer src_addr%, dst_addr% + 3, 1
  If sys.is_device%("mmb4l") Then
    assert_raw_error("Destination address not divisible by 8")
  Else
    assert_raw_error("Address not divisible by 8")
  Endif
End Sub

Sub test_copy_float()
  Local dst!(array.new%(32)), src!(array.new%(32)) ' 256-bytes
  Const dst_addr% = Peek(VarAddr dst!())
  Const src_addr% = Peek(VarAddr src!())
  Local i%
  For i% = Bound(src!(), 0) + 1 To Bound(src!(), 1) - 1
    src!(i%) = 123.456
  Next

  Memory Copy Float src_addr%, dst_addr%, 256 / 8

  Local delta!(array.new%(32))
  assert_float_array_equals(src!(), dst!(), delta!())
  assert_float_equals(0.0, Peek(Float dst_addr%))
  assert_float_equals(0.0, Peek(Float dst_addr% + 248))

  ' Test unaligned source address.
  On Error Skip
  Memory Copy Float src_addr% + 3, dst_addr%, 1
  If sys.is_device%("mmb4l") Then
    assert_raw_error("Source address not divisible by 8")
  Else
    assert_raw_error("Address not divisible by 8")
  Endif

  ' Test unaligned destination address.
  On Error Skip
  Memory Copy Float src_addr%, dst_addr% + 3, 1
  If sys.is_device%("mmb4l") Then
    assert_raw_error("Destination address not divisible by 8")
  Else
    assert_raw_error("Address not divisible by 8")
  Endif
End Sub

Sub test_copy_given_overlap()
  Local buf%(array.new%(4)) ' 32-bytes
  Local pbuf% = Peek(VarAddr buf%())

  buf%(BASE%)     = &h0
  buf%(BASE% + 1) = &hFF01020304050607
  buf%(BASE% + 2) = &h08090A0B0C0D0E0F
  buf%(BASE% + 3) = &h0
  assert_hex_equals(0, Peek(Integer pbuf%))
  assert_hex_equals(&hFF01020304050607, Peek(Integer pbuf% + 8))
  assert_hex_equals(&h08090A0B0C0D0E0F, Peek(Integer pbuf% + 16))
  assert_hex_equals(0, Peek(Integer pbuf% + 24))

  ' Copy onto itself.
  Memory Copy pbuf%, pbuf%, 32

  assert_hex_equals(0, Peek(Integer pbuf%))
  assert_hex_equals(&hFF01020304050607, Peek(Integer pbuf% + 8))
  assert_hex_equals(&h08090A0B0C0D0E0F, Peek(Integer pbuf% + 16))
  assert_hex_equals(0, Peek(Integer pbuf% + 24))

  buf%(BASE%)     = &h0
  buf%(BASE% + 1) = &hFF01020304050607
  buf%(BASE% + 2) = &h08090A0B0C0D0E0F
  buf%(BASE% + 3) = &h0

  ' Copy where pdst < psrc.
  Memory Copy pbuf% + 8, pbuf% + 5, 16

  assert_hex_equals(&h0506070000000000, Peek(Integer pbuf%))
  assert_hex_equals(&h0D0E0FFF01020304, Peek(Integer pbuf% + 8))
  assert_hex_equals(&h08090A08090A0B0C, Peek(Integer pbuf% + 16))
  assert_hex_equals(&h0,                Peek(Integer pbuf% + 24))

  buf%(BASE%)     = &h0
  buf%(BASE% + 1) = &hFF01020304050607
  buf%(BASE% + 2) = &h08090A0B0C0D0E0F
  buf%(BASE% + 3) = &h0

  If sys.is_device%("mmb4l", "pm*") Then
    ' Copy where pdst > psrc.
    ' Colour Maximite 2 and MMB4W (?) don't handle this correctly.
    Memory Copy pbuf% + 8, pbuf% + 11, 16

    assert_hex_equals(&h0,                Peek(Integer pbuf%))
    assert_hex_equals(&h0304050607050607, Peek(Integer pbuf% + 8))
    assert_hex_equals(&h0B0C0D0E0FFF0102, Peek(Integer pbuf% + 16))
    assert_hex_equals(&h000000000008090A, Peek(Integer pbuf% + 24))
  EndIf
End Sub
