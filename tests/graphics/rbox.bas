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
Data "draw_rboxes", "draw_rbox_array", ""

Sub draw_rboxes()
  Local col%, fill%, i%, x%, y%, w%, h%
  For i% = 1 To NUM_SHAPES
    col% = COLOURS(Int(NUM_COLOURS * Rnd()))
    fill% = Choice(i% Mod 2, col%, Rgb(Black))
    x% = (Mm.Info(HRes) + 40) * Rnd() - 20
    y% = (Mm.Info(VRes) + 40) * Rnd() - 20
    w% = 100 * Rnd() + 20
    h% = 100 * Rnd() + 20
    RBox x%, y%, w%, h%, 10, col%, fill%
  Next
End Sub

Sub draw_rbox_array()
  Local col%(NUM_SHAPES - 1), fill%(NUM_SHAPES - 1), i%
  Local x%(NUM_SHAPES - 1), y%(NUM_SHAPES - 1), w%(NUM_SHAPES - 1), h%(NUM_SHAPES - 1)
  For i% = 0 To NUM_SHAPES - 1
    col%(i%) = COLOURS(Int(NUM_COLOURS * Rnd()))
    fill%(i%) = Choice(i% Mod 2, col%(i%), Rgb(Black))
    x%(i%) = (Mm.Info(HRes) + 40) * Rnd() - 20
    y%(i%) = (Mm.Info(VRes) + 40) * Rnd() - 20
    w%(i%) = 100 * Rnd() + 20
    h%(i%) = 100 * Rnd() + 20
  Next
  RBox x%(), y%(), w%(), h%(), 10, col%(), fill%()
End Sub
