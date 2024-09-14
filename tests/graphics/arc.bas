Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, 640, 480
  Graphics Write 0
EndIf

Dim t% = Timer
Dim colours%(4) = ( Rgb(Red), Rgb(Green), Rgb(Blue), Rgb(Yellow), Rgb(Magenta) )

Dim i%, x%, y%, r1%, r2%, arcrad1%, arcrad2%, col%

For i% = 1 To 40
  x% = (Mm.Info(HRes) + 40) * Rnd() - 20
  y% = (Mm.Info(VRes) + 40) * Rnd() - 20
  r1% = 20 * Rnd()
  r2% = r1% + 10 * Rnd()
  arcrad1% = Rnd() * 360
  arcrad2% = arcrad1% + Rnd() * 270
  col% = colours%(Int(5 * Rnd()))
  Arc x%, y%, r1%, r2%, arcrad1%, arcrad2%, col%
Next

? Timer - t%

Do : Loop
