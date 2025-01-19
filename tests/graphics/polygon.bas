Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, 640, 480
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

Dim counter%, n%, s$, t% = Timer

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
Data "draw_triangles", "draw_triangle_array"
Data "draw_squares", "draw_square_array"
Data "draw_diamonds", "draw_diamond_array"
Data "draw_pentagons", "draw_pentagon_array"
Data "draw_hexagons", "draw_hexagon_array"
Data "draw_stars", "draw_star_array"
Data "draw_strange_shapes", "draw_strange_shape_array"
Data ""

Sub draw_triangles()
  draw_shapes("triangle")
End Sub

Sub draw_triangle_array()
  draw_shape_array("triangle")
End Sub

Sub draw_squares()
  draw_shapes("square")
End Sub

Sub draw_square_array()
  draw_shape_array("square")
End Sub

Sub draw_diamonds()
  draw_shapes("diamond")
End Sub

Sub draw_diamond_array()
  draw_shape_array("diamond")
End Sub

Sub draw_pentagons()
  draw_shapes("pentagon")
End Sub

Sub draw_pentagon_array()
  draw_shape_array("pentagon")
End Sub

Sub draw_hexagons()
  draw_shapes("hexagon")
End Sub

Sub draw_hexagon_array()
  draw_shape_array("hexagon")
End Sub

Sub draw_stars()
  draw_shapes("star")
End Sub

Sub draw_star_array()
  draw_shape_array("star")
End Sub

Sub draw_strange_shapes()
  draw_shapes("strange_shape")
End Sub

Sub draw_strange_shape_array()
  draw_shape_array("strange_shape")
End Sub

Sub init_shape(shape$)
  Read Save
  Restore shape$ + "_data"
  Read n%
  On Error Skip
  Erase shape.x!(), shape.y!()
  Dim shape.x!(n% - 1), shape.y!(n% - 1)
  Local i%
  For i% = 0 To n% - 1
    Read shape.x!(i%), shape.y!(i%)
  Next
  Read Restore
End Sub

Sub draw_shapes(s$)
  init_shape(s$)
  Local col%, fill%, i%, j%, size%, x!(n% - 1), xdelta%, y!(n% - 1), ydelta%
  For i% = 1 To NUM_SHAPES
    xdelta% = (Mm.Info(HRes) + 40) * Rnd() - 20
    ydelta% = (Mm.Info(VRes) + 40) * Rnd() - 20
    size% = 60 * Rnd()
    col% = COLOURS(Int(NUM_COLOURS * Rnd()))
    fill% = Choice(Rnd() * 2 > 1, col%, Rgb(Black))
    For j% = 0 To n% - 1
      x!(j%) = size% * shape.x!(j%) + xdelta%
      y!(j%) = size% * shape.y!(j%) + ydelta%
    Next
    Polygon n%, x!(), y!(), col%, fill%
  Next
End Sub

Sub draw_shape_array(s$)
  init_shape(s$)
  Local col%(NUM_SHAPES - 1), fill%(NUM_SHAPES - 1)
  ' Local col%, fill%
  Local i%, j%, size%, xdelta%, ydelta%
  Local x!(n% * NUM_SHAPES - 1), y!(n% * NUM_SHAPES - 1)
  Local n_array%(NUM_SHAPES - 1)
  For i% = 0 To NUM_SHAPES - 1
    n_array%(i%) = n%
    col%(i%) = Rgb(White) ' COLOURS(Int(NUM_COLOURS * Rnd()))
    fill%(i%) = COLOURS(Int(NUM_COLOURS * Rnd())) 'Choice(Rnd() * 2 > 1, Rgb(Red), Rgb(Black))
'    col% = COLOURS(Int(NUM_COLOURS * Rnd()))
'    fill% = Choice(Rnd() * 2 > 1, col%, Rgb(Black))
    xdelta% = (Mm.Info(HRes) + 40) * Rnd() - 20
    ydelta% = (Mm.Info(VRes) + 40) * Rnd() - 20
    size% = 60 * Rnd()
    For j% = 0 To n% - 1
      x!(i% * n% + j%) = size% * shape.x!(j%) + xdelta%
      y!(i% * n% + j%) = size% * shape.y!(j%) + ydelta%
    Next
  Next
'  Polygon n_array%(), x!(), y!(), col%, fill%
  Polygon n_array%(), x!(), y!(), col%(), fill%()
End Sub

triangle_data:
Data 3, 0, -1, 1, 1, -1, 1

square_data:
Data 4, 0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5

diamond_data:
Data 4, 0, 1, 1, 0, 0, -1, -1, 0

pentagon_data:
Data 5, 0, -1, 0.9511, -0.3090, 0.5878, 0.8090, -0.5878, 0.8090, -0.9511, -0.3090

hexagon_data:
Data 6, 1, 0, 0.5, -0.8660, -0.5, -0.8660, -1, 0, -0.5, 0.8660, 0.5, 0.8660

star_data:
Data 16, 0, 1, 0.25, 0.5, 0.75, 0.75, 0.5, 0.25, 1, 0, 0.5, -0.25, 0.75, -0.75
Data 0.25, -0.5, 0, -1, -0.25, -0.5, -0.75, -0.75, -0.5, -0.25, -1, 0, -0.5
Data 0.25, -0.75, 0.75, -0.25, 0.5

strange_shape_data:
Data 10, 0, 0, 1.5, -0.25, 1.25, 1, 1, 0.75, 1.25, 0, 0.25, 0.25, 0.25, 0.5
Data 1, 0.25, 0.25, 0.75, -0.25, 0.5
