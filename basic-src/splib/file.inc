' Copyright (c) 2020-2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

'!remove_if NO_INCLUDE_GUARDS
On Error Skip 1 : Dim sys.VERSION$ = ""
If sys.VERSION$ = "" Then Error "'system.inc' not included"
sys.provides("file")
If sys.err$ <> "" Then Error sys.err$
'!endif

Const file.PROG_DIR$ = file.get_parent$(Mm.Info$(Current))

' Gets the number of files in directory d$ whose names match pattern$.
'
' @param   d$        directory to process.
' @param   pattern$  file pattern to match on, e.g. "*.bas".
' @param   type$     type of files to return, "dir", "file" or "all".
' @return  the number of matching files.
Function file.count_files%(d$, pattern$, type$)
  Local f$

  Select Case LCase$(type$)
    Case "all"      : f$ = Dir$(d$ + "/*", All)
    Case "dir"      : f$ = Dir$(d$ + "/*", Dir)
    Case "file", "" : f$ = Dir$(d$ + "/*", File)
    Case Else       : Error "unknown type: " + type$
  End Select

  Do While f$ <> ""
    If file.fnmatch%(pattern$, f$) Then Inc file.count_files%
    f$ = Dir$()
  Loop
End Function

' Does the file/directory 'f$' exist?
'
' @return 1 if the file exists, otherwise 0
Function file.exists%(f$)
  Local f_$ = file.get_canonical$(f$)
  If f_$ = "A:" Then
    file.exists% = 1
  Else
    file.exists% = Mm.Info(FileSize f_$) <> -1
  EndIf
End Function

' Find files whose names match pattern$
'
' @param   path$     root directory to start looking from.
' @param   pattern$  file pattern to match on, e.g. "*.bas".
' @param   type$     type of files to return, "dir", "file" or "all".
' @return  the absolute path to the first match found. To retrieve subsequent
'          matches call this function with no parameters, i.e. f$ = file.find().
'          Returns the empty string if there are no more matches.
'
' TODO: In order to return the files in alphabetical order this uses a 128K workspace.
'       I think there is another potential implementation where it just records the last
'       file returned by the previous call and restarts the search from there, however
'       that would be much slower and probably better implemented as a CSUB.
Function file.find$(path$, pattern$, type$)
  If Mm.Device$ = "MMB4L" Then
    Static stack$(1000) Length 255 ' ~256K workspace.
  Else
    Static stack$(1000) Length 128 ' ~128K workspace.
  EndIf
  Static stack_ptr% = Bound(stack$(), 0)
  Static pattern_$
  Static type_$
  Local f$, is_dir%, lb%, name$, num%

  If path$ <> "" Then
    f$ = file.get_canonical$(path$)
    stack$(stack_ptr%) = f$ : Inc stack_ptr%
    pattern_$ = pattern$
    If pattern_$ = "" Then pattern_$ = "*"
    type_$ = Choice(type$ = "", "FILE", UCase$(type$))
    If InStr("|ALL|DIR|FILE|", "|" + type_$ + "|") < 1 Then Error "unknown type: " + type_$
  EndIf

  Do
    Do
      If stack_ptr% = Mm.Info(Option Base) Then file.find$ = "" : Exit Function
      Inc stack_ptr%, -1 : file.find$ = stack$(stack_ptr%)
      name$ = file.get_name$(file.find$)
    Loop Until name$ <> ".git" ' Doesn't recurse into .git directories.

    ' If it is a directory then expand it.
    is_dir% = Choice(file.find$ = "A:", 1, Mm.Info(FileSize file.find$) = -2)
    If is_dir% Then
      lb% = stack_ptr%
      ' On Error Skip 1
      f$ = Choice(type_$ = "DIR", Dir$(file.find$ + "/*", Dir), Dir$(file.find$ + "/*", All))
      ' If Mm.ErrMsg$ <> "" Then Print MM.ErrMsg$ : Print file.find$ : End
      Do While f$ <> ""
        stack$(stack_ptr%) = file.find$  + "/" + f$ : Inc stack_ptr%
        f$ = Dir$()
      Loop

      ' Sort the newly pushed dirs/files so that those beginning 'a|A'
      ' are at the top and those beginning 'z|Z' are near the bottom.
      num% = stack_ptr% - lb%
      If num% > 0 Then Sort stack$(), , &b11, lb%, num%)
    EndIf

    ' I've profiled it and its faster to do the name checks before the type checks.
    If file.fnmatch%(pattern_$, name$) Then
      Select Case type_$
        Case "ALL"  : Exit Do
        Case "DIR"  : If is_dir% Then Exit Do
        Case "FILE" : If Not is_dir% Then Exit Do
      End Select
    EndIf

  Loop

End Function

' Does name$ match pattern$ ?
'
' @param   pattern$  *nix style 'shell wildcard' pattern.
' @param   name$     the name to test.
' @return  1 if the name matches the pattern otherwise 0.
'
' Derived from the work of Russ Cox: https://research.swtch.com/glob
Function file.fnmatch%(pattern$, name$)
  Local p$ = UCase$(pattern$)
  Local n$ = UCase$(name$)
  Local c%, px% = 1, nx% = 1, nextPx% = 0, nextNx% = 0

  Do While px% <= Len(p$) Or nx% <= Len(n$)

    If px% <= Len(p$) Then

      c% = Peek(Var p$, px%)
      Select Case c%
        Case 42 ' *
          ' Zero-or-more-character wildcard
          ' Try to match at sx%,
          ' if that doesn't work out,
          ' restart at nx%+1 next.
          nextPx% = px%
          nextNx% = nx% + 1
          Inc px%
          Goto file.fnmatch_cont

        Case 63 ' ?
          ' Single-character wildcard
          If nx% <= Len(n$) Then
            Inc px%
            Inc nx%
            Goto file.fnmatch_cont
          EndIf

        Case Else
          ' Ordinary character
          If nx% <= Len(n$) Then
            If c% = Peek(Var n$, nx%) Then
              Inc px%
              Inc nx%
              Goto file.fnmatch_cont
            EndIf
          EndIf
      End Select

    EndIf

    If nextNx% > 0 Then
      If nextNx% <= Len(n$) + 1 Then
        px% = nextPx%
        nx% = nextNx%
        Goto file.fnmatch_cont
      EndIf
    EndIf

    Exit Function

    file.fnmatch_cont:

  Loop

  file.fnmatch% = 1

End Function

' Gets the canonical path for file/directory 'f$'.
Function file.get_canonical$(f$)

  Local f2$ = UCase$(f$), i%
  If f2$ = "A:" Then
    file.get_canonical$ = "A:"
    Exit Function
  ElseIf Left$(f2$, 3) = "A:\" Or Left$(f2$, 3) = "A:/" Then
    f2$ = "A:"
    i% = 4
  ElseIf Left$(f2$, 1) = "\" Or Left$(f2$, 1) = "/" Then
    f2$ = "A:"
    i% = 2
  ElseIf f2$ = "~" Then
    file.get_canonical$ = sys.string_prop$("home")
    Exit Function
  ElseIf Left$(f2$, 2) = "~\" Or Left$(f2$, 2) = "~/" Then
    f2$ = sys.string_prop$("home")
    i% = 3
  Else
    f2$ = Left$(Mm.Info$(Directory), Len(Mm.Info$(Directory)) - 1)
    i% = 1
  EndIf

  ' Copy characters from f$ into f2$:
  '   - replace \ by /
  '   - if encounter /./ replace with /
  '   - if encounter /../ then backtrack to previous /
  Local ch%, j%, s$ = "/"
  For i% = i% To Len(f$)
    ch% = Peek(Var f$, i%)
    Select Case ch%
      Case 47, 92 ' / or \
        Select Case s$
          Case "/." : ' Do nothing
          Case "/.."
            ' Back track
            For j% = Len(f2$) To 1 Step -1
              If Peek(Var f2$, j%) = 47 Then Poke Var f2$, 0, j% - 1 : Exit For
            Next
          Case Else : Cat f2$, s$
        End Select
        s$ = "/"
      Case Else
        Cat s$, Chr$(ch%)
    End Select
  Next

  Select Case s$
    Case "/", "/."
      ' Do nothing
    Case "/.."
      ' Back track
      For j% = Len(f2$) To 1 Step -1
        If Peek(Var f2$, j%) = 47 Then Poke Var f2$, 0, j% - 1 : Exit For
      Next
    Case Else
      Cat f2$, s$
  End Select

  file.get_canonical$ = f2$
End Function

' Gets the dot file-extension, from filename f$.
' e.g. file.get_extension("foo.bas") => ".bas"
Function file.get_extension$(f$)
  Local i%
  For i% = Len(f$) To 1 Step -1
    Select Case Peek(Var f$, i%)
      Case 46     ' .
        file.get_extension$ = Mid$(f$, i%)
        Exit Function
      Case 47, 92 ' / or \
        Exit For
    End Select
  Next
End Function

' Gets the files in directory d$ whose names match pattern$.
'
' @param       d$        directory to process.
' @param       pattern$  file pattern to match on, e.g. "*.bas".
' @param       type$     type of files to return, "dir", "file" or "all".
' @param[out]  out$()    the names of matching files are copied into this array.
Sub file.get_files(d$, pattern$, type$, out$())
  Local f$

  Select Case LCase$(type$)
    Case "all"      : f$ = Dir$(d$ + "/*", All)
    Case "dir"      : f$ = Dir$(d$ + "/*", Dir)
    Case "file", "" : f$ = Dir$(d$ + "/*", File)
    Case Else       : Error "unknown type: " + type$
  End Select

  Local i% = Mm.Info(Option Base)
  Do While f$ <> ""
    If file.fnmatch%(pattern$, f$) Then
      out$(i%) = f$
      Inc i%
    EndIf
    f$ = Dir$()
  Loop

  If i% > Mm.Info(Option Base) Then
    Sort out$(), , &b10, Mm.Info(Option Base), i% - Mm.Info(Option Base)
  EndIf
End Sub

' Gets the name of file/directory 'f$' minus any path information.
Function file.get_name$(f$)
  Local i%

  For i% = Len(f$) To 1 Step -1
    If InStr("/\", Mid$(f$, i%, 1)) > 0 Then Exit For
  Next i%

  file.get_name$ = Mid$(f$, i% + 1)
End Function

' Gets the parent directory of 'f$', or the empty string if it does not have one.
Function file.get_parent$(f$)
  Local i%

  For i% = Len(f$) To 1 Step -1
    If InStr("/\", Mid$(f$, i%, 1)) > 0 Then Exit For
  Next i%

  If i% > 0 Then file.get_parent$ = Left$(f$, i% - 1)
End Function

Function file.is_absolute%(f$)
  Select Case Left$(f$, 1)
    Case "a", "A"
      Select Case Mid$(f$, 2, 2)
        Case ":", ":\", ":/" : file.is_absolute% = 1
      End Select
    Case "/", "\"
      file.is_absolute% = 1
  End Select
End Function

Function file.is_directory%(f$)
  Local f_$ = file.get_canonical$(f$)
  If f_$ = "A:" Then
    file.is_directory% = 1
  Else
    file.is_directory% = Mm.Info(FileSize f_$) = -2
  EndIf
End Function

' Makes directory 'f$' if it does not already exist.
Sub file.mkdir(f$)
  If Not file.exists%(f$) Then MkDir f$
End Sub

' Returns a copy of f$ with any dot file-extension removed.
' e.g. file.trim_extension("foo.bas") => "foo"
Function file.trim_extension$(f$)
  Local i%
  For i% = Len(f$) To 1 Step -1
    Select Case Peek(Var f$, i%)
      Case 46     ' .
        file.trim_extension$ = Mid$(f$, 1, i% - 1)
        Exit Function
      Case 47, 92 ' / or \
        Exit For
    End Select
  Next
  file.trim_extension$ = f$
End Function
