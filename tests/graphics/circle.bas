Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, -1, -1, 640, 480
  Graphics Write 0
EndIf

Dim t% = Timer
Dim a!, c%, fill%, i%, r%, x%, y%
Dim colours%(4) = ( Rgb(Red), Rgb(Green), Rgb(Blue), Rgb(Yellow), Rgb(Magenta) )

For i% = 1 To 100
  c% = colours%(Int(5 * Rnd()))
  fill% = Choice(i% Mod 2, c%, Rgb(Black))
  x% = (Mm.Info(HRes) + 40) * Rnd() - 20
  y% = (Mm.Info(VRes) + 40) * Rnd() - 20
  r% = 100 * Rnd()
  a! = 0.5 + 0.5 * Rnd()
  Circle x%, y%, r%, 1, a!, c%, fill%
Next

? Timer - t%
