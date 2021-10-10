' Copyright (c) 2020-2021 Thomas Hugo Williams
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
Const CRLF$ = Chr$(13) + Chr$(10)
Const TMP_DIR$ = "/tmp"

add_test("test_chdir_mkdir_rmdir")
add_test("test_copy")
add_test("test_dir")
add_test("test_kill")
add_test("test_loc")
add_test("test_lof")
add_test("test_rename")
add_test("test_seek")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
End Sub

Sub teardown_test()
End Sub

Sub test_chdir_mkdir_rmdir()
    Local current_dir$ = Mm.Info$(Directory))
    Local new_dir$ = "test_chdir_mkdir_rmdir.tmpdir"
    If file.exists%(TMP_DIR$ + "/" + new_dir$) Then RmDir(TMP_DIR$ + "/" + new_dir$)

    ChDir TMP_DIR$
    MkDir new_dir$
    ChDir new_dir$

    Local expected$ = TMP_DIR$ + "/" + new_dir$ + "/"
    If Mm.Device$ <> "MMB4L" Then expected$ = "A:" + UCase$(expected$)
    assert_string_equals(expected$, Mm.Info$(Directory))

    ChDir ".."
    RmDir new_dir$

    assert_false(file.exists%(new_dir$))

    ChDir current_dir$
    assert_string_equals(current_dir$, Mm.Info$(Directory))
End Sub

Sub test_copy()
    Local f$ = TMP_DIR$ + "/test_copy.tmp"
    Local f_copy$ = TMP_DIR$ + "/test_copy.tmp.copy"
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
  Local tst_dir$ = TMP_DIR$ + "/test_dir.tmpdir"
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

Sub test_kill() {
    Local f$ = TMP_DIR$ + "/test_kill.tmp"

    Open f$ For Output As #1
    Print #1, "Hello World"
    Print #1, "Goodbye World"
    Close #1

    assert_true(file.exists%(f$))

    Kill f$

    assert_false(file.exists%(f$))
End Sub

Sub test_rename()
    Local f$ = TMP_DIR$ + "/test_new.tmp"
    Local f_new$ = TMP_DIR$ + "/test_new.tmp.new"
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
    Local f$ = TMP_DIR + "/test_loc.tmp"

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

Sub test_lof()
    Local f$ = TMP_DIR + "/test_lof.tmp"

    Open f$ For Output As #1
    Print #1, "Hello World";

    assert_int_equals(11, Lof(#1))
    Close #1

    Open f$ For Random As #1
    assert_int_equals(11, Lof(#1))
    Close #1
End Sub

Sub test_seek()
    Local f$ = TMP_DIR + "/test_seek.tmp"
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
