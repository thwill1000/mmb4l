Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, -1, -1, 640, 480
  Graphics Write 0
EndIf

Dim t% = Timer

Dim ch%, font%, x%, y%

For font% = 1 To 7
  Font font%
  For ch% = 32 To 255
    Text x%, y%, Chr$(ch%)
    Inc x%, Mm.Info(FontWidth)
    If x% > Mm.HRes - Mm.Info(FontWidth) Then
      Inc y%, Mm.Info(FontHeight)
      x% = 0
    EndIf
  Next
  Inc y%, Mm.Info(FontHeight)
  x% = 0
Next

? Timer - t%
