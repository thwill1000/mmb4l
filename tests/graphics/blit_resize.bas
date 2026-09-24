Dim integer mmb4w
Dim integer Bcl(16,160),speed=3
phase
' --- Initialisierung ---
CLS
' --- Init_Graphics: Setup 320x240 Resolution, Layer-Buffer).
If MM.DEVICE$ = "MMBasic for Windows" Then mmb4w = 1
If mmb4w Then
MODE 7
PAGE WRITE 1
CLS
Else
'Pico
MODE 2
FRAMEBUFFER create : FRAMEBUFFER write f
End If
' 2. Z-Table
Dim integer x,y, z_dist(38)
For y = 0 To 37
  Read scal:z_dist(y)=16+(scal/4)
Next y

CLS 'clear FRAMEBUFFER
Dim integer hh=120,hw=160,f,n,w
Dim integer c1=RGB(0,180,0),c2=RGB(White)
Dim Integer sky=RGB(0,85,255),sky1=RGB(0,170,255),sky2=RGB(cyan)
' --- Create_Checkerboard Source
For f =0 To 288 Step 32
Line f,hh-1,f+15,hh-1,,c1:Line f+16,hh-1,f+31,hh-1,,c2
Line f,hh,f+15,hh,,c2:Line f+16,hh,f+31,hh,,c1
Next
Box 8,16,304,72,,sky,sky
Line 8,51,311,51,,sky1:Line 8,52,311,52,,sky2
main:
p=0
Do
For y = 0 To 37
mult=y<<1
 f=z_dist(y):sy=y+53:w=200-f*2-y2'w=144-mult
 If mmb4w Then
    IMAGE RESIZE_FAST f+camX,hh-Bcl(p,y+50), w,1, 8, y+53,304,1
  Else
      BLIT RESIZE f,f,f+camX,hh-Bcl(p,y+50), w,1, 8, y+53,304,1
  End If
Next y
   ' --- Keyboard ---
   k$ = Inkey$
   Select Case k$
       Case "w", "W"
          Inc P,speed:p=p Mod 15   ' vorwrts

       Case "s", "S"
           p=p-speed:If p<0 Then p=15

       Case "a", "A"
           camX = camX - speed:If camX<0 Then camX=31

       Case "d", "D"
            camX = camX + speed:camX = camX Mod 32
   End Select
If mmb4w Then
Page copy 1,0
'Page write 0
Else
 FRAMEBUFFER copy f,n,b
End If
'pause 20
Loop
Sub phase
  'Precalculation pharse lookup table
   Local integer X,Y, SW = 120,CYCLES = 7
   Local C_VALUE = (2 * Pi * CYCLES) / (SW * SW),START_PHASE = 0,Agl
   For dst=0 To 15
     For X = 0 To 119
       Agl = C_VALUE * X * X + START_PHASE
       Y = Int(Sin(Agl)*5)
       bcl(dst,120-x)=Not(y>0)
     Next X
     Inc START_PHASE, 0.393
   Next
End Sub

Data 36, 39, 43, 46, 50, 54, 57, 61, 64, 68, 72,75, 79, 82, 86
Data 89, 93, 97, 100, 104, 107, 111, 115, 118, 122, 125, 129
Data 133, 136, 140, 143, 147, 151, 154, 158, 161, 165, 169
