' Copyright (c) 2020-2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

Option Explicit On
Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1") > 0

#Include "../basic-src/splib/system.inc"
#Include "../basic-src/splib/array.inc"
#Include "../basic-src/splib/list.inc"
#Include "../basic-src/splib/string.inc"
#Include "../basic-src/splib/file.inc"
#Include "../basic-src/splib/vt100.inc"
#Include "../basic-src/sptest/unittest.inc"

Const BASE% = Mm.Info(Option Base)

add_test("test_peek_byte")
add_test("test_peek_float")
add_test("test_peek_integer")
add_test("test_peek_short")
add_test("test_peek_var")
add_test("test_peek_word")
add_test("test_peek_cfunaddr")
add_test("test_peek_progmem")
add_test("test_peek_vartbl")
add_test("test_peek_varheader")

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

  ' Having PEEK's first operand in brackets was broken in MMBasic 5.07.01.
  assert_hex_equals(&h01, Peek(Byte (num_addr% + 7)), 2)
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

Sub test_peek_progmem()
  Local offset% = 0

  assert_hex_equals(1, Peek(ProgMem, offset%))

  ' Skip initial T_NEWLINE token (1)
  Inc offset%

  ' Skip comment giving location of file.
  Do While Peek(ProgMem, offset%) <> 1
    Inc offset%
  Loop

  ' Skip T_NEWLINE token (1)
  Inc offset%

  ' Accumulate first actual non-comment line of program in tokenised form.
  Local s$
  Do While Peek(ProgMem, offset%) <> 1)
    Cat s$, Chr$(Peek(ProgMem, offset%))
    Inc offset%
  Loop

  ' Different token ids for OPTION on different platforms.
  If Mm.Device$ = "MMB4L" Then
    assert_string_equals(Chr$(203) + "EXPLICIT ON'|5" + Chr$(0), s$)
  Else
    assert_string_equals(Chr$(197) + "EXPLICIT ON'|5" + Chr$(0), s$)
  EndIf
End Sub

Sub test_peek_vartbl()
  Local easy_to_find% = &h1122AABBCCDDEEFF
  Local var_offset% = find_var_offset%("easy_to_find")
  Local header%(array.new%(8)) ' 64-bytes
  Local header_addr% = Peek(VarAddr header%())
  Local i%
  For i% = 0 To 64
    Poke Byte header_addr% + i%, Peek(VarTbl, var_offset% + i%)
  Next

  check_header(header%())
End Sub

' Finds byte offset into the variable table for a named variable.
Function find_var_offset%(needle$)
  Local offset% = 0
  Local name$
  Local name_addr% = Peek(VarAddr name$)
  Local ch%, done%, i%, j%
  For i% = 0 To 1023
    done% = 0
    For j% = 1 To 32
      ch% = Peek(VarTbl, offset%)
      If ch% = 0 And Not done% Then
        Poke Byte name_addr%, j% - 1
        done% = 1
      Else
        Poke Byte name_addr% + j%, ch%
      EndIf
      Inc offset%
    Next
    ' If Len(name$) > 0 Then
    '   Print "[" Str$(i%) "] " Chr$(34) name$ Chr$(34) ", " Str$(Len(name$))
    ' EndIf
    If name$ = UCase$(needle$) Then
      find_var_offset% = offset% - 32
      Exit For
    EndIf
    Inc offset%, 32
  Next
End Function

Sub check_header(header%())
  Local addr% = Peek(VarAddr header%())

  assert_hex_equals(Asc("E"), Peek(Byte addr% + 0))
  assert_hex_equals(Asc("A"), Peek(Byte addr% + 1))
  assert_hex_equals(Asc("S"), Peek(Byte addr% + 2))
  assert_hex_equals(Asc("Y"), Peek(Byte addr% + 3))
  assert_hex_equals(Asc("_"), Peek(Byte addr% + 4))
  assert_hex_equals(Asc("T"), Peek(Byte addr% + 5))
  assert_hex_equals(Asc("O"), Peek(Byte addr% + 6))
  assert_hex_equals(Asc("_"), Peek(Byte addr% + 7))
  assert_hex_equals(Asc("F"), Peek(Byte addr% + 8))
  assert_hex_equals(Asc("I"), Peek(Byte addr% + 9))
  assert_hex_equals(Asc("N"), Peek(Byte addr% + 10))
  assert_hex_equals(Asc("D"), Peek(Byte addr% + 11))
  assert_hex_equals(0,        Peek(Byte addr% + 12))
  assert_hex_equals(4,        Peek(Byte addr% + 32)) ' type
  assert_hex_equals(3,        Peek(Byte addr% + 33)) ' level
  Local i%
  For i% = 34 To 49 ' 8 x 2 byte array dimensions
    assert_hex_equals(0,      Peek(Byte addr% + i%))
  Next
  Local expected_size% = Choice(Mm.Device$ = "MMB4L", 255, 0)
  assert_hex_equals(expected_size%, Peek(Byte addr% + 50)) ' string size
  For i% = 51 To 55 ' 5 bytes of padding
    assert_hex_equals(0,      Peek(Byte addr% + i%))
  Next
  assert_hex_equals(&hFF, Peek(Byte addr% + 56)) ' value (1st byte)
  assert_hex_equals(&hEE, Peek(Byte addr% + 57)) ' value (2nd byte)
  assert_hex_equals(&hDD, Peek(Byte addr% + 58)) ' value (3rd byte)
  assert_hex_equals(&hCC, Peek(Byte addr% + 59)) ' value (4th byte)
  assert_hex_equals(&hBB, Peek(Byte addr% + 60)) ' value (5th byte)
  assert_hex_equals(&hAA, Peek(Byte addr% + 61)) ' value (6th byte)
  assert_hex_equals(&h22, Peek(Byte addr% + 62)) ' value (7th byte)
  assert_hex_equals(&h11, Peek(Byte addr% + 63)) ' value (8th byte)
End Sub

Sub test_peek_varheader()
  Local easy_to_find% = &h1122AABBCCDDEEFF
  Local src% = Peek(VarHeader easy_to_find%)
  Local header%(array.new%(8)) ' 64-bytes
  Local dst% = Peek(VarAddr header%())

  Memory Copy src%, dst%, 64

  check_header(header%())
End Sub
