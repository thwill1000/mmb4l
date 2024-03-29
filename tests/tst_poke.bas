' Copyright (c) 2021-2024 Thomas Hugo Williams
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

add_test("test_poke_byte")
add_test("test_poke_float")
add_test("test_poke_integer")
add_test("test_poke_short")
add_test("test_poke_var")
add_test("test_poke_word")
add_test("test_poke_vartbl")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

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

Sub test_poke_vartbl()
  Local easy_to_find% = 0
  Local offset% = find_var_offset%("easy_to_find")
  ' PicoMite has different VarTbl layout.
  Inc offset%, Choice(sys.is_platform%("pm*"), 47, 55)

  Poke VarTbl, nxt%(offset%), &h01
  Poke VarTbl, nxt%(offset%), &h23
  Poke VarTbl, nxt%(offset%), &h45
  Poke VarTbl, nxt%(offset%), &h67
  Poke VarTbl, nxt%(offset%), &h89
  Poke VarTbl, nxt%(offset%), &hAB
  Poke VarTbl, nxt%(offset%), &hCD
  Poke VarTbl, nxt%(offset%), &hEF

  assert_hex_equals(&hEFCDAB8967452301, easy_to_find%)
End Sub

Function nxt%(i%)
  Inc i%
  nxt% = i%
End Function

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
    Inc offset%, Choice(sys.is_platform%("pm*"), 24, 32)
  Next
End Function
