' Original "Brownian Motion" sprite demo by Peter Mather.

Option Base 0
Option Default None
Option Explicit On

If Mm.Device$ = "MMB4L" Then
  If InStr(Mm.CmdLine$, "--gamemite") Then
    Option Simulate "Game*Mite"
  ElseIf InStr(Mm.CmdLine$, "--pmvga") Or InStr(Mm.CmdLine$, "--picomitevga") Then
    Option Simulate "PicoMiteVGA"
  ElseIf InStr(Mm.CmdLine$, "--cmm2") Then
    Option Simulate "Colour Maximite 2"
  EndIf
EndIf

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, 1024, 768
  Graphics Buffer 1, 1024, 768
  Graphics Write 1
  Const BASE_SPRITE = 1
  Const SPRITE_RADIUS = 10
  Const NUM_SPRITES = 254
  Const ROWS = 16
  Const COLUMNS = 16
ElseIf InStr(Mm.Device$, "PicoMite") Then
  If Mm.Device$="PicoMiteVGA" Then Mode 2
  FrameBuffer Create
  FrameBuffer Write F
  Const BASE_SPRITE = 0
  Const SPRITE_RADIUS = 4
  Const NUM_SPRITES = 64
  Const ROWS = 8
  Const COLUMNS = 8
ElseIf InStr(Mm.Device$, "Colour Maximite 2") Then
  Mode 7
  Page Write 1
  Const BASE_SPRITE = 0
  Const SPRITE_RADIUS = 4
  Const NUM_SPRITES = 64
  Const ROWS = 8
  Const COLUMNS = 8
EndIf

Dim atom.x%(NUM_SPRITES), atom.y%(NUM_SPRITES), atom.c%(NUM_SPRITES) ' Element 0 is unused.

Sprite Set Transparent 0

create_sprites()

Cls Rgb(Myrtle)
Box 0, 0, Mm.HRes, Mm.VRes

' test_sprite_hide()
' test_sprite_hide_all()
' test_sprite_restore()
test_sprite_scroll()

Pause 2000
End

Sub create_sprites()
  Local i%, h%, w%, x%, y%
  Cls
  For i% = 1 To NUM_SPRITES
    atom.c%(i%) = Rgb(Rnd()*255, Rnd()*255, Rnd()*255)
    Circle 20, 20, SPRITE_RADIUS, 1, , Rgb(White), atom.c%(i%)
    x% = 20 - SPRITE_RADIUS : y% = x%
    w% = 2 * SPRITE_RADIUS + 1 : h% = w%
    Sprite Read BASE_SPRITE + i%, x%, y%, w%, h%
  Next
End Sub

Sub set_sprite_start_positions()
  Const X_DELTA = Mm.HRes \ (COLUMNS + 1)
  Const Y_DELTA = Mm.VRes \ (ROWS + 1)
  Local i% = 1, x% = X_DELTA, y% = Y_DELTA
  For i% = 1 To NUM_SPRITES
    atom.x%(i%) = x%
    atom.y%(i%) = y%
    Inc y%, Y_DELTA
    If y% > Y_DELTA * ROWS Then
      Inc x%, X_DELTA
      y% = Y_DELTA
    EndIf
  Next
End Sub

Sub show_sprites()
  Const LAYER = 1
  Local i%
  For i% = 1 To NUM_SPRITES
    Sprite Show i% + BASE_SPRITE, atom.x%(i%), atom.y%(i%), LAYER
  Next
  copy_buffer_to_display()
End Sub

Sub draw_background()
  Local c%, i%, x%, y%, r%
  For i% = 1 To 100
    x% = Rnd() * Mm.HRes
    y% = Rnd() * Mm.VRes
    r% = Rnd() * 100 + 20
    c% = Rgb(Rnd()*255, Rnd()*255, Rnd()*255)
    Circle x%, y%, r%, , , , c%
  Next
  copy_buffer_to_display()
End Sub

Sub copy_buffer_to_display()
  If InStr(Mm.Device$, "PicoMite") Then FrameBuffer Copy F, N Else Graphics Copy 1 To 0
End Sub

Sub test_sprite_hide()
  Local i%

  set_sprite_start_positions()
  show_sprites()
  Pause 500

  For i% = 1 To NUM_SPRITES
    Sprite Hide i% + BASE_SPRITE
    copy_buffer_to_display()
    Pause 250
  Next

  On Error Skip
  Sprite Hide 1 + BASE_SPRITE
  expect_error(Choice(Mm.Info$(Device X) = "MMB4L", "Sprite not showing", "Not showing"))
End Sub

Sub test_sprite_hide_all()
  set_sprite_start_positions()
  show_sprites()
  Pause 500
  Sprite Hide All
  copy_buffer_to_display()

  On Error Skip
  Sprite Hide All
  expect_error("Sprites are hidden")
End Sub

Sub test_sprite_restore()
  set_sprite_start_positions()
  show_sprites()
  Pause 500
  ? "*** SPRITE HIDE ALL"
  Sprite Hide All
  copy_buffer_to_display()
  Pause 500

  ? "*** SPRITE RESTORE"
  Sprite Restore
  copy_buffer_to_display()
End Sub

Sub test_sprite_scroll()
  Local col%(3) = (0, Rgb(White), -1, -2), i%, j%, k%
  If InStr(Mm.Device$, "PicoMite") Then col%(1) = 15
  For i% = 1 To 3
    For j% = 1 To 4
      set_sprite_start_positions()
      Sprite Hide All
      draw_background()
      Sprite Restore
      show_sprites()
      Pause 500
      ? i%, j%

      For k% = 1 To 20
        Select Case j%
          Case 1
            Sprite Scroll 10, 0, col%(i%)
          Case 2
            Sprite Scroll -10, 0, col%(i%)
          Case 3
            Sprite Scroll 0, 10, col%(i%)
          Case 4
            Sprite Scroll 0, -10, col%(i%)
        End Select
        copy_buffer_to_display()
        Pause 100
      Next
    Next
  Next
End Sub

Sub expect_error(expected$)
  Const colon_pos% = InStr(Mm.ErrMsg$, ":")
  Const actual$ = Choice(colon_pos, Mid$(Mm.ErrMsg$, colon_pos% + 2), Mm.ErrMsg$)
  If expected$ <> actual$ Then
    Print "Expected error: " + expected$
    Print "Received error: " + Mm.ErrMsg$
    End
  EndIf
End Sub
