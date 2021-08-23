Option Explicit On
Option Default None
Option Base 0

Sub run_tests()
  test_set()
  test_set_byte()
  test_set_short()
  test_set_word()
  test_set_integer()
  test_set_float()
  test_copy()
  test_copy_byte()
  test_copy_short()
  test_copy_word()
  test_copy_integer()
  test_copy_float()
End Sub

Sub test_set()
  Local buf%(32) ' 256-bytes
  Local addr% = Peek(VarAddr buf%())

  ' Local j%
  ' Print "Initial: " Hex$(addr%) " ";
  ' For j% = 0 To 9
  '   Print Peek(Byte addr% + j%);
  ' Next
  ' Print

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

Sub test_set_word()
End Sub

Sub test_set_integer()
End Sub

Sub test_set_float()
End Sub

Sub test_copy()
  Local dst%(31), src%(31) ' 256-bytes
  Local dst_addr% = Peek(VarAddr dst%())
  Local src_addr% = Peek(VarAddr src%())
  Local i%
  For i% = 0 To 31
    src%(i%) = &h0102030405060708
  Next

  Memory Copy src_addr%, dst_addr%, 256

  assert_int_array_equals(src%(), dst%())
End Sub

Sub test_copy_byte()
End Sub

Sub test_copy_short()
End Sub

Sub test_copy_word()
End Sub

Sub test_copy_integer()
End Sub

Sub test_copy_float()
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

Sub assert_int_array_equals(expected%(), actual%())
  Local base% = Mm.Info(Option Base)
  Local fail% = 0

  Inc ut.asserts_count%

  If Bound(expected%(), 1) = Bound(actual%(), 1) Then
    Local i%, lb%, ub%
    lb% = base%
    ub% = Bound(expected%(), 1)
    For i% = lb% To ub%
      If expected%(i%) <> actual%(i%) Then fail% = 1 : Exit For
    Next
  Else
    fail% = 1
  EndIf

  If fail% Then
    Local s$ = "Assert array equals failed, expected:" + sys.CRLF$ + "      "
    For i% = base% To Bound(expected%(), 1)
      If i% <> base% Then Cat s$, ", "
      Cat s$, Str$(expected%(i%))
    Next
    Cat s$, sys.CRLF$ + "    but actually: " + sys.CRLF$ + "      "
    For i% = base% To Bound(actual%(), 1)
      If i% <> base% Then Cat s$, ", "
      Cat s$, Str$(actual%(i%))
    Next
    ut.add_failure(s$)
  EndIf
End Sub

Sub ut.add_failure(msg$)
  Inc ut.asserts_fails%
  Print "FAILURE [" Str$(ut.asserts_count%) "] " msg$
End Sub

Sub ut.report()
  Print "Num successes: " Str$(ut.asserts_count% - ut.asserts_fails%)
  Print "Num failures:  " Str$(ut.asserts_fails%)
  Print Choice(ut.asserts_fails% = 0, "[OK]", "[FAIL]")
End Sub
