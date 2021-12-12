' Copyright (c) 2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMB4L 2021.01.00

On Error Skip 1 : Dim sys.VERSION$ = ""
If sys.VERSION$ = "" Then Error "'system.inc' not included"
sys.requires("console", "file", "string")
sys.provides("spsh")
If sys.err$ <> "" Then Error sys.err$

Const spsh.MAX_FILES% = 200

Sub spsh.cmd_files(cmd$, argc%, argv$())
  If argc% > 1 Then Error "expects 0 or 1 arguments"

  Local d$ = Choice(argv$(0) = "", ".", argv$(0))
  If file.is_directory%(d$) Then Cat d$, "/*"
  Local files$(spsh.MAX_FILES% - 1) Length 64
  d$ = spsh.get_files$(d$, files$())

  Local f$, fg$, i%, sz%
  For i% = 0 To Bound(files$(), 1)
    If files$(i%) = "" Then Exit For
    f$ = d$ + "/" + files$(i%)
    sz% = Mm.Info(FileSize f$)
    Select Case sz%
      Case -2 : fg$ = "blue" ' Directory
      Case Else
        Select Case LCase$(file.get_extension$(f$))
          Case ".bas" : fg$ = "green"
          Case ".inc" : fg$ = "magenta"
          Case Else   : fg$ = "default"
        End Select
    End Select
    con.foreground("default")
    con.print(Choice(sz% < 0, " <DIR>", str.lpad$(Str$(sz%), 6)))
    con.foreground(fg$)
    con.println(" " + files$(i%))
  Next
  con.foreground("default")
End Sub

Function spsh.get_files$(f$, out$())
  Local i%
  For i% = Bound(out$(), 0) To Bound(out$(), 1) : out$(i%) = "" : Next
  Local d$ = file.get_canonical$(Choice(f$ = "", ".", f$))
  If (f$ = "A:") Or (Mm.Info(FileSize f$) = -2) Then
    ' Directory
    out$(0) = file.get_name$(d$)
    cmd.get_files$ = file.get_parent$(d$)
    Exit Function
  Else
    Local pattern$ = file.get_name$(d$)
    d$ = file.get_parent$(d$)
  EndIf
  file.get_files(d$, pattern$, "all", out$())
  spsh.get_files$ = d$
End Function
