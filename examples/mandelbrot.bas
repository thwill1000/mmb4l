' Demonstration of CONSOLE commands for MMB4L

Option Base 0
Option Default None
Option Explicit On

Dim width%, height%

Console SetTitle "Mandelbrot"

Dim count%, key%, redraw%, curs_x%, curs_y%

update_display()

Do
  key% = Asc(Inkey$)

  Select Case key%
    Case &h80: Inc curs_y%, -1 : redraw% = 1
    Case &h81: Inc curs_y%,  1 : redraw% = 1
    Case &h82: Inc curs_x%, -1 : redraw% = 1
    Case &h83: Inc curs_x%,  1 : redraw% = 1
  End Select

  Inc count%
  If count% = 10000 Then redraw% = 1

  If redraw% Then
    update_display()
    redraw% = 0
    count% = 0
  EndIf
Loop

Sub update_display()
  If size_changed%() Then draw_mandelbrot()

  If curs_x% < 0 Then curs_x% = 0
  If curs_x% >= width% Then curs_x% = width% - 1
  If curs_y% < 0 Then curs_y% = 0
  If curs_y% >= height% Then curs_y% = height% - 1

  Local s$ = "Rows: " + Str$(height%) + ", Cols: " + Str$(width%)
  Cat s$, ", x: " + Str$(curs_x%) + ", y: " + Str$(curs_y%)
  s$ = Left$(s$, Min(width%, 255))
  s$ = s$ + Space$(Max(0, Min(width%, 255) - Len(s$)))

  Console HideCursor
  Console SetCursor 0, height% - 1
  Console Foreground White
  Console Background Black
  Print s$;
  Console SetCursor curs_x%, curs_y%
  Console ShowCursor

  Local real_x%, real_y%
  Console GetCursor real_x%, real_y%
  If curs_x% <> real_x% Then Error "X-cursor pos inconsistent " + Str$(curs_x%) + " vs. " + Str$(real_x%)
  If curs_y% <> real_y% Then Error "Y-cursor pos inconsistent " + Str$(curs_y%) + " vs. " + Str$(real_y%)
End Sub

Sub draw_mandelbrot()
  Local it%, max_it%, x!, x0!, y!, y0!, xtemp!

  Console Reset
  Console HideCursor
  Console Clear

  For x0! = -2 To 2 Step .01
    For y0! = -1.5 To 1.5 Step .01
      x! = 0
      y! = 0
      it% = 0
      max_it% = 8

      Do While (x! * x! + y! * y! <= (2 * 2) And it% < max_it%)
        xtemp! = x! * x! - y! * y! + x0!
        y! = 2 * x! * y! + y0!
        x! = xtemp!
        Inc it%
      Loop

      Console Background Choice(it% = max_it%, 0, it%)
      Console SetCursor (width% / 4) * (x0! + 2), (height% / 3) * (y0! + 1.5)
      Print " ";
    Next

    If size_changed%() Then
      x0! = -2
      Console Reset
      Console Clear
    EndIf

  Next
End Sub

Function size_changed%()
  Local new_width%, new_height%
  Console GetSize new_width%, new_height%
  size_changed% = (new_width% <> width%) Or (new_height% <> height%)
  width% = new_width%
  height% = new_height%
End Function
