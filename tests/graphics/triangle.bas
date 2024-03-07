Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, -1, -1, 640, 480
  Graphics Write 0
EndIf

Const NUM_COLOURS = 7
Const NUM_SHAPES = 20

Dim COLOURS(NUM_COLOURS - 1) As Integer
COLOURS(0) = Rgb(Red)
COLOURS(1) = Rgb(Green)
COLOURS(2) = Rgb(Blue)
COLOURS(3) = Rgb(Yellow)
COLOURS(4) = Rgb(Magenta)
COLOURS(5) = Rgb(Cyan)
COLOURS(6) = Rgb(White)

Dim counter%, s$, t% = Timer

Restore subroutine_data
Do
  Read s$
  If s$ = "" Then Exit Do
  ? s$
  Cls
  Call s$
  Pause 1000
  Inc counter%
Loop

? "Time: " + Str$(Timer - t% - 1000 * counter%) + " ms"
? "Press Ctrl-C to exit"
Do : Loop

subroutine_data:
Data "draw_triangles", "draw_triangle_array", ""

Sub draw_triangles()
  Local col%, fill%, i%, x1%, y1%, x2%, y2%, x3%, y3%
  For i% = 1 To NUM_SHAPES
    col% = COLOURS(Int(NUM_COLOURS * Rnd()))
    fill% = Choice(Rnd() * 2 > 1, col%, Rgb(Black))
    x1% = (Mm.HRes + 40) * Rnd() - 20
    x2% = (Mm.HRes + 40) * Rnd() - 20
    x3% = (Mm.HRes + 40) * Rnd() - 20
    y1% = (Mm.VRes + 40) * Rnd() - 20
    y2% = (Mm.VRes + 40) * Rnd() - 20
    y3% = (Mm.VRes + 40) * Rnd() - 20
    Triangle x1%, y1%, x2%, y2%, x3%, y3%, col%, fill%
  Next
End Sub

Sub draw_triangle_array()
  Local col%(NUM_SHAPES - 1), fill%(NUM_SHAPES - 1), i%
  Local x1%(NUM_SHAPES - 1), x2%(NUM_SHAPES - 1), x3%(NUM_SHAPES - 1)
  Local y1%(NUM_SHAPES - 1), y2%(NUM_SHAPES - 1), y3%(NUM_SHAPES - 1)
  For i% = 0 To NUM_SHAPES - 1
    col%(i%) = COLOURS(Int(NUM_COLOURS * Rnd()))
    fill%(i%) = Choice(Rnd() * 2 > 1, col%(i%), Rgb(Black))
    x1%(i%) = (Mm.HRes + 40) * Rnd() - 20
    x2%(i%) = (Mm.HRes + 40) * Rnd() - 20
    x3%(i%) = (Mm.HRes + 40) * Rnd() - 20
    y1%(i%) = (Mm.VRes + 40) * Rnd() - 20
    y2%(i%) = (Mm.VRes + 40) * Rnd() - 20
    y3%(i%) = (Mm.VRes + 40) * Rnd() - 20
  Next
  Triangle x1%(), y1%(), x2%(), y2%(), x3%(), y3%(), col%(), fill%()
End Sub
