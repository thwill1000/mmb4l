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
Const EXPECTED_FONT_HEIGHT% = 12
Const EXPECTED_FONT_WIDTH% = 8
If sys.is_platform%("mmb4l") Then
  Const EXPECTED_VERSION$ = "6000000"
ElseIf sys.is_platform%("mmb4w") Then
  Const EXPECTED_VERSION$ = "5.0703"
ElseIf sys.is_platform%("pm*") Then
  Const EXPECTED_VERSION$ = "5.0708"
Else
  Const EXPECTED_VERSION$ = "5.0702"
EndIf

add_test("test_arch")
add_test("test_cputime")
add_test("test_current")
add_test("test_device")
add_test("test_directory")
add_test("test_drive")
add_test("test_envvar")
add_test("test_errmsg")
add_test("test_errno")
add_test("test_exists")
add_test("test_exists_dir")
add_test("test_exists_file")
add_test("test_exists_symlink")
add_test("test_filesize_given_empty")
add_test("test_filesize_given_not_empty")
add_test("test_filesize_given_not_found")
add_test("test_filesize_given_directory")
add_test("test_font_address")
add_test("test_fontheight")
add_test("test_fontwidth")
add_test("test_hpos")
add_test("test_hres")
add_test("test_line")
add_test("test_option_base")
add_test("test_option_break")
add_test("test_option_case")
add_test("test_option_editor")
add_test("test_option_default")
add_test("test_option_explicit")
add_test("test_option_codepage")
add_test("test_option_fn_key")
add_test("test_option_resolution")
add_test("test_option_search_path")
add_test("test_option_serial")
add_test("test_option_tab")
add_test("test_path")
add_test("test_pid")
add_test("test_version")
add_test("test_vpos")
add_test("test_vres")

' On the PicoMite the tests should run 4 times:
'   Base 0, Drive A
'   Base 1, Drive A
'   Base 0, Drive B
'   Base 1, Drive B
If sys.is_platform%("pm*") Then
  If InStr(Mm.CmdLine$, "--base=1 --drive=b") Then
    run_tests()
  ElseIf InStr(Mm.CmdLine$, "--base=1") Then
    run_tests("--base=1", "--drive=b")
  ElseIf InStr(Mm.CmdLine$, "--drive=b") Then
    run_tests("--drive=b", "--base=1 --drive=b")
  Else
    run_tests("--base=1")
  EndIf
ElseIf InStr(Mm.CmdLine$, "--base=1") Then
  run_tests("")
Else
  run_tests("--base=1")
EndIf

End

Sub test_arch()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  Local expected_arch$
  System "uname -o -m", expected_arch$
  Select Case expected_arch$
    Case "aarch64 Android"   : expected_arch$ = "Android aarch64"
    Case "aarch64 GNU/Linux" : expected_arch$ = "Linux aarch64"
    Case "i686 GNU/Linux"    : expected_arch$ = "Linux i686"
    Case "x86_64 GNU/Linux"  : expected_arch$ = "Linux x86_64"
    Case "armv6l GNU/Linux"  : expected_arch$ = "Linux armv6l"
    Case "armv7l GNU/Linux"  : expected_arch$ = "Linux armv6l"
  End Select

  assert_string_equals(expected_arch$, Mm.Info$(Arch))
End Sub

Sub test_cputime()
  If Not sys.is_platform%("mmb4l") Then Exit Sub
  Local cputime% = Mm.Info(CpuTime)
  ' Not really anything that can be tested other than it returns.
  assert_true(cputime% > 0)
End Sub

Sub test_current()
  assert_string_equals(expected_path$() + "tst_mminfo.bas", Mm.Info(Current))
End Sub

Function expected_path$()
  If sys.is_platform%("mmb4l") Then
    Local out$
    System "echo $HOME", out$
    If Mm.Info(Exists Dir out$ + "/github/thwill1000") Then
      expected_path$ = out$ + "/github/thwill1000/mmb4l/tests/"
    Else
      expected_path$ = out$ + "/github/mmb4l/tests/"
    EndIf
  ElseIf sys.is_platform%("mmb4w") Then
    expected_path$ = "C:\home-thwill\git_sandbox\github\mmb4l\tests\"
  ElseIf sys.is_platform%("pm*") Then
    expected_path$ = "A:/"
  Else
    expected_path$ = "A:/MMB4L/tests/"
    If Cwd$ + "/" = UCase$(expected_path$) Then expected_path$ = UCase$(expected_path$)
  EndIf
End Function

Sub test_device()
  Local known% = 0
  known% = known% Or (Mm.Device$ = "Colour Maximite 2")
  known% = known% Or (Mm.Device$ = "Colour Maximite 2 G2")
  known% = known% Or (Mm.Device$ = "MMB4L")
  known% = known% Or (Mm.Device$ = "MMBasic for Windows")
  known% = known% Or (Mm.Device$ = "PicoMite")
  known% = known% Or (Mm.Device$ = "PicoMiteVGA")
  assert_true(known%)
  assert_string_equals(Mm.Info$(Device), Mm.Device$)
End Sub

Sub test_directory()
  If sys.is_platform%("pm*") Then Exit Sub

  Local expected_dir$
  Select Case Mm.Device$
    Case "MMB4L"               : System "pwd", expected_dir$
    Case "MMBasic for Windows" : System "cd", expected_dir$
    Case Else                  : expected_dir$ = Cwd$
  End Select

  Local actual$ = Mm.Info$(Directory)

  assert_string_equals(expected_dir$ + sys.SEPARATOR$(), actual$)
  assert_string_equals(Left$(actual$, Len(actual$) - 1), Cwd$)
End Sub

Sub test_drive()
  If Not sys.is_platform%("pm*") Then Exit Sub
  assert_string_equals(Choice(InStr(Mm.CmdLine$, "--drive=b"), "B:", "A:"), Mm.Info$(Drive))
End Sub

Sub test_envvar()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  Local expected_home$
  System "echo $HOME", expected_home$

  assert_string_equals(expected_home$, Mm.Info$(EnvVar "HOME"))
End Sub

Sub test_errmsg()
  On Error Skip 1
  Error "foo"
  assert_string_equals("foo", Right$(Mm.ErrMsg$, 3));
  assert_string_equals("foo", Right$(Mm.Info$(ErrMsg), 3));
  If Not sys.is_platform%("pm*") Then
    assert_string_equals("Error in line", Left$(Mm.ErrMsg$, 13));
    assert_string_equals("Error in line", Left$(Mm.Info(ErrMsg), 13));
  EndIf
End Sub

Sub test_errno()
  Local expected% = Choice(sys.is_platform%("mmb4l"), 256, 16)
  On Error Skip 1
  Error "foo"
  assert_int_equals(expected%, Mm.ErrNo);
  assert_int_equals(expected%, Mm.Info$(ErrNo));

  If Not sys.is_platform%("mmb4l") Then Exit Sub

  On Error Skip 1
  Error "foo", 201
  assert_string_equals("Error in line", Left$(Mm.ErrMsg$, 13));
  assert_string_equals("foo", Right$(Mm.ErrMsg$, 3));
  assert_int_equals(201, Mm.ErrNo);
End Sub

Sub test_exists()
  If Not sys.is_platform%("mmb4l", "mmb4w") Then Exit Sub

  ' Drives/root.
  Const A_EXISTS = Not sys.is_platform%("mmb4w")
  Const B_EXISTS = Not sys.is_platform%("mmb4w")
  assert_int_equals(A_EXISTS, Mm.Info(Exists "A:"))
  assert_int_equals(A_EXISTS, Mm.Info(Exists "A:/"))
  assert_int_equals(A_EXISTS, Mm.Info(Exists "A:\"))
  assert_int_equals(B_EXISTS, Mm.Info(Exists "B:"))
  assert_int_equals(B_EXISTS, Mm.Info(Exists "B:/"))
  assert_int_equals(B_EXISTS, Mm.Info(Exists "B:\"))
  assert_int_equals(1, Mm.Info(Exists "C:"))
  assert_int_equals(1, Mm.Info(Exists "C:/"))
  assert_int_equals(1, Mm.Info(Exists "C:\"))
  assert_int_equals(1, Mm.Info(Exists "/"))
  assert_int_equals(1, Mm.Info(Exists "\"))

  ' Files and directories.
  Const PATH = Left$(Mm.Info$(Path), Len(Mm.Info$(Path)) - 1) ' Trim the trailing slash.
  Const EXISTING_FILE = Mm.Info$(Path) + "tst_mminfo.bas"
  assert_int_equals(1, Mm.Info(Exists PATH))
  assert_int_equals(1, Mm.Info(Exists PATH + "/"))
  assert_int_equals(1, Mm.Info(Exists PATH + "\"))
  assert_int_equals(1, Mm.Info(Exists PATH + "/tst_mminfo.bas"))
  assert_int_equals(1, Mm.Info(Exists PATH + "\tst_mminfo.bas"))
  assert_int_equals(0, Mm.Info(Exists PATH + "/no_such_file"))
  assert_int_equals(0, Mm.Info(Exists PATH + "\no_such_file"))

  ' Odd and ends.
  assert_int_equals(1, Mm.Info(Exists "."))
  assert_int_equals(1, Mm.Info(Exists ".."))
  assert_int_equals(Not sys.is_platform%("mmb4w"), Mm.Info(Exists ""))

  ' Symbolic links.
  If sys.is_platform%("mmb4l") Then
    MkDir TMPDIR$

    System "ln -s " + EXISTING_FILE + " " + TMPDIR$ + "/file_link"
    System "ln -s " + PATH + " " + TMPDIR$ + "/dir_link"

    assert_int_equals(1, Mm.Info(Exists TMPDIR$ + "/file_link"))
    assert_int_equals(1, Mm.Info(Exists TMPDIR$ + "/dir_link"))
  EndIf
End Sub

Sub test_exists_dir()
  MkDir TMPDIR$

  ' Drives/root.
  Const A_EXISTS = Not sys.is_platform%("mmb4w")
  Const B_EXISTS = Not sys.is_platform%("mmb4w")
  assert_int_equals(A_EXISTS, Mm.Info(Exists Dir "A:"))
  assert_int_equals(A_EXISTS, Mm.Info(Exists Dir "A:/"))
  assert_int_equals(A_EXISTS, Mm.Info(Exists Dir "A:\"))
  assert_int_equals(B_EXISTS, Mm.Info(Exists Dir "B:"))
  assert_int_equals(B_EXISTS, Mm.Info(Exists Dir "B:/"))
  assert_int_equals(B_EXISTS, Mm.Info(Exists Dir "B:\"))
  If sys.is_platform%("pm*") Then
    Local dummy%
    On Error Skip
    dummy% = Mm.Info(Exists Dir "C:")
    assert_raw_error("Invalid disk")
    On Error Skip
    dummy% = Mm.Info(Exists Dir "C:/")
    assert_raw_error("Invalid disk")
    On Error Skip
    dummy% = Mm.Info(Exists Dir "C:\")
    assert_raw_error("Invalid disk")
  Else
    assert_int_equals(1, Mm.Info(Exists Dir "C:"))
    assert_int_equals(1, Mm.Info(Exists Dir "C:/"))
    assert_int_equals(1, Mm.Info(Exists Dir "C:\"))
  EndIf
  assert_int_equals(1, Mm.Info(Exists Dir "/"))
  assert_int_equals(1, Mm.Info(Exists Dir "\"))

  ' Files and directories.
  Const PATH = Left$(Mm.Info$(Path), Len(Mm.Info$(Path)) - 1) ' Trim the trailing slash.
  Const EXISTING_FILE = Mm.Info$(Path) + "tst_mminfo.bas"
  assert_int_equals(1, Mm.Info(Exists Dir PATH))
  assert_int_equals(1, Mm.Info(Exists Dir PATH + "/"))
  assert_int_equals(1, Mm.Info(Exists Dir PATH + "\"))
  assert_int_equals(0, Mm.Info(Exists Dir PATH + "/tst_mminfo.bas"))
  assert_int_equals(0, Mm.Info(Exists Dir PATH + "\tst_mminfo.bas"))
  assert_int_equals(0, Mm.Info(Exists Dir PATH + "/no_such_file"))
  assert_int_equals(0, Mm.Info(Exists Dir PATH + "\no_such_file"))

  ' Odd and ends.
  assert_int_equals(1, Mm.Info(Exists Dir "."))
  assert_int_equals(1, Mm.Info(Exists Dir ".."))
  assert_int_equals(Not sys.is_platform%("mmb4w"), Mm.Info(Exists Dir ""))

  ' Symbolic links.
  If sys.is_platform%("mmb4l") Then
    System "ln -s " + EXISTING_FILE + " " + TMPDIR$ + "/file_link"
    System "ln -s " + PATH + " " + TMPDIR$ + "/dir_link"

    assert_int_equals(0, Mm.Info(Exists Dir TMPDIR$ + "/file_link"))
    assert_int_equals(1, Mm.Info(Exists Dir TMPDIR$ + "/dir_link"))
  EndIf

  ' Test for odd bug encountered on PicoMite when not in root dir.
  Const OLD_DIR = Cwd$
  ChDir TMPDIR$
  assert_int_equals(A_EXISTS, Mm.Info(Exists Dir "A:"))
  assert_int_equals(A_EXISTS, Mm.Info(Exists Dir "A:/"))
  assert_int_equals(A_EXISTS, Mm.Info(Exists Dir "A:\"))
  assert_int_equals(B_EXISTS, Mm.Info(Exists Dir "B:"))
  assert_int_equals(B_EXISTS, Mm.Info(Exists Dir "B:/"))
  assert_int_equals(B_EXISTS, Mm.Info(Exists Dir "B:\"))
  assert_int_equals(1, Mm.Info(Exists Dir "."))
  assert_int_equals(1, Mm.Info(Exists Dir ".."))
  assert_int_equals(Not sys.is_platform%("mmb4w"), Mm.Info(Exists Dir ""))
  ChDir OLD_DIR
End Sub

Sub test_exists_file()
  ' Drives/root.
  assert_int_equals(0, Mm.Info(Exists File "A:"))
  assert_int_equals(0, Mm.Info(Exists File "A:/"))
  assert_int_equals(0, Mm.Info(Exists File "A:\"))
  assert_int_equals(0, Mm.Info(Exists File "B:"))
  assert_int_equals(0, Mm.Info(Exists File "B:/"))
  assert_int_equals(0, Mm.Info(Exists File "B:\"))
  If sys.is_platform%("pm*") Then
    Local dummy%
    On Error Skip
    dummy% = Mm.Info(Exists Dir "C:")
    assert_raw_error("Invalid disk")
    On Error Skip
    dummy% = Mm.Info(Exists Dir "C:/")
    assert_raw_error("Invalid disk")
    On Error Skip
    dummy% = Mm.Info(Exists Dir "C:\")
    assert_raw_error("Invalid disk")
  Else
    assert_int_equals(0, Mm.Info(Exists File "C:"))
    assert_int_equals(0, Mm.Info(Exists File "C:/"))
    assert_int_equals(0, Mm.Info(Exists File "C:\"))
  EndIf
  assert_int_equals(0, Mm.Info(Exists File "/"))
  assert_int_equals(0, Mm.Info(Exists File "\"))

  ' Files and directories.
  Const PATH = Left$(Mm.Info$(Path), Len(Mm.Info$(Path)) - 1) ' Trim the trailing slash.
  Const EXISTING_FILE = Mm.Info$(Path) + "tst_mminfo.bas"
  assert_int_equals(0, Mm.Info(Exists File PATH))
  assert_int_equals(0, Mm.Info(Exists File PATH + "/"))
  assert_int_equals(0, Mm.Info(Exists File PATH + "\"))
  assert_int_equals(1, Mm.Info(Exists File PATH + "/tst_mminfo.bas"))
  assert_int_equals(1, Mm.Info(Exists File PATH + "\tst_mminfo.bas"))
  assert_int_equals(0, Mm.Info(Exists File PATH + "/no_such_file"))
  assert_int_equals(0, Mm.Info(Exists File PATH + "\no_such_file"))

  ' Odd and ends.
  assert_int_equals(0, Mm.Info(Exists File "."))
  assert_int_equals(0, Mm.Info(Exists File ".."))
  assert_int_equals(0, Mm.Info(Exists File ""))

  ' Symbolic links.
  If sys.is_platform%("mmb4l") Then
    MkDir TMPDIR$

    System "ln -s " + EXISTING_FILE + " " + TMPDIR$ + "/file_link"
    System "ln -s " + PATH + " " + TMPDIR$ + "/dir_link"

    assert_int_equals(1, Mm.Info(Exists File TMPDIR$ + "/file_link"))
    assert_int_equals(0, Mm.Info(Exists File TMPDIR$ + "/dir_link"))
  EndIf
End Sub

Sub test_exists_symlink()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  ' Drives/root.
  assert_int_equals(0, Mm.Info(Exists SymLink "A:"))
  assert_int_equals(0, Mm.Info(Exists SymLink "A:/"))
  assert_int_equals(0, Mm.Info(Exists SymLink "A:\"))
  assert_int_equals(0, Mm.Info(Exists SymLink "B:"))
  assert_int_equals(0, Mm.Info(Exists SymLink "B:/"))
  assert_int_equals(0, Mm.Info(Exists SymLink "B:\"))
  assert_int_equals(0, Mm.Info(Exists SymLink "C:"))
  assert_int_equals(0, Mm.Info(Exists SymLink "C:/"))
  assert_int_equals(0, Mm.Info(Exists SymLink "C:\"))
  assert_int_equals(0, Mm.Info(Exists SymLink "/"))
  assert_int_equals(0, Mm.Info(Exists SymLink "\"))

  ' Files and directories.
  Const PATH = Left$(Mm.Info$(Path), Len(Mm.Info$(Path)) - 1) ' Trim the trailing slash.
  Const EXISTING_FILE = Mm.Info$(Path) + "tst_mminfo.bas"
  assert_int_equals(0, Mm.Info(Exists SymLink PATH))
  assert_int_equals(0, Mm.Info(Exists SymLink PATH + "/"))
  assert_int_equals(0, Mm.Info(Exists SymLink PATH + "\"))
  assert_int_equals(0, Mm.Info(Exists SymLink PATH + "/tst_mminfo.bas"))
  assert_int_equals(0, Mm.Info(Exists SymLink PATH + "\tst_mminfo.bas"))
  assert_int_equals(0, Mm.Info(Exists SymLink PATH + "/no_such_file"))
  assert_int_equals(0, Mm.Info(Exists SymLink PATH + "\no_such_file"))

  ' Odd and ends.
  assert_int_equals(0, Mm.Info(Exists SymLink "."))
  assert_int_equals(0, Mm.Info(Exists SymLink ".."))
  assert_int_equals(0, Mm.Info(Exists SymLink ""))

  ' Symbolic links.
  MkDir TMPDIR$
  System "ln -s " + EXISTING_FILE + " " + TMPDIR$ + "/file_link"
  System "ln -s " + PATH + " " + TMPDIR$ + "/dir_link"

  assert_int_equals(1, Mm.Info(Exists SymLink TMPDIR$ + "/file_link"))
  assert_int_equals(1, Mm.Info(Exists SymLink TMPDIR$ + "/dir_link"))
End Sub

Sub test_filesize_given_empty()
  MkDir TMPDIR$
  Const f$ = TMPDIR$ + "/test_filesize.txt"
  Open f$ For Output As #1
  Close #1
  assert_int_equals(0, Mm.Info(FileSize f$))
End Sub

Sub test_filesize_given_not_empty()
  MkDir TMPDIR$
  Const f$ = TMPDIR$ + "/test_filesize.txt"
  Open f$ For Output As #1
  Print #1, "7 bytes";
  Close #1
  assert_int_equals(7, Mm.Info(FileSize f$))
End Sub

Sub test_filesize_given_not_found()
  MkDir TMPDIR$
  assert_int_equals(-1, Mm.Info(FileSize TMPDIR$ + "/test_filesize.txt"))
End Sub

Sub test_filesize_given_directory()
  MkDir TMPDIR$

  If sys.is_platform%("mmb4l", "pm*") Then
    expect_filesize_is_dir(Cwd$)
    expect_filesize_is_dir(Mm.Info$(Path))
    expect_filesize_is_dir("/")
    expect_filesize_is_dir("\")
    expect_filesize_is_dir("A:")
    expect_filesize_is_dir("A:/")
    expect_filesize_is_dir("A:\")
    expect_filesize_is_dir("B:")
    expect_filesize_is_dir("B:/")
    expect_filesize_is_dir("B:\")
    If sys.is_platform%("mmb4l") Then
      expect_filesize_is_dir("C:")
      expect_filesize_is_dir("C:/")
      expect_filesize_is_dir("C:\")
    Else
      expect_filesize_invalid_disk("C:")
      expect_filesize_invalid_disk("C:/")
      expect_filesize_invalid_disk("C:\")
    EndIf
    expect_filesize_is_dir(str.replace$(TMPDIR$, "/", "\"))
    expect_filesize_is_dir(str.replace$(TMPDIR$, "\", "/"))
    expect_filesize_is_dir(".")
    expect_filesize_is_dir("..")
  ElseIf sys.is_platform%("cmm2*") Then
    expect_filesize_is_dir(Cwd$)
    expect_filesize_invalid_file(Mm.Info$(Path))
    expect_filesize_invalid_file("/")
    expect_filesize_invalid_file("\")
    expect_filesize_invalid_file("A:")
    expect_filesize_invalid_file("A:/")
    expect_filesize_invalid_file("A:\")
    expect_filesize_invalid_file("B:")
    expect_filesize_invalid_file("B:/")
    expect_filesize_invalid_file("B:\")
    expect_filesize_invalid_file("C:")
    expect_filesize_invalid_file("C:/")
    expect_filesize_invalid_file("C:\")
    expect_filesize_is_dir(str.replace$(TMPDIR$, "/", "\"))
    expect_filesize_is_dir(str.replace$(TMPDIR$, "\", "/"))
    expect_filesize_not_found(".")
    expect_filesize_not_found("..")
  ElseIf sys.is_platform%("mmb4w") Then
    expect_filesize_is_dir(Cwd$)
    expect_filesize_is_dir(Mm.Info$(Path))
    expect_filesize_not_found("/")
    expect_filesize_not_found("\")
    expect_filesize_not_found("A:")
    expect_filesize_not_found("A:/")
    expect_filesize_not_found("A:\")
    expect_filesize_not_found("B:")
    expect_filesize_not_found("B:/")
    expect_filesize_not_found("B:\")
    expect_filesize_is_dir("C:")
    expect_filesize_is_dir("C:/")
    expect_filesize_is_dir("C:\")
    expect_filesize_is_dir(str.replace$(TMPDIR$, "/", "\"))
    expect_filesize_is_dir(str.replace$(TMPDIR$, "\", "/"))
    expect_filesize_is_dir(".")
    expect_filesize_is_dir("..")
  EndIf

  ' Test for odd bug encountered on PicoMite when not in root dir.
  If Not sys.is_platform%("cmm2*", "mmb4w") Then
    Const OLD_DIR = Cwd$
    ChDir TMPDIR$
    expect_filesize_is_dir(Cwd$)
    expect_filesize_is_dir(Mm.Info$(Path))
    expect_filesize_is_dir("/")
    expect_filesize_is_dir("\")
    expect_filesize_is_dir("A:")
    expect_filesize_is_dir("A:/")
    expect_filesize_is_dir("A:\")
    expect_filesize_is_dir("B:")
    expect_filesize_is_dir("B:/")
    expect_filesize_is_dir("B:\")
    expect_filesize_is_dir(str.replace$(TMPDIR$, "/", "\"))
    expect_filesize_is_dir(str.replace$(TMPDIR$, "\", "/"))
    expect_filesize_is_dir(".")
    expect_filesize_is_dir("..")
    expect_filesize_is_dir("")
    ChDir OLD_DIR
  EndIf
End Sub

Sub expect_filesize_is_dir(f$)
  assert_int_equals(-2, Mm.Info(FileSize f$))
End Sub

Sub expect_filesize_not_found(f$)
  assert_int_equals(-1, Mm.Info(FileSize f$))
End Sub

Sub expect_filesize_invalid_file(f$)
  On Error Skip 1
  Local size% = Mm.Info(FileSize f$)
  assert_raw_error("Invalid file specification")
  assert_int_equals(0, size%)
  On Error Clear
End Sub

Sub expect_filesize_invalid_disk(f$)
  On Error Skip 1
  Local size% = Mm.Info(FileSize f$)
  assert_raw_error("Invalid disk")
  assert_int_equals(0, size%)
  On Error Clear
End Sub

Sub test_font_address()
  If sys.is_platform%("mmb4l") Then Exit Sub

  ' First byte at the "font address" is the font width.
  assert_int_equals(8, Peek(Byte(Mm.Info(Font Address 1))))
  assert_int_equals(12, Peek(Byte(Mm.Info(Font Address 2))))
  assert_int_equals(16, Peek(Byte(Mm.Info(Font Address 3))))
  ' assert_int_equals(8, Peek(Byte(Mm.Info(Font Address 4))))
  assert_int_equals(24, Peek(Byte(Mm.Info(Font Address 5))))
  ' assert_int_equals(8, Peek(Byte(Mm.Info(Font Address 6))))
  assert_int_equals(6, Peek(Byte(Mm.Info(Font Address 7))))
End Sub

Sub test_fontheight()
  assert_int_equals(EXPECTED_FONT_HEIGHT%, Mm.Info(FontHeight))

  If sys.is_platform%("mmb4l") Then
    ' Expect error if there is a space between FONT and HEIGHT.
    On Error Skip
    Local i% = Mm.Info(Font Height)
    assert_raw_error("Unknown argument")
  Else
    ' Incorrectly reports result of MM.INFO(FONT).
    assert_int_equals(1, Mm.Info(Font Height))
  EndIf
End Sub

Sub test_fontwidth()
  assert_int_equals(EXPECTED_FONT_WIDTH%, Mm.Info(FontWidth))

  If sys.is_platform%("mmb4l") Then
    ' Expect error if there is a space between FONT and WIDTH.
    On Error Skip
    Local i% = Mm.Info(Font Width)
    assert_raw_error("Unknown argument")
  Else
    ' Incorrectly reports result of MM.INFO(FONT).
    assert_int_equals(1, Mm.Info(Font Width))
  EndIf
End Sub

Sub test_hpos()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  Option Resolution Character
  Local old_x%, old_y%
  Console GetCursor old_x%, old_y%
  Console SetCursor 5, 10

  Local actual% = Mm.Info(HPos)
  assert_int_equals(5, actual%)

  Option Resolution Pixel
  assert_int_equals(5 * EXPECTED_FONT_WIDTH%, Mm.Info(HPos))

  Option Resolution Character
  Console SetCursor old_x%, old_y%
End Sub

Sub test_hres()
  If sys.is_platform%("mmb4l") Then
    Option Resolution Character
    Local actual% = Mm.Info(HRes)
    Local out$
    System "tput cols", out$
    Local expected_hres% = Val(out$)
    assert_int_equals(expected_hres%, actual%)
    assert_int_equals(actual%, Mm.HRes)

    Option Resolution Pixel
    assert_int_equals(actual% * EXPECTED_FONT_WIDTH%, Mm.Info(HRes))
    assert_int_equals(actual% * EXPECTED_FONT_WIDTH%, Mm.HRes)
  EndIf
End Sub

Sub test_line()
  Const line$ = Mm.Info$(Line)
  If sys.is_platform%("mmb4l") Then
    assert_int_equals(633, Val(Field$(line$, 1, ",")))
    assert_string_equals(Mm.Info$(Current), Field$(line$, 2, ","))
  Else
    ' Line number refers to the transpiled file.
    assert_int_equals(2151, Val(Field$(line$, 1, ",")))
  EndIf
End Sub

Sub test_option_base()
  assert_int_equals(InStr(Mm.CmdLine$, "--base=1") > 0, Mm.Info(Option Base))
End Sub

Sub test_option_break()
  assert_int_equals(3, Mm.Info(Option Break))

  Option Break 4
  assert_int_equals(4, Mm.Info(Option Break));

  Option Break 3
  assert_int_equals(3, Mm.Info(Option Break))
End Sub

Sub test_option_case()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  assert_string_equals("Title", Mm.Info(Option Case))

  Option Case Lower
  assert_string_equals("Lower", Mm.Info(Option Case))

  Option Case Upper
  assert_string_equals("Upper", Mm.Info(Option Case))

  Option Case Title
  assert_string_equals("Title", Mm.Info(Option Case))
End Sub

Sub test_option_editor()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  Local original$ = Mm.Info(Option Editor)

  Option Editor Atom
  assert_string_equals("Atom", Mm.Info(Option Editor))

  Option Editor Code
  assert_string_equals("VSCode", Mm.Info(Option Editor))

  Option Editor Default
  assert_string_equals("Nano", Mm.Info(Option Editor))

  Option Editor Geany
  assert_string_equals("Geany", Mm.Info(Option Editor))

  Option Editor Gedit
  assert_string_equals("Gedit", Mm.Info(Option Editor))

  Option Editor Leafpad
  assert_string_equals("Leafpad", Mm.Info(Option Editor))

  Option Editor Nano
  assert_string_equals("Nano", Mm.Info(Option Editor))

  Option Editor Sublime
  assert_string_equals("Sublime", Mm.Info(Option Editor))

  Option Editor Vi
  assert_string_equals("Vi", Mm.Info(Option Editor))

  Option Editor Vim
  assert_string_equals("Vim", Mm.Info(Option Editor))

  Option Editor VSCode
  assert_string_equals("VSCode", Mm.Info(Option Editor))

  Option Editor Xed
  assert_string_equals("Xed", Mm.Info(Option Editor))

  Option Editor "my custom editor ${file} ${line}"
  assert_string_equals("my custom editor ${file} ${line}", Mm.Info(Option Editor))

  Option Editor original$
  assert_string_equals(original$, Mm.Info(Option Editor))
End Sub

Sub test_option_default()
  assert_string_equals("None", Mm.Info(Option Default))

  Option Default Integer
  assert_string_equals("Integer", Mm.Info(Option Default))

  Option Default Float
  assert_string_equals("Float", Mm.Info(Option Default))

  Option Default String
  assert_string_equals("String", Mm.Info(Option Default))

  Option Default None
  assert_string_equals("None", Mm.Info(Option Default))
End Sub

Sub test_option_explicit()
  assert_string_equals("On", Mm.Info(Option Explicit))

  If Not sys.is_platform%("mmb4l") Then Exit Sub

  ' Only MMB4L supports the optional boolean flag.

  Option Explicit Off
  assert_string_equals("Off", Mm.Info(Option Explicit))

  Option Explicit True
  assert_string_equals("On", Mm.Info(Option Explicit))

  Option Explicit False
  assert_string_equals("Off", Mm.Info(Option Explicit))

  Option Explicit On
  assert_string_equals("On", Mm.Info(Option Explicit))

  Option Explicit False
  Option Explicit
  assert_string_equals("On", Mm.Info(Option Explicit))
End Sub

Sub test_option_codepage()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  assert_string_equals("None", Mm.Info(Option CodePage))

  Option CodePage "CMM2"
  assert_string_equals("CMM2", Mm.Info(Option CodePage))

  Option CodePage "CP437"
  assert_string_equals("CP437", Mm.Info(Option CodePage))

  Option CodePage "CP1252"
  assert_string_equals("CP1252", Mm.Info(Option CodePage))

  Option CodePage "MMB4L"
  assert_string_equals("MMB4L", Mm.Info(Option CodePage))

  Option CodePage "None"
  assert_string_equals("None", Mm.Info(Option CodePage))

  ' Should also work without quotes
  Option CodePage CMM2
  assert_string_equals("CMM2", Mm.Info(Option CodePage))

  Option CodePage CP437
  assert_string_equals("CP437", Mm.Info(Option CodePage))

  Option CodePage CP1252
  assert_string_equals("CP1252", Mm.Info(Option CodePage))

  Option CodePage MMB4L
  assert_string_equals("MMB4L", Mm.Info(Option CodePage))

  Option CodePage None
  assert_string_equals("None", Mm.Info(Option CodePage))

End Sub

Sub test_option_fn_key()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  MkDir TMPDIR$

  ' Save current options and switch to defaults.
  Option Save TMPDIR$ + "/mmbasic.options.bak"
  Option Reset All

  Const CRLF$ = Chr$(&h0D) + Chr$(&h0A)
  assert_string_equals("FILES" + CRLF$, Mm.Info(Option F1))
  assert_string_equals("RUN"   + CRLF$, Mm.Info(Option F2))
  assert_string_equals("LIST"  + CRLF$, Mm.Info(Option F3))
  assert_string_equals("EDIT"  + CRLF$, Mm.Info(Option F4))

  Const QUOTES$ = Chr$(&h22) + Chr$(&h22) + Chr$(&h82)
  assert_string_equals("AUTOSAVE "       + QUOTES$, Mm.Info(Option F5))
  assert_string_equals("XMODEM RECEIVE " + QUOTES$, Mm.Info(Option F6))
  assert_string_equals("XMODEM SEND "    + QUOTES$, Mm.Info(Option F7))
  assert_string_equals("EDIT "           + QUOTES$, Mm.Info(Option F8))
  assert_string_equals("LIST "           + QUOTES$, Mm.Info(Option F9))
  assert_string_equals("RUN "            + QUOTES$, Mm.Info(Option F10))
  assert_string_equals("", Mm.Info(Option F11))
  assert_string_equals("", Mm.Info(Option F12))

  Option F11 "FOO " + CRLF$
  assert_string_equals("FOO " + CRLF$,  Mm.Info(Option F11))

  Option F12 "BAR " + QUOTES$
  assert_string_equals("BAR " + QUOTES$,  Mm.Info(Option F12))

  ' Restore old options.
  Option Reset All
  Option Load TMPDIR$ + "/mmbasic.options.bak"
End Sub

Sub test_option_resolution()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  Option Resolution Pixel
  assert_string_equals("Pixel", Mm.Info(Option Resolution))

  Option Resolution Character
  assert_string_equals("Character", Mm.Info(Option Resolution))
End Sub

Sub test_option_search_path()
  If sys.is_platform%("cmm2*", "pm*") Then Exit Sub

  Local original$ = Mm.Info$(Option Search Path)

  ' Set the SEARCH PATH to a directory that exists.
  Local path$ = Mm.Info$(Path)
  Option Search Path path$
  If sys.is_platform%("mmb4l") Then
    ' Note we the trailing '/' will have been trimmed from the Search Path.
    assert_string_equals(Left$(path$, Len(path$) - 1), Mm.Info$(Option Search Path))
  Else
    assert_string_equals(path$, Mm.Info$(Option Search Path))
  EndIf

  ' Set the SEARCH PATH to a file that exists.
  path$ = Mm.Info$(Current)
  On Error Skip
  Option Search Path path$
  Select Case Mm.Device$
    Case "MMB4L" :               assert_raw_error("Not a directory")
    Case "MMBasic for Windows" : assert_raw_error("Directory " + path$ + " does not exist")
    Case Else :                  assert_raw_error("Directory not found")
  End Select

  ' Set the SEARCH PATH to a path that does not exist.
  On Error Skip
  Option Search Path "/does/not/exist"
  Select Case Mm.Device$
    Case "MMB4L" :               assert_raw_error("No such file or directory")
    Case "MMBasic for Windows" : assert_raw_error("Directory \does\not\exist does not exist")
    Case Else :                  assert_raw_error("Directory not found")
  End Select

  ' Set the SEARCH PATH to a path that is too long.
  On Error Skip
  Option Search Path String$(255, "a")
  Select Case Mm.Device$
    Case "MMB4L" :               assert_raw_error("File name too long")
    Case "MMBasic for Windows" : assert_raw_error("Pathname too long")
    Case Else :                  assert_raw_error("Pathname too long")
  End Select

  ' Unset the SEARCH PATH.
  Option Search Path ""
  assert_string_equals("", Mm.Info$(Option Search Path))

  Option Search Path original$
  assert_string_equals(original$, Mm.Info$(Option Search Path))
End Sub

Sub test_option_serial()
  If sys.is_platform%("pm*") Then Exit Sub

  If sys.is_platform%("mmb4l") Then

    assert_string_equals("Serial", Mm.Info(Option Console))

    ' OPTION CONSOLE is a dummy command for MMB4L and the value of the option
    ' always remains SERIAL.

    Option Console Both
    assert_string_equals("Serial", Mm.Info(Option Console))

    Option Console Screen
    assert_string_equals("Serial", Mm.Info(Option Console))

    Option Console Serial
    assert_string_equals("Serial", Mm.Info(Option Console))

  Else

    ' Note that on "MMBasic for Windows" the default is OPTION CONSOLE SCREEN,
    ' but the unit-test framework sets OPTION CONSOLE BOTH.
    assert_string_equals("Both", Mm.Info(Option Console))

    Option Console Serial
    assert_string_equals("Serial", Mm.Info(Option Console))

    Option Console Screen
    assert_string_equals("Screen", Mm.Info(Option Console))

    Option Console Both
    assert_string_equals("Both", Mm.Info(Option Console))

  EndIf
End Sub

Sub test_option_tab()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  assert_int_equals(4, Mm.Info(Option Tab))

  Option Tab 2
  assert_int_equals(2, Mm.Info(Option Tab))

  Option Tab 8
  assert_int_equals(8, Mm.Info(Option Tab))

  Option Tab 4
  assert_int_equals(4, Mm.Info(Option Tab))
End Sub

Sub test_path()
  assert_string_equals(expected_path$(), MM.Info$(Path))
End Sub

Sub test_pid()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  Local out$
  System "echo $PPID", out$
  assert_int_equals(Val(out$), Mm.Info(PID))
End Sub

Sub test_vpos()
  If Not sys.is_platform%("mmb4l") Then Exit Sub

  Option Resolution Character
  Local old_x%, old_y%
  Console GetCursor old_x%, old_y%
  Console SetCursor 5, 10

  Local actual% = Mm.Info(VPos)
  assert_int_equals(10, actual%)

  Option Resolution Pixel
  assert_int_equals(10 * EXPECTED_FONT_HEIGHT%, Mm.Info(VPos))

  Option Resolution Character
  Console SetCursor old_x%, old_y%
End Sub

Sub test_vres()
  If sys.is_platform%("mmb4l") Then
    Option Resolution Character
    Local actual% = Mm.Info(VRes)
    Local out$
    System "tput lines", out$
    Local expected_vres% = Val(out$)
    assert_int_equals(expected_vres%, actual%)
    assert_int_equals(actual%, Mm.VRes)

    Option Resolution Pixel
    assert_int_equals(actual% * Mm.Info(FontHeight), Mm.Info(VRes))
    assert_int_equals(actual% * Mm.Info(FontHeight), Mm.VRes)
  EndIf
End Sub

Sub test_version()
  assert_string_equals(EXPECTED_VERSION$, Left$(Str$(Mm.Info(Version)), Len(EXPECTED_VERSION$)))

  If sys.is_platform%("mmb4l") Then
    assert_int_equals(0, Mm.Info(Version Major))
    assert_int_equals(6, Mm.Info(Version Minor))
    assert_int_equals(0, Mm.Info(Version Micro))
    assert_int_equals(0, Mm.Info(Version Build))
  End If
End Sub
