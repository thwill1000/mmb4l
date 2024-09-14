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

CLS
Sprite set transparent 0
Dim integer xx, yy, rr
Dim integer x(NUM_SPRITES),y(NUM_SPRITES),c(NUM_SPRITES)
Dim float direction(NUM_SPRITES)
Dim integer i,j,k, collision=0
Dim string q$

' Drive "b:"
For i = 1 To NUM_SPRITES
  direction(i)=Rnd*360 'establish the starting direction for each atom
  c(i)=RGB(Rnd*255,Rnd*255,Rnd*255) 'give each atom a colour
  Circle 20,20,SPRITE_RADIUS,1,,RGB(white),c(i) 'draw the atom
  j = 20 - SPRITE_RADIUS
  k = 2 * SPRITE_RADIUS + 1
  Sprite Read BASE_SPRITE + i, j, j, k, k 'read it in as a sprite
Next

CLS RGB(myrtle)
Box 0,0,MM.HRes,MM.VRes
k = 1
Const X_DELTA = Mm.HRes \ (COLUMNS + 1)
Const Y_DELTA = Mm.VRes \ (ROWS + 1)
For i = X_DELTA To X_DELTA * COLUMNS Step X_DELTA
  For j = Y_DELTA To Y_DELTA * ROWS Step Y_DELTA
    If k > NUM_SPRITES Then Exit For
    Sprite Show k + BASE_SPRITE,i,j,1
    x(k)=i
    y(k)=j
    vector k,direction(k), 0, x(k), y(k) 'load up the vector move
    k=k+1
  Next j
Next i

If InStr(Mm.Device$, "PicoMite") Then FrameBuffer Copy F, N Else Graphics Copy 1 To 0
Pause 1000

Do
  For i=1 To NUM_SPRITES
    ' Print i, x(i), y(i)
    vector i, direction(i), 1, x(i), y(i)
    Sprite Show i + BASE_SPRITE,x(i),y(i),1
  '   If sprite(S,i)<>-1 Then
    If sprite(C, i + BASE_SPRITE) > 0 Then
  '     Print "***" + Bin$(sprite(T, i), 10)
      break_collision i
    EndIf
  Next i

  If InStr(Mm.Device$, "PicoMite") Then FrameBuffer Copy F, N Else Graphics Copy 1 To 0
  Pause 1
Loop
'
Sub vector(myobj As integer, angle As float, distance As float, x_new As integer, y_new As integer)
Static float y_move(NUM_SPRITES), x_move(NUM_SPRITES)
Static float x_last(NUM_SPRITES), y_last(NUM_SPRITES)
Static float last_angle(NUM_SPRITES)
If distance=0 Then
  x_last(myobj)=x_new
  y_last(myobj)=y_new
EndIf
If angle<>last_angle(myobj) Then
  y_move(myobj)=-Cos(Rad(angle))
  x_move(myobj)=Sin(Rad(angle))
  last_angle(myobj)=angle
EndIf
x_last(myobj) = x_last(myobj) + distance * x_move(myobj)
y_last(myobj) = y_last(myobj) + distance * y_move(myobj)
x_new=Cint(x_last(myobj))
y_new=Cint(y_last(myobj))
Return

' keep doing stuff until we break the collisions
Sub break_collision(atom As integer)
  Const sprite_id = atom + BASE_SPRITE
  Local integer j=1
  Local float current_angle=direction(atom)
  'start by a simple bounce to break the collision
  If sprite(e,sprite_id)=1 Then
    'collision with left of screen
    current_angle=360-current_angle
  ElseIf sprite(e,sprite_id)=2 Then
    'collision with top of screen
      current_angle=((540-current_angle) Mod 360)
  ElseIf sprite(e,sprite_id)=4 Then
    'collision with right of screen
    current_angle=360-current_angle
  ElseIf sprite(e,sprite_id)=8 Then
    'collision with bottom of screen
    current_angle=((540-current_angle) Mod 360)
  Else
    'collision with another sprite or with a corner
    current_angle = current_angle+180
  EndIf
  direction(atom)=current_angle
  vector atom,direction(atom),j,x(atom),y(atom) 'break the collision
  Sprite Show sprite_id,x(atom),y(atom),1
  'if the simple bounce didn't work try a random bounce
  Do While (sprite(t,sprite_id) Or sprite(e,sprite_id)) And j<10
    Do
      direction(atom)= Rnd*360
      vector atom,direction(atom),j,x(atom),y(atom) 'break the collision
      j=j+1
    Loop Until x(atom)>=0 And x(atom)<=MM.HRes-sprite(w,sprite_id) And y(atom)>=0 And y(atom)<=MM.VRes-sprite(h,sprite_id)
    Sprite Show sprite_id,x(atom),y(atom),1
  Loop
  ' if that didn't work then place the atom randomly
  Do While (sprite(t,sprite_id) Or sprite(e,sprite_id))
    direction(atom)= Rnd*360
    x(atom)=Rnd*(MM.HRes-sprite(w,atom))
    y(atom)=Rnd*(MM.VRes-sprite(h,atom))
    vector atom,direction(atom),0,x(atom),y(atom) 'break the collision
    Sprite Show sprite_id,x(atom),y(atom),1
  Loop
End Sub
