' Copyright (c) 2020-2023 Thomas Hugo Williams
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

  num% = 255
  assert_int_equals(255, Peek(Byte num_addr%))
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

  ' This incorrectly returned -1 with a MMB4W 5.07.03b9 build.
  num% = 255
  assert_int_equals(255, Peek(Var num%, 0))
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

  If Mm.Device$ = "MMBasic for Windows" Then Exit Sub

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
  If Mm.Device$ = "MMBasic for Windows" Then Exit Sub

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
  If sys.is_device%("mmb4l") Then
    assert_string_equals(Chr$(204) + "EXPLICIT ON'|5" + Chr$(0), s$)
  ElseIf sys.is_device%("cmm2*") Then
    assert_string_equals(Chr$(197) + "EXPLICIT ON'|5" + Chr$(0), s$)
  ElseIf sys.is_device%("pm*") Then
    ' Assuming we are testing transpiled code.
    assert_string_equals("' Transpiled on ", Left$(s$, 16))
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
  Local ch%, done%, i%, j%
  For i% = 0 To 1023
    done% = 0
    name$ = ""
    For j% = 1 To 32
      ch% = Peek(VarTbl, offset%)
      If ch% = 0 Then
        done% = 1
      Else
        Cat name$, Chr$(ch%)
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
    ' VarTbl entries are 56 bytes on the PicoMite and 64 bytes on the other platforms.
    Inc offset%, Choice(sys.is_device%("pm*"), 24, 32)
  Next
End Function

Sub check_header(header%())
  Local addr% = Peek(VarAddr header%())
  Local i% = -1

  assert_hex_equals(Asc("E"), Peek(Byte addr% + nxt%(i%)))
  assert_hex_equals(Asc("A"), Peek(Byte addr% + nxt%(i%)))
  assert_hex_equals(Asc("S"), Peek(Byte addr% + nxt%(i%)))
  assert_hex_equals(Asc("Y"), Peek(Byte addr% + nxt%(i%)))
  assert_hex_equals(Asc("_"), Peek(Byte addr% + nxt%(i%)))
  assert_hex_equals(Asc("T"), Peek(Byte addr% + nxt%(i%)))
  assert_hex_equals(Asc("O"), Peek(Byte addr% + nxt%(i%)))
  assert_hex_equals(Asc("_"), Peek(Byte addr% + nxt%(i%)))
  assert_hex_equals(Asc("F"), Peek(Byte addr% + nxt%(i%)))
  assert_hex_equals(Asc("I"), Peek(Byte addr% + nxt%(i%)))
  assert_hex_equals(Asc("N"), Peek(Byte addr% + nxt%(i%)))
  assert_hex_equals(Asc("D"), Peek(Byte addr% + nxt%(i%)))
  assert_hex_equals(0,        Peek(Byte addr% + nxt%(i%)))
  i% = 31
  assert_hex_equals(4,        Peek(Byte addr% + nxt%(i%))) ' type
  assert_hex_equals(3,        Peek(Byte addr% + nxt%(i%))) ' level

  If sys.is_device%("pm*") Then
    assert_hex_equals(0, Peek(Byte addr% + nxt%(i%))) ' string size
    assert_hex_equals(0, Peek(Byte addr% + nxt%(i%))) ' name length ?
  EndIf

  ' PicoMite has 6 x 2 byte array dimensions,
  ' other platforms have 8 x 2 byte array dimensions.
  Local limit% = i% + 2 * Choice(sys.is_device%("pm*"), 6, 8)
  Do While i% < limit%
    assert_hex_equals(0, Peek(Byte addr% + nxt%(i%)))
  Loop

  If Not sys.is_device%("pm*") Then
    assert_hex_equals(0, Peek(Byte addr% + nxt%(i%))) ' string size
    assert_hex_equals(0, Peek(Byte addr% + nxt%(i%))) ' 1 byte of padding
    assert_hex_equals(&hAE * sys.is_device%("mmb4l"), Peek(Byte addr% + nxt%(i%))) ' 2 bytes of hash
    assert_hex_equals(&h03 * sys.is_device%("mmb4l"), Peek(Byte addr% + nxt%(i%)))
    assert_hex_equals(0, Peek(Byte addr% + nxt%(i%))) ' 2 bytes of padding
    assert_hex_equals(0, Peek(Byte addr% + nxt%(i%)))
  EndIf

  assert_hex_equals(&hFF, Peek(Byte addr% + nxt%(i%))) ' value (1st byte)
  assert_hex_equals(&hEE, Peek(Byte addr% + nxt%(i%))) ' value (2nd byte)
  assert_hex_equals(&hDD, Peek(Byte addr% + nxt%(i%))) ' value (3rd byte)
  assert_hex_equals(&hCC, Peek(Byte addr% + nxt%(i%))) ' value (4th byte)
  assert_hex_equals(&hBB, Peek(Byte addr% + nxt%(i%))) ' value (5th byte)
  assert_hex_equals(&hAA, Peek(Byte addr% + nxt%(i%))) ' value (6th byte)
  assert_hex_equals(&h22, Peek(Byte addr% + nxt%(i%))) ' value (7th byte)
  assert_hex_equals(&h11, Peek(Byte addr% + nxt%(i%))) ' value (8th byte)
End Sub

Function nxt%(i%)
  Inc i%
  nxt% = i%
End Function

Sub test_peek_varheader()
  Local easy_to_find% = &h1122AABBCCDDEEFF
  Local src% = Peek(VarHeader easy_to_find%)
  Local header%(array.new%(8)) ' 64-bytes
  Local dst% = Peek(VarAddr header%())

  Memory Copy src%, dst%, 64

  check_header(header%())
End Sub
