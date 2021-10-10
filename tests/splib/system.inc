' Copyright (c) 2020-2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

Const sys.VERSION$ = "2.0.0-a1"
Const sys.NO_DATA$ = Chr$(&h7F)
Const sys.CRLF$ = Chr$(13) + Chr$(10)

Dim sys.err$

'!remove_if NO_INCLUDE_GUARDS
Const sys.MAX_INCLUDES% = 20
Dim sys.includes$(sys.MAX_INCLUDES%) Length 20

Sub sys.provides(f$)
  Local f_$ = LCase$(f$)
  Local i% = 1
  Do
    Select Case sys.includes$(i%)
      Case f_$ : sys.err$ = "file already included: " + f_$ + ".inc" : Exit Sub
      Case ""  : sys.includes$(i%) = f_$ : Exit Sub
    End Select
    Inc i%
    If i% > sys.MAX_INCLUDES% Then sys.err$ = "too many includes" : Exit Sub
  Loop
End Sub

Sub sys.requires(f1$, f2$, f3$, f4$, f5$, f6$, f7$, f8$, f9$, f10$)

  ' Use indexes from 1..10 even when Option Base 0.
  Local f$(10) Length 20
  f$(1) = f1$ : f$(2) = f2$ : f$(3) = f3$ : f$(4) = f4$ : f$(5) = f5$
  f$(6) = f6$ : f$(7) = f7$ : f$(8) = f8$ : f$(9) = f9$ : f$(10) = f10$
  Local i%
  For i% = 1 To 10 : f$(i%) = LCase$(f$(i%)) : Next

  Local j%, ok%, fail%
  For i% = 1 To 10
    If f$(i%) <> "" Then
      ok% = 0
      For j% = 1 To sys.MAX_INCLUDES%
        Select Case sys.includes$(j%)
          Case f$(i%) : ok% = 1 : Exit For
          Case ""     : Exit For
        End Select
      Next
      If Not ok% Then
        If Not fail% Then
          sys.err$ = "required file(s) not included: " + f$(i%) + ".inc"
          fail% = 1
        Else
          Cat sys.err$, ", " + f$(i%) + ".inc"
        EndIf
      EndIf
    EndIf
  Next
End Sub
'!endif

' Formats a firmware version as a 5-digit number, e.g.
'   5.05.06 => 50506
'   5.04    => 50400
'
' @param version$  the firmware version to format.
'                  If empty then formats the current firmware version number.
Function sys.firmware_version%(version$)
  Local i%, s$, v$ = version$
  If v$ = "" Then v$ = Str$(Mm.Info$(Version))
  For i% = 1 To Len(v$)
    If InStr("0123456789", Mid$(v$, i%, 1)) > 0 Then s$ = s$ + Mid$(v$, i%, 1)
  Next
  Do While Len(s$) < 5 : s$ = s$ + "0" : Loop
  sys.firmware_version% = Val(s$)
End Function
