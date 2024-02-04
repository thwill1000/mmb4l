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
Const MAX_INT% = 9223372036854775807
Const MIN_INT% = -9223372036854775808

add_test("test_bin_function")
add_test("test_chr_ascii")
add_test("test_chr_utf8")
add_test("test_format_function")
add_test("test_hex_function")
add_test("test_mid_function")
add_test("test_mid_command")
add_test("test_oct_function")
add_test("test_str_function")
add_test("test_bin2str_function")
add_test("test_str2bin_function")
add_test("test special chars with OPTION ESCAPE","test_option_escape")
add_test("test special chars in DATA strings","test_option_escape_given_data")
add_test("test special case &00 and 000","test_option_escape_given_null")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub test_bin_function()
  assert_string_equals("0", Bin$(0))
  assert_string_equals("1", Bin$(1))
  assert_string_equals("1111111111111111111111111111111111111111111111111111111111111111", Bin$(-1))
  assert_string_equals("111111111111111111111111111111111111111111111111111111111111111", Bin$(MAX_INT%))
  assert_string_equals("1000000000000000000000000000000000000000000000000000000000000000", Bin$(MIN_INT%))

  ' Value of 0 for second argument should give same result as not providing argument.
  If Not sys.is_platform%("pm*") Then ' TODO: re-enable  
  assert_string_equals("0", Bin$(0, 0))
  assert_string_equals("1", Bin$(1, 0))
  assert_string_equals("1111111111111111111111111111111111111111111111111111111111111111", Bin$(-1, 0))
  assert_string_equals("111111111111111111111111111111111111111111111111111111111111111", Bin$(MAX_INT%, 0))
  assert_string_equals("1000000000000000000000000000000000000000000000000000000000000000", Bin$(MIN_INT%, 0))
  EndIf

  assert_string_equals("0000", Bin$(0, 4))
  assert_string_equals("0001", Bin$(1, 4))
  assert_string_equals("1111111111111111111111111111111111111111111111111111111111111111", Bin$(-1, 4))
  assert_string_equals("111111111111111111111111111111111111111111111111111111111111111", Bin$(MAX_INT%, 4))
  assert_string_equals("1000000000000000000000000000000000000000000000000000000000000000", Bin$(MIN_INT%, 4))
End Sub

Sub test_chr_ascii()
  assert_string_equals("1", Chr$(49))
  assert_string_equals("f", Chr$(102))
  assert_string_equals("Z", Chr$(90))
  assert_string_equals("(", Chr$(40))
  Local s$ = Chr$(255)
  assert_int_equals(1, Peek(Byte Peek(VarAddr s$)))
  assert_int_equals(255, Peek(Byte Peek(VarAddr s$) + 1))
End Sub

Sub test_chr_utf8()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  ' 1-byte unicode = ASCII
  Local s$ = Chr$(Utf8 102)
  assert_int_equals(1, Peek(Byte Peek(VarAddr s$)))
  assert_int_equals(102, Peek(Byte Peek(VarAddr s$) + 1))

  ' 2-byte unicode
  s$ = Chr$(Utf8 &h00A3)
  assert_int_equals(2, Peek(Byte Peek(VarAddr s$)))
  assert_int_equals(&hC2, Peek(Byte Peek(VarAddr s$) + 1))
  assert_int_equals(&hA3, Peek(Byte Peek(VarAddr s$) + 2))

  ' 3-byte unicode
  s$ = Chr$(Utf8 &hFB40)
  assert_int_equals(3, Peek(Byte Peek(VarAddr s$)))
  assert_int_equals(&hEF, Peek(Byte Peek(VarAddr s$) + 1))
  assert_int_equals(&hAD, Peek(Byte Peek(VarAddr s$) + 2))
  assert_int_equals(&h80, Peek(Byte Peek(VarAddr s$) + 3))

  ' 4-byte unicode
  s$ = Chr$(Utf8 &h12013)
  assert_int_equals(4, Peek(Byte Peek(VarAddr s$)))
  assert_int_equals(&hF0, Peek(Byte Peek(VarAddr s$) + 1))
  assert_int_equals(&h92, Peek(Byte Peek(VarAddr s$) + 2))
  assert_int_equals(&h80, Peek(Byte Peek(VarAddr s$) + 3))
  assert_int_equals(&h93, Peek(Byte Peek(VarAddr s$) + 4))
End Sub

Sub test_format_function()
  ' TODO: Actually write some tests.
  assert_string_equals("3.142", Format$(3.142, "%-5g"))
  assert_string_equals("42   ", Format$(42, "%-5g"))
End Sub

Sub test_hex_function()
  assert_string_equals("0", Hex$(0))
  assert_string_equals("1", Hex$(1))
  assert_string_equals("FFFFFFFFFFFFFFFF", Hex$(-1))
  assert_string_equals("7FFFFFFFFFFFFFFF", Hex$(MAX_INT%))
  assert_string_equals("8000000000000000", Hex$(MIN_INT%))

  ' Value of 0 for second argument should give same result as not providing argument.
  If Not sys.is_platform%("pm*") Then ' TODO: re-enable
  assert_string_equals("0", Hex$(0, 0))
  assert_string_equals("1", Hex$(1, 0))
  assert_string_equals("FFFFFFFFFFFFFFFF", Hex$(-1, 0))
  assert_string_equals("7FFFFFFFFFFFFFFF", Hex$(MAX_INT%, 0))
  assert_string_equals("8000000000000000", Hex$(MIN_INT%, 0))
  EndIf

  assert_string_equals("0000", Hex$(0, 4))
  assert_string_equals("0001", Hex$(1, 4))
  assert_string_equals("FFFFFFFFFFFFFFFF", Hex$(-1, 4))
  assert_string_equals("7FFFFFFFFFFFFFFF", Hex$(MAX_INT%, 4))
  assert_string_equals("8000000000000000", Hex$(MIN_INT%, 4))
End Sub

Sub test_mid_function()
  assert_string_equals("f", Mid$("foobar", 1, 1))
  assert_string_equals("oo", Mid$("foobar", 2, 2))
  assert_string_equals("bar", Mid$("foobar", 4, 10))
End Sub

Sub test_mid_command()
  Local a$ = "123456"

  Mid$(a$, 3, 3) = "aaa"
  assert_string_equals("12aaa6", a$)

  Mid$(a$, 1, 1) = "b"
  assert_string_equals("b2aaa6", a$)

  On Error Skip
  Mid$(a$, 1, 10) = "cccccccccc"
  Local expected$ = "10 is invalid (valid is 1 to 6)"
  assert_string_equals(expected$, Right$(Mm.ErrMsg$, Len(expected$)))
End Sub

Sub test_oct_function()
  assert_string_equals("0", Oct$(0))
  assert_string_equals("1", Oct$(1))
  assert_string_equals("1777777777777777777777", Oct$(-1))
  assert_string_equals("777777777777777777777", Oct$(MAX_INT%))
  assert_string_equals("1000000000000000000000", Oct$(MIN_INT%))

  ' Value of 0 for second argument should give same result as not providing argument.
  If Not sys.is_platform%("pm*") Then ' TODO: re-enable
  assert_string_equals("0", Oct$(0, 0))
  assert_string_equals("1", Oct$(1, 0))
  assert_string_equals("1777777777777777777777", Oct$(-1, 0))
  assert_string_equals("777777777777777777777", Oct$(MAX_INT%, 0))
  assert_string_equals("1000000000000000000000", Oct$(MIN_INT%, 0))
  EndIf

  assert_string_equals("0000", Oct$(0, 4))
  assert_string_equals("0001", Oct$(1, 4))
  assert_string_equals("1777777777777777777777", Oct$(-1, 4))
  assert_string_equals("777777777777777777777", Oct$(MAX_INT%, 4))
  assert_string_equals("1000000000000000000000", Oct$(MIN_INT%, 4))
End Sub

Sub test_str_function()
  assert_string_equals("0", Str$(0))
  assert_string_equals("1", Str$(1))
  assert_string_equals("-1", Str$(-1))
  assert_string_equals("9223372036854775807", Str$(MAX_INT%))
  assert_string_equals("-9223372036854775808", Str$(MIN_INT%))

  ' Value of 0 for second argument should give same result as not providing argument.
  assert_string_equals("0", Str$(0, 0))
  assert_string_equals("1", Str$(1, 0))
  assert_string_equals("-1", Str$(-1, 0))
  assert_string_equals("9223372036854775807", Str$(MAX_INT%, 0))
  assert_string_equals("-9223372036854775808", Str$(MIN_INT%, 0))

  assert_string_equals("   0", Str$(0, 4))
  assert_string_equals("   1", Str$(1, 4))
  assert_string_equals("  -1", Str$(-1, 4))
  assert_string_equals("9223372036854775807", Str$(MAX_INT%, 4))
  assert_string_equals("-9223372036854775808", Str$(MIN_INT%, 4))
End Sub

Sub test_bin2str_function()
  Local expected$, fn$, val$, type$

  ' Little endian.
  Restore bin2str_data
  Do
    Read val$, type$, expected$
    If val$ = "" Then Exit Do
    expected$ = str.decode$(expected$)
    fn$ = "Bin2Str$(" + type$ + ", " + val$ + ")"
    assert_string_equals(expected$, Eval(fn$))
  Loop

  ' Big endian.
  Restore bin2str_data
  Do
    Read val$, type$, expected$
    If val$ = "" Then Exit Do
    expected$ = str.reverse$(str.decode$(expected$))
    fn$ = "Bin2Str$(" + type$ + ", " + val$ + ", Big)"
    assert_string_equals(expected$, Eval(fn$))
  Loop
End Sub

bin2str_data:
Data "32",   "Int64",  "\x20\x00\x00\x00\x00\x00\x00\x00"
Data "32",   "UInt64", "\x20\x00\x00\x00\x00\x00\x00\x00"
Data "32",   "Int32",  "\x20\x00\x00\x00"
Data "32",   "UInt32", "\x20\x00\x00\x00"
Data "32",   "Int16",  "\x20\x00"
Data "32",   "UInt16", "\x20\x00"
Data "32",   "Int8",   "\x20"
Data "32",   "UInt8",  "\x20"
Data "32.0", "Single", "\x00\x00\x00\x42"
Data "32.0", "Double", "\x00\x00\x00\x00\x00\x00\x40\x40"
Data "", "", ""

Sub test_str2bin_function()
  Local expected$, fn$, val$, type$

  ' Little endian.
  Restore bin2str_data
  Do
    Read expected$, type$, val$
    If expected$ = "" Then Exit Do
    val$ = str.decode$(val$)
    fn$ = "Str2Bin(" + type$ + ", val$)"
    If InStr("Single|Double", type$) Then
      assert_float_equals(Val(expected$), Eval(fn$))
    Else
      assert_int_equals(Val(expected$), Eval(fn$))
    EndIf
  Loop

  ' Big endian.
  Restore bin2str_data
  Do
    Read expected$, type$, val$
    If expected$ = "" Then Exit Do
    val$ = str.reverse$(str.decode$(val$))
    fn$ = "Str2Bin(" + type$ + ", val$, Big)"
    If InStr("Single|Double", type$) Then
      assert_float_equals(Val(expected$), Eval(fn$))
    Else
      assert_int_equals(Val(expected$), Eval(fn$))
    EndIf
  Loop
End Sub

Sub test_option_escape()
  If Not sys.is_platform%("pm*") Then Exit Sub

  Option Escape
  Const QU$ = Chr$(34)
  Const NL$ = CHR$(10)
  assert_string_equals(str.quote$("hello"), "\qhello\q")
  assert_string_equals(str.quote$("hello world"), "\qhello world\q")
  assert_string_equals(str.quote$(str.quote$("hello world")), "\q\qhello world\q\q")
  assert_string_equals(QU$ + "good" + NL$ + "bye", "\qgood\nbye")
  assert_string_equals(Chr$(13), "\r")
  assert_string_equals(Chr$(34), "\q")
  assert_string_equals(Chr$(10), "\n")
  assert_string_equals(Chr$(7), "\a")
  assert_string_equals(Chr$(8), "\b")
  assert_string_equals(Chr$(&h1b), "\e")
  assert_string_equals(Chr$(&h0c), "\f")
  assert_string_equals(Chr$(&h09), "\t")
  assert_string_equals(Chr$(&h0b), "\v")
  assert_string_equals(Chr$(&h5c), "\\")
  assert_string_equals(Chr$(13), "\013")
  assert_string_equals(Chr$(13), "\&0d")
End Sub

Sub test_option_escape_given_data()
  If Not sys.is_platform%("pm*") Then Exit Sub

  Option Escape
  Restore string_data
  Const QU$ = Chr$(34)
  Local s$
  Read s$
  assert_string_equals(str.quote$("hello world"), s$)
  Read s$
  assert_string_equals("good" + QU$ + "bye", s$)
End Sub

string_data:
Data "\qhello world\q","good\qbye"

Sub test_option_escape_given_null()
  If Not sys.is_platform%("pm*") Then Exit Sub

  Option Escape
  Const QU$ = Chr$(34)
  Const NL$ = Chr$(10)
  Const CHRZERO$ = Chr$(0)
  Local s$

  assert_string_equals("*" + Chr$(255) + "*", "*\&FF*")
  assert_string_equals("*" + Chr$(255) + "*", "*\255*")
  assert_string_equals("*" + Chr$(1) + "*", "*\&01*")
  assert_string_equals("*" + Chr$(1) + "*", "*\001*")

  On Error Skip 1
  s$ = "*\&00*"
  assert_raw_error("Null character \\&00 in escape sequence - use CHR$(0)")

  On Error Skip 1
  s$ = "*\000*"
  assert_raw_error("Null character \\000 in escape sequence - use CHR$(0)")

  Restore string_data_null_1
  On Error Skip 1
  Read s$
  assert_raw_error("Null character \\&00 in escape sequence - use CHR$(0)")

  Restore string_data_null_2
  On Error Skip 1
  Read s$
  assert_raw_error("Null character \\000 in escape sequence - use CHR$(0)")
End Sub

string_data_null_1:
Data "*\&00*"

string_data_null_2:
Data "*\000*"
