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

add_test("test_append")
add_test("test_clear")
add_test("test_copy")
add_test("test_concat")
add_test("test_lcase")
add_test("test_lcompare")
add_test("test_left")
add_test("test_lgetbyte")
add_test("test_lgetstr")
add_test("test_linstr")
add_test("test_llen")
add_test("test_load")
add_test("test_mid")
add_test("test_print")
add_test("test_replace")
add_test("test_resize")
add_test("test_right")
add_test("test_setbyte")
add_test("test_trim")
add_test("test_ucase")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_append()
  Local array%(1000)
  Local addr% = Peek(VarAddr array%())

  LongString Append array%(), "Hello World"

  assert_hex_equals(11, array%(BASE% + 0))
  assert_hex_equals(Asc("H"), Peek(Byte addr% + 8))
  assert_hex_equals(Asc("e"), Peek(Byte addr% + 9))
  assert_hex_equals(Asc("l"), Peek(Byte addr% + 10))
  assert_hex_equals(Asc("l"), Peek(Byte addr% + 11))
  assert_hex_equals(Asc("o"), Peek(Byte addr% + 12))
  assert_hex_equals(Asc(" "), Peek(Byte addr% + 13))
  assert_hex_equals(Asc("W"), Peek(Byte addr% + 14))
  assert_hex_equals(Asc("o"), Peek(Byte addr% + 15))
  assert_hex_equals(Asc("r"), Peek(Byte addr% + 16))
  assert_hex_equals(Asc("l"), Peek(Byte addr% + 17))
  assert_hex_equals(Asc("d"), Peek(Byte addr% + 18))
  assert_hex_equals(&h00,     Peek(Byte addr% + 19))
End Sub

Sub test_clear()
  Local array%(100)
  LongString Append array%(), "Hello World"

  LongString Clear array%()

  assert_hex_equals(&h0, array%(BASE% + 0));
End Sub

Sub test_copy()
  Local src%(100), dest%(100)
  LongString Append src%(), "Hello World"

  LongString Copy dest%(), src%()

  assert_string_equals("Hello World", LGetStr$(dest%(), 1, LLen(dest%())))
End Sub

Sub test_concat()
  Local src%(100), dest%(100)
  LongString Append src%(), "Hello World"

  LongString Concat dest%(), src%()
  LongString Concat dest%(), src%()

  assert_string_equals("Hello WorldHello World", LGetStr$(dest%(), 1, LLen(dest%())))
End Sub

Sub test_lcase()
  Local array%(100)
  LongString Append array%(), "Hello World"

  LongString LCase array%()

  assert_string_equals("hello world", LGetStr$(array%(), 1, LLen(array%())))
End Sub

Sub test_lcompare()
  Local array1%(100), array2%(100)
  LongString Append array1%(), "Hello World"

  LongString Append array2%(), "Hello World"
  assert_int_equals(0, LCompare(array1%(), array2%()))

  LongString Clear array2%()
  LongString Append array2%(), "Hello"
  assert_int_equals(1, LCompare(array1%(), array2%()))

  LongString Clear array2%()
  LongString Append array2%(), "Hello World, Goodbye Mars"
  assert_int_equals(-1, LCompare(array1%(), array2%()))
End Sub

Sub test_left()
  Local dest%(100), src%(100)
  LongString Append src%(), "Hello World"

  LongString Left dest%(), src%(), 5

  assert_int_equals(5, LLen(dest%()))
  assert_string_equals("Hello", LGetStr$(dest%(), 1, LLen(dest%())))
End Sub

Sub test_lgetbyte()
  Local array%(100)
  LongString Append array%(), "Hello World"

  assert_hex_equals(Asc("H"), LGetByte(array%(), BASE% + 0))
  assert_hex_equals(Asc("e"), LGetByte(array%(), BASE% + 1))
  assert_hex_equals(Asc("l"), LGetByte(array%(), BASE% + 2))
  assert_hex_equals(Asc("l"), LGetByte(array%(), BASE% + 3))
  assert_hex_equals(Asc("o"), LGetByte(array%(), BASE% + 4))
  assert_hex_equals(Asc(" "), LGetByte(array%(), BASE% + 5))
  assert_hex_equals(Asc("W"), LGetByte(array%(), BASE% + 6))
  assert_hex_equals(Asc("o"), LGetByte(array%(), BASE% + 7))
  assert_hex_equals(Asc("r"), LGetByte(array%(), BASE% + 8))
  assert_hex_equals(Asc("l"), LGetByte(array%(), BASE% + 9))
  assert_hex_equals(Asc("d"), LGetByte(array%(), BASE% + 10))
  assert_hex_equals(&h00,     LGetByte(array%(), BASE% + 11))
End Sub

Sub test_lgetstr()
  Local array%(100)
  LongString Append array%(), "Hello World"

  assert_string_equals("Hello", LGetStr$(array%(), 1, 5))
  assert_string_equals(" World", LGetStr$(array%(), 6, 6))
End Sub

Sub test_linstr()
  Local array%(100)
  LongString Append array%(), "Hello World"

  assert_int_equals(1, LInStr(array%(), "Hell"))
  assert_int_equals(8, LInStr(array%(), "o", 6))
  assert_int_equals(0, LInStr(array%(), "z"))
End Sub

Sub test_llen()
  Local array%(100)
  LongString Append array%(), "Hello World"

  assert_int_equals(11, LLen(array%()))
End Sub

Sub test_load()
  Local dest%(100)
  Local src$ = "Hello World"

  LongString Load dest%(), 5, src$

  assert_string_equals("Hello", LGetStr$(dest%(), 1, LLen(dest%())))
End Sub

Sub test_mid()
  Local dest%(100), src%(100)
  LongString Append src%(), "Hello World"

  LongString Mid dest%(), src%(), 3, 5

  assert_string_equals("llo W", LGetStr$(dest%(), 1, LLen(dest%())))
End Sub

Sub test_print()
  Local array%(100)
  LongString Append array%(), "Hello World"

  Open "/tmp/tst_longstring-test_print.txt" For Output As #1
  LongString Print #1, array%()
  Close #1

  Open "/tmp/tst_longstring-test_print.txt" For Input As #1
  Local s$
  Line Input #1, s$
  assert_string_equals("Hello World", s$)
  assert_true(Eof(#1))
  Close #1
End Sub

Sub test_replace()
  Local array%(100)
  LongString Append array%(), "Hello World"

  LongString Replace array%(), "Mars", 7

  assert_string_equals("Hello Marsd", LGetStr$(array%(), 1, LLen(array%())))
End Sub

Sub test_resize()
  Local array%(100 + BASE) ' 8 bytes for the size and 800 bytes for the data
  LongString Append array%(), "Hello World"

  LongString Resize array%(), 4
  assert_string_equals("Hell", LGetStr$(array%(), 1, LLen(array%())))

  LongString Resize array%(), 0
  assert_int_equals(0, LLen(array%()))

  LongString Resize array%(), 800
  assert_int_equals(800, LLen(array%()))

  On Error Skip 1
  LongString Resize array%(), 801
  assert_raw_error("801 is invalid (valid is 0 to 800)")
End Sub

Sub test_right()
  Local dest%(100), src%(100)
  LongString Append src%(), "Hello World"

  LongString Right dest%(), src%(), 5

  assert_string_equals("World", LGetStr$(dest%(), 1, LLen(dest%())))
End Sub

Sub test_setbyte()
  Local array%(100)
  LongString Append array%(), "Hello World"

  LongString SetByte array%(), BASE% + 1, Asc("X")
  LongString SetByte array%(), BASE% + 3, Asc("Y")
  LongString SetByte array%(), BASE% + 5, Asc("Z")

  assert_string_equals("HXlYoZWorld", LGetStr$(array%(), 1, LLen(array%())))
End Sub

Sub test_trim()
  Local array%(100)
  LongString Append array%(), "Hello World"

  LongString Trim array%(), 3

  assert_string_equals("lo World", LGetStr$(array%(), 1, LLen(array%())))
End Sub

Sub test_ucase()
  Local array%(100)
  LongString Append array%(), "Hello World"

  LongString UCase array%()

  assert_string_equals("HELLO WORLD", LGetStr$(array%(), 1, LLen(array%())))
End Sub
