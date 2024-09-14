Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, 640, 480
  Graphics Write 0
EndIf

Dim ch$, i%, locks%, mods%, x%, y%

Do
  ch$ = Inkey$
  If ch$ <> "" Then
    If Asc(ch$) < 32 Or Asc(ch$) >= 127 Then
      ch$ = "<" + Hex$(Asc(ch$), 2) + ">"
    EndIf
    output(ch$, Rgb(White))
  EndIf

  If Keydown(0) Then
    For i% = 1 To Keydown(0)
      output(format_char$(Keydown(i%)), Rgb(Green))
    Next
  EndIf

  mods% = Keydown(7)
  If mods% And &h01 Then output("LALT", Rgb(Yellow))
  If mods% And &h02 Then output("LCTRL", Rgb(Yellow))
  If mods% And &h04 Then output("LGUI", Rgb(Yellow))
  If mods% And &h08 Then output("LSHIFT", Rgb(Yellow))
  If mods% And &h10 Then output("RALT", Rgb(Yellow))
  If mods% And &h20 Then output("RCTRL", Rgb(Yellow))
  If mods% And &h40 Then output("RGUI", Rgb(Yellow))
  If mods% And &h80 Then output("RSHIFT", Rgb(Yellow))

  locks% = Keydown(8)
  If locks% And &h01 Then output("CAPS", Rgb(Red))
  If locks% And &h02 Then output("NUM", Rgb(Red))
  If locks% And &h04 Then output("SCROLL", Rgb(Red))

  Pause 100
Loop

Function format_char$(ch%)
  If ch% < 32 Or ch% > 127 Then
    format_char$ = "<" + Hex$(ch%, 2) + ">"
  Else
    format_char$ = Chr$(ch%)
  EndIf
End Function

Sub output(s$, colour%)
  Text x%, y%, s$, , , , colour%
  Inc x%, Len(s$) * Mm.Info(FontWidth)
  If x% >= Mm.HRes Then
    x% = 0
    Inc y%, Mm.Info(FontHeight)
  EndIf
End Sub
