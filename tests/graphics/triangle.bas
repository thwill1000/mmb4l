Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, -1, -1, 640, 480
  Graphics Write 0
EndIf

Dim t% = Timer
Dim c%, fill%, i%, x1%, y1%, x2%, y2%, x3%, y3%
Dim colours%(4) = ( Rgb(Red), Rgb(Green), Rgb(Blue), Rgb(Yellow), Rgb(Magenta) )

For i% = 1 To 5
  c% = colours%(Int(5 * Rnd()))
  fill% = c% ' Choice(i% Mod 2, c%, Rgb(Black))
  x1% = (Mm.HRes + 40) * Rnd() - 20
  x2% = (Mm.HRes + 40) * Rnd() - 20
  x3% = (Mm.HRes + 40) * Rnd() - 20
  y1% = (Mm.VRes + 40) * Rnd() - 20
  y2% = (Mm.VRes + 40) * Rnd() - 20
  y3% = (Mm.VRes + 40) * Rnd() - 20
  Triangle x1%, y1%, x2%, y2%, x3%, y3%, c%, fill%
Next

? Timer - t%
