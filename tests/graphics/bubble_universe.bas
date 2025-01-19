Option Base 0
Option Default Float
Option Explicit

Option Simulate "PicoMiteVGA"

'If Mm.Device$ = "MMB4L" Then
'  Graphics Window 0, 320, 240
'  Graphics Write 0
'EndIf

Mode 2
Dim cx = Mm.HRes\2
Dim cy = Mm.VRes\2
Dim sc = Mm.VRes/4
Dim r = (2*Pi)/25
Dim x = 0, v = 0, t = 0
Dim col, i, j, u

FrameBuffer Layer
FrameBuffer Write L
Do
  Cls
  col = 1
  For i = 50 To 80 Step 2
    For j = 50 To 70 Step 1
      u = Sin(i + v) + Sin(r * i + x)
      v = Cos(i + v) + Cos(r * i + x)
      x = u + t
      Pixel cx + sc * u, cy + sc * v, Map(col)
    Next
    Inc col
    If col > 15 Then col = 1
  Next
  Inc t, .025
  FrameBuffer Copy L, N, B
Loop

Function Map(col)
  Select Case col
    Case 0: Map = Rgb(Black)
    Case 1: Map = Rgb(Blue)
    Case 2: Map = Rgb(Myrtle)
    Case 3: Map = Rgb(Cobalt)
    Case 4: Map = Rgb(MidGreen)
    Case 5: Map = Rgb(Cerulean)
    Case 6: Map = Rgb(Green)
    Case 7: Map = Rgb(Cyan)
    Case 8: Map = Rgb(Red)
    Case 9: Map = Rgb(Magenta)
    Case 10: Map = Rgb(Rust)
    Case 11: Map = Rgb(Fuchsia)
    Case 12: Map = Rgb(Brown)
    Case 13: Map = Rgb(Lilac)
    Case 14: Map = Rgb(Yellow)
    Case 15: Map = Rgb(White)
  End Select
End Function
