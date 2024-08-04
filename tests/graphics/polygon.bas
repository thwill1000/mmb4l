Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, -1, -1, 640, 480
  Graphics Write 0
EndIf

Dim t% = Timer
Dim colours%(4) = ( Rgb(Red), Rgb(Green), Rgb(Blue), Rgb(Yellow), Rgb(Magenta) )

Dim i%, j%, n%
Dim col%, fill%, size%, xdelta%, ydelta%

For i% = 1 To 40
  Restore pentagon_data
  Read n%
  On Error Skip
  Erase x!(), y!()
  Dim x!(n% - 1), y!(n% - 1)
  xdelta% = (Mm.Info(HRes) + 40) * Rnd() - 20
  ydelta% = (Mm.Info(VRes) + 40) * Rnd() - 20
  size% = 60 * Rnd()
  col% = colours%(Int(5 * Rnd()))
  fill% = Choice(i% Mod 2, col%, Rgb(Black))
  For j% = 0 To n% - 1
    Read x!(j%), y!(j%)
    x!(j%) = size% * x!(j%) + xdelta%
    y!(j%) = size% * y!(j%) + ydelta%
  Next
  Polygon n%, x!(), y!(), col%, fill%
Next i%

? Timer - t%

Do : Loop

pentagon_data:
Data 5, 0, -1, 0.9511, -0.3090, 0.5878, 0.8090, -0.5878, 0.8090, -0.9511, -0.3090

hexagon_data:
Data 6, 1, 0, 0.5, -0.8660, -0.5, -0.8660, -1, 0, -0.5, 0.8660, 0.5, 0.8660

