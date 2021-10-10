' Copyright (c) 2020-2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

On Error Skip 1 : Dim sys.VERSION$ = ""
If sys.VERSION$ = "" Then Error "'system.inc' not included"
sys.requires("array", "file", "list", "string", "vt100")
sys.provides("unittest")
If sys.err$ <> "" Then Error sys.err$

Const ut.MAX_FAILURES% = 100
Const ut.MAX_TESTS% = 100

Dim ut.asserts_count%
Dim ut.failure_count%
Dim ut.failure_len%(array.new%(ut.MAX_FAILURES%))
Dim ut.failure_start%(array.new%(ut.MAX_FAILURES%))
Dim ut.failure_txt%(1000)
Dim ut.test_names$(list.new%(ut.MAX_TESTS%)) Length 128
Dim ut.test_subs$(list.new%(ut.MAX_TESTS%)) Length 40

list.init(ut.test_names$())
list.init(ut.test_subs$())

Sub add_test(name$, sub$)
  list.add(ut.test_names$(), name$)
  list.add(ut.test_subs$(), Choice(sub$ = "", name$, sub$))
End Sub

Sub ut.add_failure(msg$)
  Inc ut.failure_count%
  ut.failure_start%(ut.failure_count% + Mm.Info(Option Base) - 1) = LLen(ut.failure_txt%()) + 1
  LongString Append ut.failure_txt%(), Str$(ut.asserts_count%) + ": " + msg$
  ut.update_failure()
End Sub

Sub ut.update_failure()
  Local i% = ut.failure_count% + Mm.Info(Option Base) - 1
  ut.failure_len%(i%) = LLen(ut.failure_txt%()) - ut.failure_start%(i%) + 1
End Sub

Sub assert_fail(msg$)
  Inc ut.asserts_count%
  ut.add_failure(msg$)
End Sub

Sub assert_false(z%, msg$)
  Inc ut.asserts_count%
  If z% Then ut.add_failure(Choice(msg$ = "", "assert_false() failed", msg$))
End Sub

Sub assert_true(z%, msg$)
  Inc ut.asserts_count%
  If Not z% Then ut.add_failure(Choice(msg$ = "", "assert_true() failed", msg$))
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

Sub assert_float_equals(expected!, actual!, delta!)
  Inc ut.asserts_count%
  If Not equals_float%(expected!, actual!, delta!) Then
    Local s$ = "Assert equals failed, expected " + Str$(expected!)
    s$ = s$ + " but actually " + Str$(actual!)
    ut.add_failure(s$)
  EndIf
End Sub

Function equals_float%(a!, b!, delta!)
  equals_float% = (a! >= b! - delta!) And (a! <= b! + delta!)
End Function

Sub assert_string_equals(expected_$, actual_$)
  Inc ut.asserts_count%
  If expected_$ <> actual_$ Then
    Local expected$ = str.quote$(expected_$)
    Local actual$ = str.quote$(actual_$)
    If Len(expected_$) = 1 Then expected$ = "Chr$(" + Str$(Asc(expected_$)) + ")"
    If Len(actual_$) = 1 Then actual$ = "Chr$(" + Str$(Asc(actual_$)) + ")"

    Local s$ = "Assert equals failed, expected " + ut.sanitise_string$(expected$)
    Cat s$, " but actually " + ut.sanitise_string$(actual$)
    ut.add_failure(s$)
  EndIf
End Sub

Function ut.sanitise_string$(s$)
  Local c%, i%, s2$
  For i% = 1 To Len(s$)
    c% = Peek(Var s$, i%)
    Cat s2$, Choice(c% < 32 Or c% > 126, "<" + Str$(c%) + ">", Chr$(c%))
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

Sub assert_float_array_equals(expected!(), actual!(), delta!())
  Local base% = Mm.Info(Option Base)
  Local fail% = 0

  Inc ut.asserts_count%

  If Bound(expected!(), 1) = Bound(actual!(), 1) Then
    Local i%, lb%, ub%
    lb% = base%
    ub% = Bound(expected!(), 1)
    For i% = lb% To ub%
      If Not equals_float%(expected!(i%), actual!(i%), delta!(i%)) Then fail% = 1 : Exit For
    Next
  Else
    fail% = 1
  EndIf

  If fail% Then
    Local s$ = "Assert array equals failed, expected:" + sys.CRLF$ + "      "
    For i% = base% To Bound(expected!(), 1)
      If i% <> base% Then Cat s$, ", "
      Cat s$, Str$(expected!(i%))
    Next
    Cat s$, sys.CRLF$ + "    but actually: " + sys.CRLF$ + "      "
    For i% = base% To Bound(actual!(), 1)
      If i% <> base% Then Cat s$, ", "
      Cat s$, Str$(actual!(i%))
    Next
    ut.add_failure(s$)
  EndIf
End Sub

Sub assert_string_array_equals(expected$(), actual$())
  Local base% = Mm.Info(Option Base)
  Local fail% = 0

  Inc ut.asserts_count%

  If Bound(expected$(), 1) = Bound(actual$(), 1) Then
    Local i%, lb%, ub%
    lb% = base%
    ub% = Bound(expected$(), 1)
    For i% = lb% To ub%
      If expected$(i%) <> actual$(i%) Then fail% = 1 : Exit For
    Next
  Else
    fail% = 1
  EndIf

  If fail% Then
    ut.add_failure("Assert array equals failed, expected:")
    Local width% = Len(Str$(Bound(expected$(), 1)))
    For i% = base% To Bound(expected$(), 1)
      If expected$(i%) = actual$(i%) Then Continue For
      LongString Append ut.failure_txt%(), sys.CRLF$
      LongString Append ut.failure_txt%(), Format$(i%, "      [%" + Str$(width%) + ".0f] ")
      LongString Append ut.failure_txt%(), str.quote$(ut.sanitise_string$(expected$(i%))))
    Next
    LongString Append ut.failure_txt%(), sys.CRLF$ + "    but actually:")
    For i% = base% To Bound(actual$(), 1)
      If expected$(i%) = actual$(i%) Then Continue For
      LongString Append ut.failure_txt%(), sys.CRLF$
      LongString Append ut.failure_txt%(), Format$(i%, "      [%" + Str$(width%) + ".0f] ")
      LongString Append ut.failure_txt%(), str.quote$(ut.sanitise_string$(actual$(i%))))
    Next
    ut.update_failure()
  EndIf
End Sub

Sub assert_no_error()
  Inc ut.asserts_count%
  If sys.err$ <> "" Then
    Local s$ = "Expected no error, but actually " + str.quote$(sys.err$)
    ut.add_failure(s$)
  EndIf
End Sub

Sub assert_error(expected$)
  Inc ut.asserts_count%
  If sys.err$ <> expected$ Then
    Local s$ = "Expected Error " + str.quote$(expected$) + ", but actually " + str.quote$(sys.err$)
    ut.add_failure(s$)
  EndIf
End Sub

Sub assert_raw_error(expected$)
  Inc ut.asserts_count%
  If InStr(Mm.ErrMsg$, expected$) < 1 Then
    Local s$ = "Expected Error " + str.quote$(expected$)
    Cat s$, ", but actually " + str.quote$(Mm.ErrMsg$)
    ut.add_failure(s$)
  EndIf
End Sub

' @param  repeat$  if this is not empty then after running the test suite the program
'                  should run itself again appending 'repeat$' to its command line.
Sub run_tests(repeat$)
  Local num_failed_tests% = 0
  Local num_tests% = list.size%(ut.test_names$())
  Local test$ = Mm.Info$(Current) + " " + Mm.CmdLine$;

  Print test$;
  If ut.is_verbose%() Then Print

  Local t% = Timer

  Local i%
  Local lb% = Mm.Info(Option Base)
  Local ub% = lb% + num_tests% - 1
  For i% = lb% To ub%
    If Not ut.run_single_test%(i%) Then Inc num_failed_tests%
  Next

  If ut.is_verbose%() Then
    Print "  Execution time: " Str$((Timer - t%) / 1000) " s"
    Print "  ";
  Else
    Print Space$(Max(1, 85 - Len(test$)));
  EndIf

  If num_tests% = 0 Then
    ut.print_colour("magenta")
    Print "NO TESTS";
  Else If num_failed_tests% = 0 Then
    ut.print_colour("green")
    Print "PASS (" Str$(num_tests%) "/" Str$(num_tests%) ")";
  Else
    ut.print_colour("red")
    Print "FAIL (" Str$(num_failed_tests%) "/" Str$(num_tests%) ")";
  EndIf
  ut.print_colour("reset")
  Print

  If repeat$ <> "" Then
    ut.run_same(repeat$)
  ElseIf InStr(Mm.CmdLine$, "--all") Then
    ut.run_next()
  EndIf
End Sub

' @return 1 if the test passes, 0 if it fails.
Function ut.run_single_test%(idx%)
  Local catch_errors% = ut.is_catch_errors%()

  If ut.is_verbose%() Then
    Print "  " + ut.test_names$(idx%); ":"; Space$(Max(1, 50 - Len(ut.test_names$(idx%))));
  EndIf

  ut.asserts_count% = 0
  ut.failure_count% = 0
  LongString Clear ut.failure_txt%()

  If catch_errors% Then On Error Ignore

  sys.err$ = ""
  setup_test()
  Call ut.test_subs$(idx%)
  teardown_test()

  If catch_errors% Then
    If Mm.ErrNo <> 0 Then add_mmbasic_error()
    On Error Abort
  EndIf

  If ut.is_verbose%() Then
    If ut.asserts_count% = 0 Then
      ut.print_colour("magenta")
      Print "NO TESTS";
    Else If ut.failure_count% = 0 Then
      ut.print_colour("green")
      Print "PASS ("; Str$(ut.asserts_count%); "/"; Str$(ut.asserts_count%); ")";
    Else If ut.failure_count% > 0 Then
      ut.print_colour("red")
      Print "FAIL ("; Str$(ut.failure_count%); "/"; Str$(ut.asserts_count%); ")";
      Local i%, j%
      Local lb% = Mm.Info(Option Base)
      Local ub% = lb% + ut.failure_count% - 1
      For i% = lb% To ub%
        Print
        Print "    ";
        j% = ut.failure_start%(i%)
        Do
          Print LGetStr$(ut.failure_txt%(), j%, Min(255, ut.failure_len%(i%)));
          Inc j%, Min(255, ut.failure_len%(i%))
        Loop While j% < ut.failure_len%(i%)
      Next
    EndIf
    ut.print_colour("reset")
    Print
  EndIf

  ut.run_single_test% = ut.failure_count% = 0
End Function

' Should verbose output be generated?
Function ut.is_verbose%()
  ut.is_verbose% = InStr(" " + Mm.CmdLine$ + " ", " --all ") < 1
  If Not ut.is_verbose% Then ut.is_verbose% = InStr(" " + Mm.CmdLine$ + " ", " --verbose ") > 0
End Function

' Should MMBasic errors be caught and reported as assertion failures?
Function ut.is_catch_errors%()
  ut.is_catch_errors% = InStr(" " + Mm.CmdLine$ + " ", " --catch-errors ")
End Function

Sub add_mmbasic_error()
  Inc ut.asserts_count%
  ut.add_failure("MMBasic " + Mid$(Mm.ErrMsg$, InStr(Mm.ErrMsg$, "Error")))
End Sub

' First:  creates the file "/tmp/sptest.list" containing the full paths (one per line)
'         of each sptest suite/file found by walking the file-system from the current
'         working directory.
' Second: executes the first sptest suite/file found.

Sub ut.run_first()
  Print "Building list of tests ..."
  Open "/tmp/sptest.lst" For Output As #1
  Print #1, Time$
  Local f$ = file.find$(Cwd$, "*st*.bas", "file")
  Local first$
  Do While f$ <> ""
    If ut.is_test_file%(f$) Then
      Print #1, f$
      If first$ = "" Then first$ = f$
    EndIf
    f$ = file.find$()
  Loop
  Close #1

  If first$ = "" Then
    Print "No tests found."
    End
  Else
    Print "Executing tests ..."
    Local cmd$ = "Run " + str.quote$(first$) + ", --all"
    If InStr(" " + Mm.CmdLine$ + " ", " --catch-errors") Then Cat cmd$, " --catch-errors"
    If InStr(" " + Mm.CmdLine$ + " ", " --verbose ")     Then Cat cmd$, " --verbose"
    Execute cmd$
  EndIf
End Sub

Function ut.is_test_file%(f$)
  Local name$ = file.get_name$(f$)

  ' Check name matches pattern for sptest files.
  If file.fnmatch%("test_*.bas", name$) Then ut.is_test_file% = 1
  If file.fnmatch%("tst_*.bas", name$) Then ut.is_test_file% = 1
  If file.fnmatch%("*_test.bas", name$) Then ut.is_test_file% = 1
  If file.fnmatch%("*_tst*.bas", name$) Then ut.is_test_file% = 1

  If Not ut.is_test_file% Then Exit Function

  ' Scan first 50 lines of file for #Include of "unittest.inc".
  Local i%, s$
  ut.is_test_file% = 0
  Open f$ For Input As #2
  For i% = 1 To 50
    If Eof(#2) Then Exit For
    Line Input #2, s$
    If file.fnmatch%("*#include*unittest.inc*", s$) Then ut.is_test_file% = 1 : Exit For
  Next
  Close #2

End Function

Function ut.parse_time%(t$)
  Inc ut.parse_time%, Val(Mid$(t$, 1, 2)) * 60 * 60
  Inc ut.parse_time%, Val(Mid$(t$, 4, 2)) * 60
  Inc ut.parse_time%, Val(Mid$(t$, 7, 2))
End Function

Sub ut.run_next()
  Local f$, start_time$
  Open "/tmp/sptest.lst" For Input As #1
  Line Input #1, start_time$

  Do
    Line Input #1, f$
    If f$ = file.get_canonical$(Mm.Info$(Current)) Then Exit Do
  Loop While Not Eof(#1)

  If Eof(#1) Then
    Close #1
    Local start% = ut.parse_time%(start_time$)
    Local end% = ut.parse_time%(Time$)
    Print "Total execution time: " + Str$(end% - start%) + " s"
  Else
    Line Input #1, f$
    Close #1
    Local cmd$ = "Run " + str.quote$(f$) + ", --all"
    If ut.is_catch_errors%() Then Cat cmd$, " --catch-errors"
    If ut.is_verbose%()      Then Cat cmd$, " --verbose"
    Execute cmd$
  EndIf
End Sub

' Runs the current test file again appending 'cmd$' to its command line.
Sub ut.run_same(cmd$)
  Local tgt$ = "Run " + str.quote$(Mm.Info(Current)) + ", "
  If Mm.CmdLine$ <> "" Then tgt$ = tgt$ + Mm.CmdLine$ + " "
  tgt$ = tgt$ + cmd$
  Execute tgt$
End Sub

Sub ut.print_colour(c$)
  Option Console Serial
  Print vt100.colour$(c$);

  ' TODO: Should really restore the previous value of "Option Console",
  '       but currently that cannot be read.
  Option Console Both

  Select Case c$
    Case "green"   : Colour(RGB(Green))
    Case "magenta" : Colour(RGB(Magenta))
    Case "red"     : Colour(RGB(Red))
    Case "reset"   : Colour(RGB(White))
    Case Else      : Error "unsupported colour: " + c$
  End Select
End Sub
