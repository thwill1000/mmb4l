' Copyright (c) 2022 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 5.07

Option Explicit On
Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1")  > 0

#Include "../sptools/src/splib/system.inc"
#Include "../sptools/src/splib/array.inc"
#Include "../sptools/src/splib/list.inc"
#Include "../sptools/src/splib/string.inc"
#Include "../sptools/src/splib/file.inc"
#Include "../sptools/src/splib/vt100.inc"
#Include "../sptools/src/sptest/unittest.inc"

Const BASE% = Mm.Info(Option Base)

If Mm.Device$ <> "MMBasic for Windows" Then
  add_test("test_csub_as_data")
EndIf

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub test_csub_as_data()
  Local addr% = Peek(CFunAddr my_data)

  assert_hex_equals(&h04, Peek(Byte addr%))
  assert_hex_equals(&h03, Peek(Byte addr% + 1))
  assert_hex_equals(&h02, Peek(Byte addr% + 2))
  assert_hex_equals(&h01, Peek(Byte addr% + 3))

  assert_hex_equals(&h0708, Peek(Short addr% + 4))
  assert_hex_equals(&h0506, Peek(Short addr% + 6))

  assert_hex_equals(&h090A0B0C, Peek(Word addr% + 8))
  assert_hex_equals(&h0D0E0F10, Peek(Word addr% + 12))
  assert_hex_equals(&h11121314, Peek(Word addr% + 16))
  assert_hex_equals(&h15161718, Peek(Word addr% + 20))

  ' Note that PEEK(INTEGER address) requires address to be on an 8-byte
  ' boundary, otherwise it rounds address down to the nearest 8-byte boundary.
  assert_hex_equals(&h1516171811121314, Peek(Integer addr% + 16))
End Sub

CSub my_data()
  00000000
  01020304 05060708  090A0B0C  0D0E0F10  11121314  15161718
End CSub
