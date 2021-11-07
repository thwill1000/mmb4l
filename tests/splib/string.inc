' Copyright (c) 2020-2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

'!remove_if NO_INCLUDE_GUARDS
On Error Skip 1 : Dim sys.VERSION$ = ""
If sys.VERSION$ = "" Then Error "'system.inc' not included"
sys.provides("string")
If sys.err$ <> "" Then Error sys.err$
'!endif

' Pads a string with spaces to the left and right so that it will be centred
' within a fixed length field. If the string is longer than the field then
' this function just returns the string. If an odd number of spaces are
' required then the extra space is added to the left hand side of the string.
'
' @param  s$  the string to centre.
' @param  x   the field length.
Function str.centre$(s$, x%)
  If Len(s$) < x% Then
    str.centre$ = s$ + Space$((x% - Len(s$)) \ 2)
    str.centre$ = Space$(x% - Len(str.centre$)) + str.centre$
  Else
    str.centre$ = s$
  EndIf
End Function

' Compares s1$ and s2$ ignoring case considerations.
Function str.equals_ignore_case%(s1$, s2$)
  str.equals_ignore_case% = LCase$(s1$) = LCase$(s2$)
End Function

' Does a string contain only printable and/or whitespace ASCII ?
'
' @param  s$  the string.
' @return     1 if the string only contains printable and/or whitespace ASCII, otherwise 0.
Function str.is_plain_ascii%(s$)
  Local i%
  For i% = 1 To Len(s$)
    Select Case Peek(Var s$, i%)
      Case 9, 10, 13   : ' Tab, line-feed and carriage-return are acceptable.
      Case < 32, > 126 : Exit Function
      Case Else        : ' ASCII printable characters.
    End Select
  Next
  str.is_plain_ascii% = 1
End Function

Function str.lpad$(s$, x%)
  str.lpad$ = s$
  If Len(s$) < x% Then str.lpad$ = Space$(x% - Len(s$)) + s$
End Function

' Tokenises a string.
'
' @param   s$     string to tokenise.
' @param   sep$   one or more token separator characters.
'                 If empty then use space and skip empty tokens.
' @param   skip%  1 to skip empty tokens, 0 to return them.
' @return  the first token. To retrieve subsequent tokens call this function
'          with no parameters, i.e. tk$ = str.next_token$().
'          Returns sys.NO_DATA$ if there are no more tokens.
'
' WARNING! Take care when calling this function naively in cases where s$ might
' be the empty string as that will return data from a previously incomplete
' tokenisation. If necessary call str.next_token$(sys.NO_DATA$) to clear the
' internal state first.
Function str.next_token$(s$, sep$, skip%)
  Static s_$ = sys.NO_DATA$, sep_$, skip_%, st%

  If s$ <> "" Then
    s_$ = s$
    sep_$ = Choice(sep$ = "", " ", sep$)
    skip_% = Choice(sep$ = "", 1, skip%)
    st% = 1
  EndIf

  ' Have we already processed then entire string?
  If s_$ = sys.NO_DATA$ Then str.next_token$ = sys.NO_DATA$ : Exit Function

  Local i%
  For i% = st% To Len(s_$)
    If InStr(sep_$, Mid$(s_$, i%, 1)) > 0 Then
      str.next_token$ = Mid$(s_$, st%, i% - st%)
      st% = i% + 1
      If skip_% And str.next_token$ = "" Then Continue For
      Exit Function
    EndIf
  Next

  ' Return the last token.
  Local tmp$ = Mid$(s_$, st%, i% - st%)
  str.next_token$ = Choice(tmp$ = "" And skip_%, sys.NO_DATA$, tmp$)
  s_$ = sys.NO_DATA$
End Function

Function str.quote$(s$, begin$, end$)
  Local begin_$ = Choice(begin$ = "", Chr$(34), Left$(begin$, 1))
  Local end_$ = Choice(end$ = "", begin_$, Left$(end$, 1))
  str.quote$ = begin_$ + s$ + end_$
End Function

' Gets copy of 'haystack$' with occurrences of 'needle$' replaced by 'rep$'.
Function str.replace$(haystack$, needle$, rep$)
  Local p%, st%, s$
  Do
    Inc st%
    p% = InStr(st%, haystack$, needle$)
    If p% < 1 Then Exit Do
    Cat s$, Mid$(haystack$, st%, p% - st%) + rep$
    st% = p% + Len(needle$) - 1
  Loop
  Cat s$, Mid$(haystack$, st%)
  str.replace$ = s$
End Function

Function str.rpad$(s$, x%)
  str.rpad$ = s$
  If Len(s$) < x% Then str.rpad$ = s$ + Space$(x% - Len(s$))
End Function

' Returns a copy of s$ with leading and trailing spaces removed.
Function str.trim$(s$)
  Local st%, en%
  For st% = 1 To Len(s$)
    If Peek(Var s$, st%) <> 32 Then Exit For
  Next
  For en% = Len(s$) To 1 Step -1
    If Peek(Var s$, en%) <> 32 Then Exit For
  Next
  If en% >= st% Then str.trim$ = Mid$(s$, st%, en% - st% + 1)
End Function

' If s$ both begins and ends with " then returns a copy of s$ with those characters removed,
' otherwise returns an unmodified copy of s$.
Function str.unquote$(s$)
  If Peek(Var s$, 1) = 34 Then
    If Peek(var s$, Len(s$)) = 34 Then
      str.unquote$ = Mid$(s$, 2, Len(s$) - 2)
      Exit Function
    EndIf
  EndIf
  str.unquote$ = s$
End Function
