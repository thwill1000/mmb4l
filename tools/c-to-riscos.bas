' Copyright (c) 2022 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 5.07

Option Base 0
Option Default None
Option Explicit On

#Include "../sptools/src/splib/system.inc"
#Include "../sptools/src/splib/file.inc"

Const SEP$ = "/"

move_files_to_riscos(".c")
move_files_to_riscos(".h")
move_files_to_riscos(".cpp")
move_files_to_riscos(".cxx")

End

Sub move_files_to_riscos(extension$)
  Local dir_name$ = UCase$(Mid$(extension$, 2))
  Local new_f$, new_parent$, parent$
  Local f$ = file.find$(Cwd$, "*" + extension$, "file")
  Do While f$ <> ""
    parent$ = file.get_parent$(f$)
    If file.get_name$(parent$) <> dir_name$) Then
      new_parent$ = parent$ + SEP$ + dir_name$
      file.mkdir(new_parent$)
      new_f$ = new_parent$ + SEP$ + file.trim_extension$(file.get_name$(f$))
      Print "Rename " f$ " As " new_f$
      Rename f$ As new_f$
    EndIf
    f$ = file.find$()
  Loop
End Sub
