' Copyright (c) 2022 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 5.07

' Updates license comments for MMB4L source files.

Option Base 0
Option Default None
Option Explicit On

#Include "../sptools/src/splib/system.inc"
#Include "../sptools/src/splib/file.inc"
#Include "../sptools/src/splib/string.inc"

Const SEP$ = "/"
Const C_TEMPLATE$ = Mm.Info$(Path) + "/" + "c_copyright.template"
Const C_MIT_TEMPLATE$ = Mm.Info$(Path) + "/" + "c_mit_copyright.template"
Const BAS_TEMPLATE$ = Mm.Info$(Path) + "/" + "bas_copyright.template"

Dim IGNORED$(4) = ("/build/", "/build-clang/", "/build-distrib/", "/sptools/src/splib/", "/sptools/src/sptest/")

update_comments(".c", C_TEMPLATE$)
update_comments(".h", C_TEMPLATE$)
update_comments(".cpp", C_TEMPLATE$)
update_comments(".cxx", C_TEMPLATE$)
update_comments(".bas", BAS_TEMPLATE$)
update_comments(".inc", BAS_TEMPLATE$)

End

Sub update_comments(extension$, template$)
  Local dir_name$ = UCase$(Mid$(extension$, 2))
  Local i%, ignore%, new_f$, new_parent$, parent$, s$
  Local f$ = file.find$(Cwd$, "*" + extension$, "file")
  Local state%
  Do While f$ <> ""
    ignore% = 0
    For i% = 0 To Bound(IGNORED$(), 1)
      If InStr(f$, IGNORED$(i%)) Then
        ignore% = 1
        Exit For
      EndIf
    Next

    If ignore% Then
      ' Print "Ignored " f$
      f$ = file.find$()
      Continue Do
    EndIf

    If InStr(f$, "/gtest/") And template$ = C_TEMPLATE$ Then template$ = C_MIT_TEMPLATE$

    Print "Processing " f$ " ..."
    parent$ = file.get_parent$(f$)
    new_f$ = f$ + ".new"

    Open new_f$ For Output As #1

    ' Transcribe template.
    Open template$ For Input As #2
    Do While Not Eof(#2)
      Line Input #2, s$
      If s$ = "<filename>" Then s$ = file.get_name$(f$)
      Print #1, s$ Chr$(10);
    Loop
    Close #2

    Print #1, Chr$(10);

    ' Transcribe original file, removing any leading comment.
    state% = 0
    Open f$ For Input As #2
    Do While Not Eof(#2)
      Line Input #2, s$
      Select Case state%
        Case 0
          If Left$(str.trim$(s$), 2) = "/*" Then
            state% = 1
          Else If Left$(str.trim$(s$), 1) = "'" Then
            state% = 1
          Else
            Print #1, s$ Chr$(10);
          EndIf
        Case 1
          If Right$(str.trim$(s$), 2) = "*/" Then
            state% = 2
          Else If Left$(str.trim$(s$), 1) <> "'" Then
            state% = 2
          EndIf
        Case 2
          If str.trim$(s$) <> "" Then Print #1, s$ Chr$(10)
          state% = 3
        Case 3
          Print #1, s$ Chr$(10);
        Case Else
          Error "Invalid state"
      End Select
    Loop
    Close #2

    Close #1

    Rename new_f$ As f$

    f$ = file.find$()
  Loop
End Sub
