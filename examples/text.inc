' Copyright (c) 2020 Thomas Hugo Williams
' For Maximite BASIC v4.5C

' LED font display library.

' Makes use of a global font4x6() array created by calling text_init().

Dim text.interrupt% = 0

Sub text_init()
  Local end_%, height%, i%, j%, s$, start%, x%, width%

  Open Mm.Info$(Path) + "tt4x6.fnt" For Input As #1

  Input #1, height%, width%, start%, end_%
  If height% <> 6 Then Error
  If width% <> 4  Then Error
  If start% <> 20 Then Error
  If end_% <> 126  Then Error

  ' This is actually more storage than is required because it allocates
  ' 4 bytes per glyph and we only need 3.
  Dim font4x6%(end_% - start%)

  For i% = 0 To (end_% - start%) ' Iterate over glyphs
    For j% = 0 To 5 Step 2 ' Iterate over rows, 2 at a time
      x% = 0
      Line Input #1, s$
      If Len(s$) <> width% Then Error
      If Peek(Var s$, 1) > 32 Then x% = x% Or &b00000001
      If Peek(Var s$, 2) > 32 Then x% = x% Or &b00000010
      If Peek(Var s$, 3) > 32 Then x% = x% Or &b00000100
      If Peek(Var s$, 4) > 32 Then x% = x% Or &b00001000
      Line Input #1, s$
      If Len(s$) <> width% Then Error
      If Peek(Var s$, 1) > 32 Then x% = x% Or &b00010000
      If Peek(Var s$, 2) > 32 Then x% = x% Or &b00100000
      If Peek(Var s$, 3) > 32 Then x% = x% Or &b01000000
      If Peek(Var s$, 4) > 32 Then x% = x% Or &b10000000
      Poke Var font4x6%(0), i% * 3 + j% \ 2, x%
    Next j%
  Next i%
  Close #1
End Sub

Sub text_scroll(s$)
  Local c%, ch%, i%
  text.interrupt% = 0

  For i% = 1 To Len(s$)
    ch% = Peek(Var s$, i%)
    For c% = 0 To 3
      ' Poor man's proportional spacing.
      If c% = 0 And ch% = 105 Then c% = 1
      If c% = 2 And ch% = 105 Then c% = 3
      If c% = 1 And ch% = 32  Then c% = 4
      text_blit(Peek(Var s$, i%), c%)
      Pause 150
      matrix_left()
    Next c%
    If text.interrupt% Then Exit For
  Next

  For i% = 1 To matrix_nc%
    If text.interrupt% Then Exit For
    Pause 100
    matrix_left()
  Next

  If text.interrupt% Then matrix.wipe()
End Sub

' Copies column 'c' of glyph 'a' to the last column of the matrix.
Sub text_blit(a%, c%)
  Local i%, p%, s%, z%

  p% = 1
  s% = (a% - 20) * 3
  For i% = s% To s% + 2
    z% = Peek(Var font4x6%(0), i%)
    matrix_set(matrix_nc% - 1, p%, (z% And 2^c%) > 0)
    p% = p% + 1
    matrix_set(matrix_nc% - 1, p%, (z% And 2^(c%+4)) > 0)
    p% = p% + 1
  Next
  matrix.redraw% = 1
End Sub
