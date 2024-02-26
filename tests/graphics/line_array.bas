Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, -1, -1, 640, 480
  Graphics Write 0
EndIf

Dim t% = Timer
Dim colours%(4) = ( Rgb(Red), Rgb(Green), Rgb(Blue), Rgb(Yellow), Rgb(Magenta) )
Dim c%(3), i%, j%, x1%(3), y1%(3), x2%(3), y2%(3)

For i% = 1 To 100
  For j% = 0 To 3
    c%(j%) = colours%(Int(5 * Rnd()))
    x1%(j%) = (Mm.Info(HRes) + 40) * Rnd() - 20
    x2%(j%) = (Mm.Info(HRes) + 40) * Rnd() - 20
    y1%(j%) = (Mm.Info(VRes) + 40) * Rnd() - 20
    y2%(j%) = (Mm.Info(VRes) + 40) * Rnd() - 20
  Next
  Line x1%(), y1%(), x2%(), y2%(), 1, c%()
Next

? Timer - t%
