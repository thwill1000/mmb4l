#!/usr/local/bin/mmbasic -i

' Copyright (c) 2021-2022 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMB4L 2022.01.00

Option Base 0
Option Default None
Option Explicit On

#Include "../basic-src/splib/system.inc"
#Include "../basic-src/splib/array.inc"
#Include "../basic-src/splib/map.inc"
#Include "../basic-src/splib/string.inc"
#Include "../basic-src/splib/inifile.inc"
#Include "../basic-src/splib/file.inc"
#Include "console.inc"
#Include "spsh.inc"
#Include "gonzo.inc"

Option CodePage "CMM2"

Const INI_FILE$ = file.PROG_DIR$ + "/gonzo.ini"

Dim cmd$
Dim argc%
Dim argv$(9)

main()
End

Sub main()
  If Not gonzo.load_inifile%(INI_FILE$) Then
    con.errorln(sys.err$)
    Exit Sub
  EndIf

  sys.override_break()

  Local cmd_line$ = str.trim$(Mm.CmdLine$)
  If cmd_line$ <> "" Then parse(cmd_line$)

  If cmd$ = "" Then
    con.cls()
    con.foreground("yellow")
    con.println("Welcome to gonzo v" + sys.VERSION$)
  EndIf

  gonzo.connect(cmd$ = "");

  If cmd$ <> "" Then
    gonzo.do_command(cmd$, argc%, argv$())

    If Mm.Device$ <> "MMB4L" Then
      ' Move cursor up one line on both VGA and Serial Console.
      Option Console Screen
      Local y% = Mm.Info(VPos) - Mm.Info(FontHeight)
      Print @(0, y%);
      Option Console Serial
      Print Chr$(27) "[A";
      Option Console Both
    EndIf

    sys.restore_break()
    End
  EndIf

  Local history%(array.new%(128)) ' 1K

  Do
    sys.break_flag% = 0
    con.foreground("yellow")
    con.print("gonzo$ ")
    cmd_line$ = con.readln$("", history%())
    If sys.break_flag% Then con.println() : Exit Do
    parse(cmd_line$)
    ' con.foreground("default")
    If cmd$ = "" Then Continue Do
  Loop Until gonzo.do_command%(cmd$, argc%, argv$())

  If Not gonzo.save_inifile%(INI_FILE$) Then con.errorln(sys.err$)
  sys.restore_break()
  If Pos > 1 Then con.cursor_previous() ' So that we don't get an extra newline when program ends.
End Sub

Sub parse(cmd_line$)
  cmd$ = str.next_token$(cmd_line$)
  If cmd$ = sys.NO_DATA$ Then cmd$ = ""
  If Left$(cmd$, 1) = "*" Then cmd$ = Mid$(cmd$, 2)

  ' Read no more arguments than will fit in array and fill remainder with empty strings.
  argc% = 0
  Local i%, s$
  For i% = 0 To Bound(argv$(), 1)
    s$ = str.next_token$()
    If s$ = sys.NO_DATA$ Then s$ = "" Else Inc argc%
    argv$(i%) = s$
  Next
End Sub
