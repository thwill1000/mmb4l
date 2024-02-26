Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, -1, -1, 640, 480
  Graphics Write 0
EndIf

Const NUM_COLOURS = 7
Const NUM_LINES = 40

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
Data "draw_orthogonal_lines", "draw_orthogonal_line_array"
Data "draw_random_lines", "draw_random_line_array", "draw_line_graph", ""

Sub draw_orthogonal_lines()
  Local col%, i%, width%, x1%, y1%, x2%, y2%
  For i% = 1 To NUM_LINES / 2
    col% = COLOURS(Int(NUM_COLOURS * Rnd()))
    x1% = (Mm.Info(HRes) + 40) * Rnd() - 20
    x2% = (Mm.Info(HRes) + 40) * Rnd() - 20
    y1% = (Mm.Info(VRes) + 40) * Rnd() - 20
    y2% = (Mm.Info(VRes) + 40) * Rnd() - 20
    width% = Int((10 * Rnd()) + 1)
    Line x1%, y1%, x1%, y2%, width%, col%
    Line x1%, y1%, x2%, y1%, width%, col%
  Next
End Sub

Sub draw_random_lines()
  Local col%, i%, x1%, y1%, x2%, y2%
  For i% = 1 To NUM_LINES
    col% = COLOURS(Int(NUM_COLOURS * Rnd()))
    x1% = (Mm.Info(HRes) + 40) * Rnd() - 20
    x2% = (Mm.Info(HRes) + 40) * Rnd() - 20
    y1% = (Mm.Info(VRes) + 40) * Rnd() - 20
    y2% = (Mm.Info(VRes) + 40) * Rnd() - 20
    Line x1%, y1%, x2%, y2%, 1, col%
  Next
End Sub

Sub draw_orthogonal_line_array()
  Local col%(NUM_LINES - 1), i%, width%(NUM_LINES - 1)
  Local x1%(NUM_LINES - 1), y1%(NUM_LINES - 1), x2%(NUM_LINES - 1), y2%(NUM_LINES - 1)
  For i% = 0 To NUM_LINES - 1
    col%(i%) = COLOURS(Int(NUM_COLOURS * Rnd()))
    x1%(i%) = (Mm.Info(HRes) + 40) * Rnd() - 20
    x2%(i%) = (Mm.Info(HRes) + 40) * Rnd() - 20
    y1%(i%) = (Mm.Info(VRes) + 40) * Rnd() - 20
    y2%(i%) = (Mm.Info(VRes) + 40) * Rnd() - 20
    width%(i%) = Int((10 * Rnd()) + 1)
    If i% Mod 2 Then x2%(i%) = x1%(i%) Else y2%(i%) = y1%(i%)
  Next
  Line x1%(), y1%(), x2%(), y2%(), width%(), col%()
End Sub

Sub draw_random_line_array()
  Local col%(NUM_LINES - 1), i%
  Local x1%(NUM_LINES - 1), y1%(NUM_LINES - 1), x2%(NUM_LINES - 1), y2%(NUM_LINES - 1)
  For i% = 1 To NUM_LINES - 1
    col%(i%) = COLOURS(Int(NUM_COLOURS * Rnd()))
    x1%(i%) = (Mm.Info(HRes) + 40) * Rnd() - 20
    x2%(i%) = (Mm.Info(HRes) + 40) * Rnd() - 20
    y1%(i%) = (Mm.Info(VRes) + 40) * Rnd() - 20
    y2%(i%) = (Mm.Info(VRes) + 40) * Rnd() - 20
  Next
  Line x1%(), y1%(), x2%(), y2%(), 1, col%()
End Sub

Sub draw_line_graph()
  Local angle!, i%
  Local x%(Mm.Info(HRes) / 40), y%(Mm.Info(HRes) / 40)
  For i% = 0 To Mm.Info(HRes) / 40
    x%(i%) = i% * 40
    angle! = 2 * Pi * x%(i%) / Mm.Info(HRes)
    y%(i%) = (Sin(angle!) * 0.49 + 0.5) * Mm.Info(VRes)
  Next
  Line Graph x%(), y%(), Rgb(Green)
End Sub
