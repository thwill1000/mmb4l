Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, 640, 480
  Graphics Write 0
EndIf

Const NUM_COLOURS = 7
Const NUM_PIXELS = 100

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
Data "draw_pixels", "draw_pixel_array", ""

Sub draw_pixels()
  Local c%, i%, x%, y%
  For i% = 1 To 100
    c% = COLOURS(Int(NUM_COLOURS * Rnd()))
    x% = (Mm.Info(HRes) + 40) * Rnd() - 20
    y% = (Mm.Info(VRes) + 40) * Rnd() - 20
    Pixel x%, y%, c%
  Next
End Sub

Sub draw_pixel_array()
  Local c%(NUM_PIXELS - 1), i%, x%(NUM_PIXELS - 1), y%(NUM_PIXELS - 1)
  For i% = 0 To NUM_PIXELS - 1
    c%(i%) = COLOURS(Int(NUM_COLOURS * Rnd()))
    x%(i%) = (Mm.Info(HRes) + 40) * Rnd() - 20
    y%(i%) = (Mm.Info(VRes) + 40) * Rnd() - 20
  Next
  Pixel x%(), y%(), c%()
End Sub
