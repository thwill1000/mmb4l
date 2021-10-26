' Copyright (c) 2020 Thomas Hugo Williams
' For Maximite BASIC v4.5C

' LED "Snake" game.

Option Base 0
Option Default None
Option Explicit On

#Include "matrix.inc"
#Include "text.inc"

Const USE_M7219% = 1
Const CMD_UP% = 1, CMD_RIGHT% = 2, CMD_DOWN% = 3, CMD_LEFT% = 4
Const CMD_SELECT% = 10, CMD_START% = 20, CMD_DEMO% = 30

Console HideCursor

Cls
Print "LED SNAKE - Tom Williams 2020-2021"
Print "Cursor keys to move"
Print "[S] to START"
Print "[D] to SELECT difficulty"
Print "[E] to start DEMO"
Print "[Q] to QUIT"

text_init()

If USE_M7219% Then m7219_init(32, 16) Else matrix_init()

Dim SNAKE_SZ% = matrix_w% * matrix_h%
Dim snake%(SNAKE_SZ% - 1, 1)
Dim head%, tail%
Dim demo% = 0, diff% = 1
Dim cmd%, dir%, pill_x%, pill_y%, score%

' One of the purposes of this program is to test interrupt handling in MMB4L
' hence it uses interrupts to drive the screen update and also read the
' keyboard. A more "conventional" implementation might have updated/polled
' as part of the main game loop.
If USE_M7219% Then SetTick 2, m7219_draw Else SetTick 2, matrix_draw
On Key on_key
On Key Asc("q"), on_quit

main_loop()
End

Sub main_loop()
  Do
    matrix_clear()
    game_intro()
    game_init()
    game_loop()
    die()
  Loop
End Sub

Sub game_intro()
  Local s$

  s$ = "SNAKE-Tom Williams 2020-SELECT to increase difficulty"

  Do
    If cmd% = CMD_SELECT% Then
      cmd% = 0
      diff% = 1 + diff% Mod 4
      text_scroll("Difficulty: " + Str$(diff%))
    Else
      text_scroll(s$)
      If cmd% = 0 Then cmd% = CMD_DEMO%
    EndIf
  Loop Until cmd% <> 0 And cmd% <> CMD_SELECT%
  demo% = (cmd% = CMD_DEMO%)
End Sub

Sub game_init()
  Local i%

  For i% = 0 To SNAKE_SZ% - 1
    snake%(i%, 0) = 0
    snake%(i%, 1) = 0
  Next
  head% = 1
  tail% = 0
  snake%(head%, 0) = (matrix_w% \ 2) - 1
  snake%(head%, 1) = (matrix_h% \ 2) - 1
  matrix_set(snake%(head%, 0), snake%(head%, 1), 1)
  snake%(tail%, 0) = snake%(head%, 0)
  snake%(tail%, 1) = snake%(head%, 1) + 1
  matrix_set(snake%(tail%, 0), snake%(tail%, 1), 1)
  dir% = CMD_UP%
  pill_x% = -1 : pill_y% = -1
  score% = 0
  cmd% = 0
  place_pill()
End Sub

Sub game_loop()
  Local x%, y%

  Do
    Pause (5 - diff%) * 100

    If demo% Then
      If cmd% <> 0 Then Exit Sub
      demo_ai()
    ElseIf cmd% <> 0 And cmd% < 10 Then
      dir% = cmd%
    EndIf
    cmd% = 0

    If will_die%() Then Exit Sub
    x% = snake%(head%, 0)
    y% = snake%(head%, 1)
    snake_move(x%, y%)
    head% = (head% + 1) Mod SNAKE_SZ%
    snake%(head%, 0) = x%
    snake%(head%, 1) = y%
    matrix_set(x%, y%, 1)

    If x% = pill_x% And y% = pill_y% Then
      score% = score% + 1
      pill_x% = -5 : pill_y% = -5
    Else
      matrix_set(snake%(tail%, 0), snake%(tail%, 1), 0)
      tail% = (tail% + 1) Mod SNAKE_SZ%
    EndIf

    If pill_x% = -1 Then place_pill()
    If pill_x% < -1 Then pill_x% = pill_x% + 1

    matrix.redraw% = 1
  Loop

End Sub

Sub snake_move(x%, y%)
  If dir% = CMD_UP% Then
    y% = y% - 1
  ElseIf dir% = CMD_DOWN% Then
    y% = y% + 1
  ElseIf dir% = CMD_LEFT% Then
    x% = x% - 1
  ElseIf dir% = CMD_RIGHT% Then
    x% = x% + 1
  EndIf
End Sub

Sub demo_ai()
  Local x%, y%

  x% = snake%(head%, 0)
  y% = snake%(head%, 1)

  If pill_x% > -1 Then
    If y% = pill_y% Then
      If x% > pill_x% Then dir% = CMD_LEFT%
      If x% < pill_x% Then dir% = CMD_RIGHT%
    ElseIf x% = pill_x% Then
      If y% > pill_y% Then dir% = CMD_UP%
      If y% < pill_y% Then dir% = CMD_DOWN%
    ElseIf pill_y% < y% Then
      dir% = CMD_UP%
    Else
      dir% = CMD_DOWN%
    EndIf
  EndIf

  If Not will_die%() Then Exit Sub
  If (y% > matrix_h% \ 2) Then dir% = CMD_UP% Else dir% = CMD_DOWN%
  If Not will_die%() Then Exit Sub
  If (x% > matrix_w% \ 2) Then dir% = CMD_LEFT% Else dir% = CMD_RIGHT%
  If Not will_die%() Then Exit Sub
  If (y% > matrix_h% \ 2) Then dir% = CMD_DOWN% Else dir% = CMD_UP%
  If Not will_die%() Then Exit Sub
  If (x% > matrix_w% \ 2) Then dir% = CMD_RIGHT% Else dir% = CMD_LEFT%
End Sub

Function will_die%()
  Local x%, y%

  x% = snake%(head%, 0)
  y% = snake%(head%, 1)
  snake_move(x%, y%)

  If y% < 0 Or y% >= matrix_h% Or x% < 0 Or x% >= matrix_w% Then
    will_die% = 1
  ElseIf matrix_get%(x%, y%) And Not (x% = pill_x% And y% = pill_y%) Then
    will_die% = 1
  EndIf
End Function

Sub die()
  Local i%, j%
  For i% = 1 To 4
    matrix_fill()
    Pause 300
    matrix_clear()
    Pause 300
  Next
  text_scroll("GAME OVER Score:" + Str$(score%))
End Sub

Sub place_pill()
  pill_x% = Int(Rnd() * matrix_w%)
  pill_y% = Int(Rnd() * matrix_h%)
  If matrix_get%(pill_x%, pill_y%) = 0 Then
    matrix_set(pill_x%, pill_y%, 1)
  Else
    pill_x% = -1 : pill_y% = -1
  EndIf
End Sub

Sub on_key()
  Select Case UCase$(Inkey$)
    Case "S" :        cmd% = CMD_START%  : text.interrupt% = 1
    Case "D" :        cmd% = CMD_SELECT% : text.interrupt% = 1
    Case "E" :        cmd% = CMD_DEMO%   : text.interrupt% = 1
    Case Chr$(&h80) : cmd% = CMD_UP%     : text.interrupt% = 1
    Case Chr$(&h81) : cmd% = CMD_DOWN%   : text.interrupt% = 1
    Case Chr$(&h82) : cmd% = CMD_LEFT%   : text.interrupt% = 1
    Case Chr$(&h83) : cmd% = CMD_RIGHT%  : text.interrupt% = 1
  End Select
End Sub

Sub on_quit()
  Console Foreground White
  Print "Goodbye!"
  End
End Sub
