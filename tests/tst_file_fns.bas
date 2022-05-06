' Copyright (c) 2020-2022 Thomas Hugo Williams
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
Const CRLF$ = Chr$(13) + Chr$(10)
Const TMPDIR$ = sys.string_prop$("tmpdir")
Const BAD_FILE_DESCRIPTOR_ERR$ = "Bad file descriptor"
Const FILE_ALREADY_OPEN_ERR$ = Choice(Mm.Device$ = "MMB4L", "File or device already open", "File number already open")
Const FILE_NOT_OPEN_ERR$ = Choice(Mm.Device$ = "MMB4L", "File or device not open", "File number is not open")
Const INVALID_FILE_NBR_ERR$ = "Invalid file number"
Const MAX_FILE_NBR% = Choice(Mm.Device$ = "MMBasic for Windows", 128, 10)
Const SEPARATOR$ = sys.string_prop$("separator")

add_test("test_chdir_mkdir_rmdir")
add_test("test_close_errors")
add_test("test_copy")
add_test("test_dir")
add_test("test_dir_given_no_matches")
add_test("test_eof")
add_test("test_eof_errors")
add_test("test_inputstr")
add_test("test_kill")
add_test("test_loc")
add_test("test_loc_errors")
add_test("test_lof")
add_test("test_lof_errors")
add_test("test_rename")
add_test("test_seek")
add_test("test_seek_errors")
add_test("test_tilde_expansion")
add_test("test_open_errors")
add_test("test_append_eof_bug")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_chdir_mkdir_rmdir()
    Local current_dir$ = Mm.Info$(Directory))
    Local new_dir$ = "test_chdir_mkdir_rmdir.tmpdir"
    If file.exists%(TMPDIR$ + SEPARATOR$ + new_dir$) Then RmDir(TMPDIR$ + SEPARATOR$ + new_dir$)

    ChDir TMPDIR$
    MkDir new_dir$
    ChDir new_dir$

    Local expected$ = TMPDIR$ + SEPARATOR$ + new_dir$ + SEPARATOR$
    Select Case Mm.Device$
      Case "MMB4L", "MMBasic for Windows" : ' Do nothing
      Case Else : expected$ = UCase$(expected$)
    End Select
    assert_string_equals(expected$, Mm.Info$(Directory))

    ChDir ".."
    RmDir new_dir$

    assert_false(file.exists%(new_dir$))

    ChDir current_dir$
    assert_string_equals(current_dir$, Mm.Info$(Directory))
End Sub

Sub test_close_errors()
  ' Can't call on an unopened file.
  On Error Skip 1
  Close #1
  assert_raw_error(FILE_NOT_OPEN_ERR$)

  ' Can't call on file number #0.
  On Error Skip 1
  Close #0
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", INVALID_FILE_NBR_ERR$, "0 is invalid"))

  ' Can't call on file number #11.
  On Error Skip 1
  Close (MAX_FILE_NBR% + 1
  Local expected$
  Select Case Mm.Device$
    Case "MMB4L" :               expected$ = INVALID_FILE_NBR_ERR$
    Case "MMBasic for Windows" : expected$ = "129 is invalid (valid is 1 to 128)"
    Case Else :                  expected$ = "11 is invalid"
  End Select
End Sub

Sub given_test_file(f$)
  Open f$ For Output As #1
  Print #1, "Hello World"
  Print #1, "Goodbye World"
  Close #1
End Sub

Sub test_copy()
  Local f$ = TMPDIR$ + "/test_copy.tmp"
  Local f_copy$ = TMPDIR$ + "/test_copy.tmp.copy"
  Local s$

  given_test_file(f$)

  Copy f$ To f_copy$

  Open f$ For Input As #1
  s$ = Input$(28, #1)
  assert_string_equals("Hello World" + CRLF$ + "Goodbye World" + CRLF$, s$)
  assert_int_equals(1, Eof(#1))
  Close #1

  Open f_copy$ For Input As #1
  s$ = Input$(28, #1)
  assert_string_equals("Hello World" + CRLF$ + "Goodbye World" + CRLF$, s$)
  assert_int_equals(1, Eof(#1))
  Close #1
End Sub

Sub test_dir()
  Const tst_dir$ = TMPDIR$ + "\test_dir.tmpdir"
  If file.exists%(tst_dir$) Then RmDir tst_dir$
  MkDir tst_dir$
  Open tst_dir$ + "/file1" For Output As #1 : Close #1
  Open tst_dir$ + "/abc" For Output As #1 : Close #1
  MkDir tst_dir$ + "/subdir"
  Open tst_dir$ + "/file3" For Output As #1 : Close #1

  Local actual$(BASE% + 9)
  Local f$ = Dir$(tst_dir$ + "/*", ALL)
  Local index%
  Do While f$ <> ""
    actual$(BASE% + index%) = f$
    f$ = Dir$()
    Inc index%
  Loop

  Sort actual$(),,,,index%
  Local expected$(BASE% + 9) = ("abc", "file1", "file3", "subdir", "", "", "", "", "", "")
  assert_string_array_equals(expected$(), actual$())

  Kill tst_dir$ + "/file1"
  Kill tst_dir$ + "/abc"
  Kill tst_dir$ + "/file3"
  RmDir tst_dir$ + "/subdir"
  RmDir tst_dir$
End Sub

Sub test_dir_given_no_matches()
  Const tst_dir$ = TMPDIR$ + "\test_dir_given_no_matches.tmpdir"
  If file.exists%(tst_dir$) Then RmDir tst_dir$
  MkDir tst_dir$
  Local f$ = Dir$(tst_dir$ + "/*.non")
  assert_string_equals("", f$)
  RmDir tst_dir$
End Sub

Sub test_eof()
  Local f$ = TMPDIR$ + "/test_eof.tmp"
  Local i%

  ' Test when file opened for INPUT.
  given_test_file(f$)
  Open f$ For Input As #1
  assert_int_equals(0, Eof(#1))
  Local s$ = Input$(255, #1)
  assert_int_equals(1, Eof(#1))
  Close #1

  ' Test on file file number #0.
  Select Case Mm.Device$
    Case "MMB4L" : assert_int_equals(0, Eof(#0)) ' TODO
    Case Else    : assert_int_equals(1, Eof(#0))
  End Select

  ' Test when file opened for OUTPUT.
  Open f$ For Output As #1
  If Mm.Device$ = "MMB4L" Then
    On Error Skip 1
    i% = Eof(#1)
    assert_raw_error(BAD_FILE_DESCRIPTOR_ERR$)
  Else
    assert_int_equals(1, Eof(#1))
    Print #1, "Hello World"
    assert_int_equals(1, Eof(#1))
  EndIf
  Close #1

  ' Test when file opened for APPEND.
  Open f$ For Append As #1
  If Mm.Device$ = "MMB4L" Then
    On Error Skip 1
    i% = Eof(#1)
    assert_raw_error(BAD_FILE_DESCRIPTOR_ERR$)
  Else
    assert_int_equals(1, Eof(#1))
    Print #1, "Goodbye World"
    assert_int_equals(1, Eof(#1))
  EndIf
  Close #1

  ' Test when file opened for RANDOM.
  given_test_file(f$)
  Open f$ For Random As #1
  assert_int_equals(1, Eof(#1)) ' File opens with pointer at end.
  Seek #1, 1                    ' The first byte is numbered 1.
  assert_int_equals(0, Eof(#1))
  Seek #1, Mm.Info(FileSize f$)
  assert_int_equals(0, Eof(#1)) ' We're at the end of the file, why does this not return 1 ?
  Seek #1, Mm.Info(FileSize f$) + 1
  assert_int_equals(1, Eof(#1))
  Close #1
End Sub

Sub test_eof_errors()
  Local f$ = TMPDIR$ + "/test_eof_errors.tmp"
  Local i%, s$

  ' Test on an unopened file.
  On Error Skip 1
  i% = Eof(#1)
  Select Case Mm.Device$
    Case "MMB4L" :               assert_raw_error("File or device not open")
    Case "MMBasic for Windows" : assert_raw_error("File number 1 is not open")
    Case Else :                  assert_raw_error("File number is not open")
  End Select

  ' Test on file number #10.
  given_test_file(f$)
  Open f$ For Input As MAX_FILE_NBR%
  assert_int_equals(0, Eof(MAX_FILE_NBR%))
  s$ = Input$(255, MAX_FILE_NBR%)
  assert_int_equals(1, Eof(MAX_FILE_NBR%))
  Close MAX_FILE_NBR%

  ' Test on file number #11.
  On Error Skip 1
  i% = Eof(MAX_FILE_NBR% + 1)
  Select Case Mm.Device$
    Case "MMB4L" :               assert_raw_error(INVALID_FILE_NBR_ERR$)
    Case "MMBasic for Windows" : assert_raw_error(INVALID_FILE_NBR_ERR$)
    Case Else :                  assert_raw_error("File number")
  End Select
End Sub

Sub test_inputstr()
  Local f$ = TMPDIR$ + "/test_inputstr.tmp"

  ' Test on file opened for INPUT.
  given_test_file(f$)
  Open f$ For Input As #1
  Local s$ = Input$(28, #1)
  assert_string_equals("Hello World" + CRLF$ + "Goodbye World" + CRLF$, s$)
  Close #1

  ' Test on unopened file.
  On Error Skip 1
  s$ = Input$(28, #1)
  assert_raw_error(FILE_NOT_OPEN_ERR$)

  ' NOTE you can call on file number #0, but I can't automatically test this.

  ' Test on file number #10.
  Open f$ For Input As MAX_FILE_NBR%
  s$ = Input$(28, MAX_FILE_NBR%)
  assert_string_equals("Hello World" + CRLF$ + "Goodbye World" + CRLF$, s$)
  Close MAX_FILE_NBR%

  ' Test on file number #11.
  On Error Skip 1
  s$ = Input$(28, MAX_FILE_NBR% + 1)
  Select Case Mm.Device$
    Case "MMB4L" : assert_raw_error(INVALID_FILE_NBR_ERR$)
    Case Else :    assert_raw_error("File number")
  End Select
End Sub

Sub test_kill() {
    Local f$ = TMPDIR$ + "/test_kill.tmp"

    given_test_file(f$)

    assert_true(file.exists%(f$))

    Kill f$

    assert_false(file.exists%(f$))
End Sub

Sub test_rename()
    Local f$ = TMPDIR$ + "/test_new.tmp"
    Local f_new$ = TMPDIR$ + "/test_new.tmp.new"
    Local s$

    given_test_file(f$)

    ' CMM2 will not RENAME over an existing file.
    If file.exists%(f_new$) Then Kill f_new$
    Rename f$ As f_new$

    assert_false(file.exists%(f$))

    Open f_new$ For Input As #1
    s$ = Input$(28, #1)
    assert_string_equals("Hello World" + CRLF$ + "Goodbye World" + CRLF$, s$)
    assert_int_equals(1, Eof(#1))
    Close #1
End Sub

Sub test_loc()
    Local f$ = TMPDIR$ + "/test_loc.tmp"

    ' Test when file opened for INPUT.
    Open f$ For Output As #1
    assert_int_equals(1, Loc(#1))
    Print #1, "foo";
    assert_int_equals(4, Loc(#1))
    Close #1
    Kill f$

    ' Test when non-existing file opened for RANDOM.
    Open f$ For Random As #1
    assert_int_equals(1, Loc(#1))
    Print #1, "foo";
    assert_int_equals(4, Loc(#1))
    Close #1
    Kill f$

    ' Test when existing empty file opened for RANDOM.
    Open f$ For Output As #1
    Close #1
    Open f$ For Random As #1
    assert_int_equals(1, Loc(#1))
    Print #1, "foo";
    assert_int_equals(4, Loc(#1))
    Close #1
    Kill f$

    ' Test when existing non-empty file opened for RANDOM,
    ' starts with r/w pointer one past the end.
    given_test_file(f$)
    Open f$ For Random As #1
    assert_int_equals(29, Loc(#1))
    Close #1
    Kill f$

    ' Test when existing non-empty file opened for INPUT.
    given_test_file(f$)
    Open f$ For Input As #1
    assert_int_equals(1, Loc(#1))
    Close #1
    Kill f$

    ' Test when existing non-empty file opened for APPEND.
    given_test_file(f$)
    Open f$ For Append As #1
    Select Case Mm.Device$
      Case "MMBasic for Windows" : assert_int_equals(1, Loc(#1))
      Case Else :                  assert_int_equals(29, Loc(#1))
    End Select
    Close #1
    Kill f$
End Sub

Sub test_loc_errors()
  ' Test on an unopened file.
  On Error Skip 1
  Local i% = Loc(#1)
  assert_raw_error(FILE_NOT_OPEN_ERR$)

  ' Test on file number #0.
  On Error Skip 1
  i% = Loc(#0)
  Select Case Mm.Device$
    Case "MMB4L" : assert_raw_error(INVALID_FILE_NBR_ERR$)
    Case Else :    assert_int_equals(0, Mm.ErrNo)
  End Select

  ' Test on file number #10.
  Local f$ = TMPDIR$ + "/test_loc_errors.tmp"
  Open f$ For Output As MAX_FILE_NBR%
  Close MAX_FILE_NBR%
  Open f$ For Random As MAX_FILE_NBR%
  assert_int_equals(1, Loc(MAX_FILE_NBR%))
  Close MAX_FILE_NBR%

  ' Test on file number #11.
  On Error Skip 1
  i% = Loc(MAX_FILE_NBR% + 1)
  Select Case Mm.Device$
    Case "MMB4L" : assert_raw_error(INVALID_FILE_NBR_ERR$)
    Case Else :    assert_raw_error("File number")
  End Select
End Sub

Sub test_lof()
  Local f$ = TMPDIR$ + "/test_lof.tmp"

  ' Test when writing a file.
  Open f$ For Output As #1
  Print #1, "Hello World"
  assert_int_equals(13, Lof(#1))
  Close #1

  ' Test when file opened for INPUT.
  Open f$ For Input As #1
  assert_int_equals(13, Lof(#1))
  Local s$ = Input$(5, #1)
  assert_string_equals("Hello", s$)
  assert_int_equals(13, Lof(#1))
  Close #1

  ' Test when file opened for APPEND.
  Open f$ For Append As #1
  assert_int_equals(13, Lof(#1))
  Print #1, "Goodbye World"
  assert_int_equals(28, Lof(#1))
  Close #1

  ' Test when file opened for RANDOM.
  Open f$ For Random As #1
  assert_int_equals(28, Lof(#1))
  Seek #1, 10
  assert_int_equals(28, Lof(#1))
  Seek #1, Lof(#1) + 1
  Print #1, "And there's more"
  assert_int_equals(46, Lof(#1))
  Close #1
End Sub

Sub test_lof_errors()
  ' Test on an unopened file.
  On Error Skip 1
  Local i% = Lof(#1)
  assert_raw_error(FILE_NOT_OPEN_ERR$)

  ' Test on file number #0.
  On Error Skip 1
  i% = Lof(#0)
  Select Case Mm.Device$
    Case "MMB4L" : assert_raw_error(INVALID_FILE_NBR_ERR$)
    Case Else :    assert_int_equals(0, Mm.ErrNo)
  End Select

  ' Test on file number #10.
  Local f$ = TMPDIR$ + "/test_lof_errors.tmp"
  Open f$ For Output As MAX_FILE_NBR%
  Print #MAX_FILE_NBR%, "Hello World";
  Close MAX_FILE_NBR%
  Open f$ For Random As MAX_FILE_NBR%
  assert_int_equals(11, Lof(MAX_FILE_NBR%))
  Close MAX_FILE_NBR%

  ' Test on file number #11.
  On Error Skip 1
  i% = Lof(MAX_FILE_NBR% + 1)
  Select Case Mm.Device$
    Case "MMB4L" : assert_raw_error(INVALID_FILE_NBR_ERR$)
    Case Else :    assert_raw_error("File number")
  End Select
End Sub

Sub test_seek()
    Local f$ = TMPDIR$ + "/test_seek.tmp"
    Local s$

    given_test_file(f$)

    Open f$ For Random As #1

    Seek #1, 7
    s$ = Input$(5, #1)
    assert_string_equals("World", s$)
    s$ = Input$(9, #1)
    assert_string_equals(CRLF$ + "Goodbye", s$)

    ' Seek back and read the same again.
    Seek #1, 7
    s$ = Input$(5, #1)
    assert_string_equals("World", s$)
    s$ = Input$(9, #1)
    assert_string_equals(CRLF$ + "Goodbye", s$)

    Close #1
End Sub

Sub test_seek_errors()
  ' Test on an unopened file.
  On Error Skip 1
  Seek #1, 1
  assert_raw_error(Choice(Mm.Device$ = "MMB4L", FILE_NOT_OPEN_ERR$, "File number"))

  ' Test on file number #0.
  On Error Skip 1
  Seek #0, 1
  Select Case Mm.Device$
    Case "MMB4L" : assert_raw_error(INVALID_FILE_NBR_ERR$)
    Case Else :    assert_raw_error("File number")
  End Select

  ' Test on file number #10.
  Local f$ = TMPDIR$ + "/test_lof_errors.tmp"
  Open f$ For Output As MAX_FILE_NBR%
  Print #MAX_FILE_NBR%, "Hello World"
  Print #MAX_FILE_NBR%, "Goodbye World"
  Close MAX_FILE_NBR%
  Open f$ For Random As MAX_FILE_NBR%
  Seek MAX_FILE_NBR%, 7
  Local s$ = Input$(5, MAX_FILE_NBR%)
  assert_string_equals("World", s$)
  s$ = Input$(9, MAX_FILE_NBR%)
  assert_string_equals(CRLF$ + "Goodbye", s$)
  Close MAX_FILE_NBR%

  ' Test on file number #11.
  On Error Skip 1
  Seek MAX_FILE_NBR% + 1, 1
  Select Case Mm.Device$
    Case "MMB4L" : assert_raw_error(INVALID_FILE_NBR_ERR$)
    Case Else :    assert_raw_error("File number")
  End Select
End Sub

Sub test_tilde_expansion()
  If Mm.Device$ <> "MMB4L" Then Exit Sub

  Local original_dir$ = Cwd$

  System "rm -Rf " + TMPDIR$ + "/test_tilde_expansion.dir"

  ' Test CHDIR.
  ChDir "~"
  assert_string_equals(Mm.Info$(EnvVar "HOME"), Cwd$)

  ' Use SYSTEM with 'realpath' to determine relative path from HOME to TMPDIR.
  Local s$
  System "realpath --relative-to=$HOME " + TMPDIR$, s$
  Local tmp_relative$ = "~/" + s$

  ChDir tmp_relative$
  assert_string_equals(TMPDIR$, Cwd$)

  ' Test MKDIR.
  Local my_test_dir$ = tmp_relative$ + "/test_tilde_expansion.dir"
  MkDir my_test_dir$
  assert_true(Mm.Info(Exists my_test_dir$))

  ' Test OPEN.
  Local my_test_file$ = my_test_dir$ + "/my_test_file.txt"
  Open my_test_file$ For Output As #1
  Print #1, "Hello World"
  Close #1

  Open my_test_file$ For Input As #1
  Line Input #1, s$
  Close #1
  assert_string_equals("Hello World", s$)

  ' Test COPY.
  Copy my_test_file$ To my_test_file$ + ".copy"
  assert_true(Mm.Info(Exists my_test_file$ + ".copy"))

  ' Test RENAME.
  Rename my_test_file$ + ".copy" As my_test_file$ + ".copy2"
  assert_false(Mm.Info(Exists my_test_file$ + ".copy"))
  assert_true(Mm.Info(Exists my_test_file$ + ".copy2"))

  ' Test DIR.
  Local actual_files$(array.new%(5))
  Local f$ = Dir$(my_test_dir$ + "/*", ALL)
  Local index%
  Do While f$ <> ""
    actual_files$(BASE% + index%) = f$
    f$ = Dir$()
    Inc index%
  Loop
  Sort actual_files$(),,,,index%
  Local expected_files$(array.new%(5)) = ("my_test_file.txt", "my_test_file.txt.copy2", "", "", "")
  assert_string_array_equals(expected_files$(), actual_files$())

  ' Test KILL.
  Kill my_test_file$
  assert_false(Mm.Info(Exists my_test_file$))
  Kill my_test_file$ + ".copy2"
  assert_false(Mm.Info(Exists my_test_file$ + ".copy2"))

  ' Test RMDIR.
  RmDir my_test_dir$
  assert_false(Mm.Info(Exists my_test_dir$))

  ChDir original_dir$
End Sub

Sub test_open_errors()
  Open TMPDIR$ + "/test_open_errors.txt" For Output As #1

  ' Cannot use a file number that is already open.
  On Error Skip 1
  Open TMPDIR$ + "/test_open_errors.txt.2" For Output As #1
  assert_raw_error(FILE_ALREADY_OPEN_ERR$)

  ' But can use it after it is closed.
  Close #1
  Open TMPDIR$ + "/test_open_errors.txt.2" For Output As #1
  Close #1

  ' Can't open file number #0.
  On Error Skip 1
  Open TMPDIR$ + "/test_open_errors.txt" For Output As #0
    Select Case Mm.Device$
    Case "MMB4L" : assert_raw_error(INVALID_FILE_NBR_ERR$)
    Case Else :    assert_raw_error("File number")
  End Select

  ' Can use file number #10.
  Open TMPDIR$ + "/test_open_errors.txt" For Output As MAX_FILE_NBR%
  Close MAX_FILE_NBR%

  ' Can't use file number #11.
  On Error Skip 1
  Open TMPDIR$ + "/test_open_errors.txt" For Output As MAX_FILE_NBR% + 1
    Select Case Mm.Device$
    Case "MMB4L" : assert_raw_error(INVALID_FILE_NBR_ERR$)
    Case Else :    assert_raw_error("File number")
  End Select
End Sub

Sub test_append_eof_bug()
  Local filename$ = TMPDIR + "/test_append_eof_bug.txt"

  If file.exists%(filename$) Then Kill filename$

  Open filename$ For Append As #1
  Print #1, "Goodbye World"
  If Mm.Device$ = "MMB4L" Then On Error Skip
  Local i% = Eof(#1)
  If Mm.Device$ = "MMB4L" Then assert_raw_error(BAD_FILE_DESCRIPTOR_ERR$)
  Close #1

  Open filename$ For Input As #1
  Local s$ = Input$(255, #1)
  Close #1

  assert_int_equals(15, Len(s$))
End Sub
