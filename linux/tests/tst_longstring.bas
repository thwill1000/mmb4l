Option Explicit On
Option Default None
Option Base 0

Sub run_tests()
  test_append()
  test_lgetbyte()
  test_lgetstr()
  test_llen()
End Sub

Sub test_append()
  Local ls%(1000)
  Local addr% = Peek(VarAddr ls%())

  LongString Append ls%(), "foo"

  assert_hex_equals(&h03, ls%(0))
  assert_hex_equals(Asc("f"), Peek(Byte addr% + 8))
  assert_hex_equals(Asc("o"), Peek(Byte addr% + 9))
  assert_hex_equals(Asc("o"), Peek(Byte addr% + 10))
  assert_hex_equals(&h00,     Peek(Byte addr% + 11))
End Sub

Sub test_lgetbyte()
  Local ls%(1000)
  LongString Append ls%(), "Hello World"

  assert_hex_equals(Asc("H"), LGetByte(ls%(), 0))
  assert_hex_equals(Asc("e"), LGetByte(ls%(), 1))
  assert_hex_equals(Asc("l"), LGetByte(ls%(), 2))
  assert_hex_equals(Asc("l"), LGetByte(ls%(), 3))
  assert_hex_equals(Asc("o"), LGetByte(ls%(), 4))
  assert_hex_equals(Asc(" "), LGetByte(ls%(), 5))
  assert_hex_equals(Asc("W"), LGetByte(ls%(), 6))
  assert_hex_equals(Asc("o"), LGetByte(ls%(), 7))
  assert_hex_equals(Asc("r"), LGetByte(ls%(), 8))
  assert_hex_equals(Asc("l"), LGetByte(ls%(), 9))
  assert_hex_equals(Asc("d"), LGetByte(ls%(), 10))
  assert_hex_equals(&h00,     LGetByte(ls%(), 11))
End Sub

Sub test_lgetstr()
  Local ls%(1000)
  LongString Append ls%(), "Hello World"

  assert_string_equals("Hello", LGetStr$(ls%(), 1, 5))
  assert_string_equals(" World", LGetStr$(ls%(), 6, 6))
End Sub

Sub test_llen()
  Local ls%(1000)
  LongString Append ls%(), "Hello World"

  assert_int_equals(11, LLen(ls%()))
End Sub

ut.init()
run_tests()
ut.report()

Sub ut.init()
  Dim ut.asserts_count%
  Dim ut.asserts_fails%
End Sub

Sub assert_hex_equals(expected%, actual%, chars%)
  Inc ut.asserts_count%
  If expected% <> actual% Then
    Local s$ = "Assert equals failed, expected &h" + Hex$(expected%, chars%)
    s$ = s$ + " but actually &h" + Hex$(actual%, chars%)
    ut.add_failure(s$)
  EndIf
End Sub

Sub assert_int_equals(expected%, actual%)
  Inc ut.asserts_count%
  If expected% <> actual% Then
    Local s$ = "Assert equals failed, expected " + Str$(expected%)
    s$ = s$ + " but actually " + Str$(actual%)
    ut.add_failure(s$)
  EndIf
End Sub

Sub assert_string_equals(expected_$, actual_$)
  Inc ut.asserts_count%
  If expected_$ <> actual_$ Then
    Local expected$ = Chr$(34) + expected_$ + Chr$(34)
    Local actual$ = Chr$(34) + actual_$ + Chr$(34)
    If Len(expected_$) = 1 Then expected$ = "Chr$(" + Str$(Asc(expected_$)) + ")"
    If Len(actual_$) = 1 Then actual$ = "Chr$(" + Str$(Asc(actual_$)) + ")"

    Local s$ = "Assert equals failed, expected " + ut.sanitise_string$(expected$)
    Inc s$, " but actually " + ut.sanitise_string$(actual$)
    ut.add_failure(s$)
  EndIf
End Sub

Function ut.sanitise_string$(s$)
  Local c%, i%, s2$
  For i% = 1 To Len(s$)
    c% = Peek(Var s$, i%)
    Inc s2$, Choice(c% < 32 Or c% > 126, "<" + Str$(c%) + ">", Chr$(c%))
  Next
  ut.sanitise_string$ = s2$
End Function

Sub ut.add_failure(msg$)
  Inc ut.asserts_fails%
  Print "FAILURE [" Str$(ut.asserts_count%) "] " msg$
End Sub

Sub ut.report()
  Print "Num successes: " Str$(ut.asserts_count% - ut.asserts_fails%)
  Print "Num failures:  " Str$(ut.asserts_fails%)
  Print Choice(ut.asserts_fails% = 0, "[OK]", "[FAIL]")
End Sub
