Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, 640, 480
  Graphics Write 0
EndIf

Dim t% = Timer

Dim ch%, font%, x%, y%

For font% = 1 To 7
  Cls
  Font 3
  Text 0, 0, "Font " + Str$(font%)
  x% = 0
  y% = 30
  Font font%
  For ch% = 32 To 255
    Text x% + 2, y% + 2, Chr$(ch%)
    Box x%, y%, Mm.Info(FontWidth) + 4, Mm.Info(FontHeight) + 4, 1, Rgb(Grey)
    Inc x%, Mm.Info(FontWidth) + 6
    If x% > Mm.HRes - Mm.Info(FontWidth) Then
      Inc y%, Mm.Info(FontHeight) + 6
      x% = 0
    EndIf
  Next
  Pause 2000
Next

? Timer - t% - 7 * 2000
