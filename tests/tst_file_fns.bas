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
Const CRLF$ = Chr$(13) + Chr$(10)

add_test("test_chdir_mkdir_rmdir")
add_test("test_close_errors")
add_test("test_copy")
add_test("test_dir")
add_test("test_eof")
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

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_chdir_mkdir_rmdir()
    Local current_dir$ = Mm.Info$(Directory))
    Local new_dir$ = "test_chdir_mkdir_rmdir.tmpdir"
    If file.exists%(file.TMPDIR$ + "/" + new_dir$) Then RmDir(file.TMPDIR$ + "/" + new_dir$)

    ChDir file.TMPDIR$
    MkDir new_dir$
    ChDir new_dir$

    Local expected$ = file.TMPDIR$ + "/" + new_dir$ + "/"
    If Mm.Device$ <> "MMB4L" Then expected$ = "A:" + UCase$(expected$)
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
  assert_raw_error("File or device not open")

  ' Can't call on file number #0.
  On Error Skip 1
  Close #0
  assert_raw_error("Invalid file number")

  ' Can't call on file number #11.
  On Error Skip 1
  Close #11
  assert_raw_error("Invalid file number")
End Sub

Sub test_copy()
    Local f$ = file.TMPDIR$ + "/test_copy.tmp"
    Local f_copy$ = file.TMPDIR$ + "/test_copy.tmp.copy"
    Local s$

    Open f$ For Output As #1
    Print #1, "Hello World"
    Print #1, "Goodbye World"
    Close #1

    Copy f$ To f_copy$

    Open f$ For Input As #1
    s$ = Input$(28, #1)
    assert_string_equals("Hello World" + CRLF$ + "Goodbye World" + CRLF$, s$)
    assert_true(Eof(#1))
    Close #1

    Open f_copy$ For Input As #1
    s$ = Input$(28, #1)
    assert_string_equals("Hello World" + CRLF$ + "Goodbye World" + CRLF$, s$)
    assert_true(Eof(#1))
    Close #1
End Sub

Sub test_dir()
  Local tst_dir$ = file.TMPDIR$ + "/test_dir.tmpdir"
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
End Sub

Sub test_eof()
  Local f$ = file.TMPDIR$ + "/test_eof.tmp"

  ' Write some test data.
  Open f$ For Output As #1
  Print #1, "Hello World"
  Print #1, "Goodbye World"
  Close #1

  ' Simplest test.
  Open f$ For Input As #1
  assert_int_equals(0, Eof(#1))
  Local s$ = Input$(255, #1)
  assert_int_equals(-1, Eof(#1))
  Close #1

  ' Can't call on an unopened file.
  On Error Skip 1
  Local i% = Eof(#1)
  assert_raw_error("File or device not open")

  ' You can call on file number #0, it always returns 0.
  assert_int_equals(0, Eof(#0))

  ' Can call on file number #10.
  Open f$ For Input As #10
  assert_int_equals(0, Eof(#10))
  s$ = Input$(255, #10)
  assert_int_equals(-1, Eof(#10))
  Close #10

  ' Can't call on file number #11.
  On Error Skip 1
  i% = Eof(#11)
  assert_raw_error("Invalid file number")
End Sub

Sub test_inputstr()
  Local f$ = file.TMPDIR$ + "/test_inputstr.tmp"

  ' Write some test data.
  Open f$ For Output As #1
  Print #1, "Hello World"
  Print #1, "Goodbye World"
  Close #1

  ' Simplest test.
  Open f$ For Input As #1
  Local s$ = Input$(28, #1)
  assert_string_equals("Hello World" + CRLF$ + "Goodbye World" + CRLF$, s$)
  Close #1

  ' Can't call on an unopened file.
  On Error Skip 1
  s$ = Input$(28, #1)
  assert_raw_error("File or device not open")

  ' NOTE you can call on file number #0, but I can't automatically test this.

  ' Can call on file number #10.
  Open f$ For Input As #10
  s$ = Input$(28, #10)
  assert_string_equals("Hello World" + CRLF$ + "Goodbye World" + CRLF$, s$)
  Close #10

  ' Can't call on file number #11.
  On Error Skip 1
  s$ = Input$(28, #11)
  assert_raw_error("Invalid file number")
End Sub

Sub test_kill() {
    Local f$ = file.TMPDIR$ + "/test_kill.tmp"

    Open f$ For Output As #1
    Print #1, "Hello World"
    Print #1, "Goodbye World"
    Close #1

    assert_true(file.exists%(f$))

    Kill f$

    assert_false(file.exists%(f$))
End Sub

Sub test_rename()
    Local f$ = file.TMPDIR$ + "/test_new.tmp"
    Local f_new$ = file.TMPDIR$ + "/test_new.tmp.new"
    Local s$

    Open f$ For Output As #1
    Print #1, "Hello World"
    Print #1, "Goodbye World"
    Close #1

    ' CMM2 will not RENAME over an existing file.
    If file.exists%(f_new$) Then Kill f_new$
    Rename f$ As f_new$

    assert_false(file.exists%(f$))

    Open f_new$ For Input As #1
    s$ = Input$(28, #1)
    assert_string_equals("Hello World" + CRLF$ + "Goodbye World" + CRLF$, s$)
    assert_true(Eof(#1))
    Close #1
End Sub

Sub test_loc()
    Local f$ = file.TMPDIR + "/test_loc.tmp"

    ' Start with an empty file.
    Open f$ For Output As #1
    Close #1

    ' Should be opened with r/w pointer at position 1.
    Open f$ For Random As #1
    assert_int_equals(1, Loc(#1))

    ' Printing 3 characters should move r/w pointer to position 4.
    Print #1, "foo";
    assert_int_equals(4, Loc(#1))
    Close #1

    ' Should be opened with r/w pointer at one past the end.
    Open f$ For Random As #1
    assert_int_equals(4, Loc(#1))
    Close #1
End Sub

Sub test_loc_errors()
  ' Can't call on an unopened file.
  On Error Skip 1
  Local i% = Loc(#1)
  assert_raw_error("File or device not open")

  ' Can't call on file number #0.
  On Error Skip 1
  i% = Loc(#0)
  assert_raw_error("Invalid file number")

  ' Can call on file number #10.
  Local f$ = file.TMPDIR + "/test_loc_errors.tmp"
  Open f$ For Output As #10
  Close #10
  Open f$ For Random As #10
  assert_int_equals(1, Loc(#10))
  Close #10

  ' Can't call on file number #11.
  On Error Skip 1
  i% = Loc(#11)
  assert_raw_error("Invalid file number")
End Sub

Sub test_lof()
    Local f$ = file.TMPDIR + "/test_lof.tmp"

    Open f$ For Output As #1
    Print #1, "Hello World";

    assert_int_equals(11, Lof(#1))
    Close #1

    Open f$ For Random As #1
    assert_int_equals(11, Lof(#1))
    Close #1
End Sub

Sub test_lof_errors()
  ' Can't call on an unopened file.
  On Error Skip 1
  Local i% = Lof(#1)
  assert_raw_error("File or device not open")

  ' Can't call on file number #0.
  On Error Skip 1
  i% = Lof(#0)
  assert_raw_error("Invalid file number")

  ' Can call on file number #10.
  Local f$ = file.TMPDIR + "/test_lof_errors.tmp"
  Open f$ For Output As #10
  Print #10, "Hello World";
  Close #10
  Open f$ For Random As #10
  assert_int_equals(11, Lof(#10))
  Close #10

  ' Can't call on file number #11.
  On Error Skip 1
  i% = Lof(#11)
  assert_raw_error("Invalid file number")
End Sub

Sub test_seek()
    Local f$ = file.TMPDIR + "/test_seek.tmp"
    Local s$

    Open f$ For Output As #1
    Print #1, "Hello World"
    Print #1, "Goodbye World"
    Close #1

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
  ' Can't call on an unopened file.
  On Error Skip 1
  Seek #1, 1
  assert_raw_error("File or device not open")

  ' Can't call on file number #0.
  On Error Skip 1
  Seek #0, 1
  assert_raw_error("Invalid file number")

  ' Can call on file number #10.
  Local f$ = file.TMPDIR + "/test_lof_errors.tmp"
  Open f$ For Output As #10
  Print #10, "Hello World"
  Print #10, "Goodbye World"
  Close #10
  Open f$ For Random As #10
  Seek #10, 7
  Local s$ = Input$(5, #10)
  assert_string_equals("World", s$)
  s$ = Input$(9, #10)
  assert_string_equals(CRLF$ + "Goodbye", s$)
  Close #10

  ' Can't call on file number #11.
  On Error Skip 1
  Seek #11, 1
  assert_raw_error("Invalid file number")
End Sub

Sub test_tilde_expansion()
  If Mm.Device$ <> "MMB4L" Then Exit Sub

  Local original_dir$ = Cwd$

  System "rm -Rf " + file.TMPDIR$ + "/test_tilde_expansion.dir"

  ' Test CHDIR.
  ChDir "~"
  assert_string_equals(Mm.Info$(EnvVar "HOME"), Cwd$)

  ' Use SYSTEM with 'realpath' to determine relative path from HOME to TMPDIR.
  Local s$
  System "realpath --relative-to=$HOME " + file.TMPDIR$, s$
  Local tmp_relative$ = "~/" + s$

  ChDir tmp_relative$
  assert_string_equals(file.TMPDIR$, Cwd$)

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
  Open file.TMPDIR$ + "/test_open_errors.txt" For Output As #1

  ' Cannot use a file number that is already open.
  On Error Skip 1
  Open file.TMPDIR$ + "/test_open_errors.txt.2" For Output As #1
  assert_raw_error("File or device already open")

  ' But can use it after it is closed.
  Close #1
  Open file.TMPDIR$ + "/test_open_errors.txt.2" For Output As #1
  Close #1

  ' Can't open file number #0.
  On Error Skip 1
  Open file.TMPDIR$ + "/test_open_errors.txt" For Output As #0
  assert_raw_error("Invalid file number")

  ' Can use file number #10.
  Open file.TMPDIR$ + "/test_open_errors.txt" For Output As #10
  Close #10

  ' Can't use file number #11.
  On Error Skip 1
  Open file.TMPDIR$ + "/test_open_errors.txt" For Output As #11
  assert_raw_error("Invalid file number")
End Sub
