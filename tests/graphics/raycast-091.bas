' Raycaster 2 in MMBasic, DDA method (Digital Differential Analyser)
' Picomite Firmware at least V6.02.01 RC1
' V0.91.. added different tiles
' added Framebuffer Resize/Image RESIZE_FAST command
' added View-Bobbing, Lookuptables
'
'By Martin H. for https://www.thebackshed.com
'The software is subject to the terms of the GNU General Public Licence (GPL)
'and may be freely used, copied and updated.
'Individuals are encouraged to freely use and modify the software.
' --- Initialisierung ---
Dim integer dither%=1, mmb4w
If MM.DEVICE$ = "MMBasic for Windows" Then mmb4w = 1
CLS
If mmb4w Then
 MODE 7 : CLS
 PAGE WRITE 1
 cls
Else
 ' Dein bewhrter Modus fr den Pico
 MODE 2 : CLS
 FRAMEBUFFER create : FRAMEBUFFER write f
End If
' --- Konstanten & Tabellen ---
Const LUTSIZE=360, MULT=128
Dim integer sinLUT(LUTSIZE), cosLUT(LUTSIZE)
For i = 0 To LUTSIZE - 1
 angle = i * (2 * Pi / LUTSIZE)
 sinLUT(i) = Int(Sin(angle)*MULT)
 cosLUT(i) = Int(Cos(angle)*MULT)
Next
Const TEX_SIZE = 64
'Dim integer tex(TEX_SIZE-1, TEX_SIZE-1)
Dim integer tb(3,1,1)

' --- Labyrinth Definition ---

Dim integer mapW = 64, mapH = 51
Dim integer m(mapW*mapH)
Restore MapData1
Dim k$ length 64
For y = 0 To mapH-1 : Read k$
 For x=0 To Len(k$)-1 : m(x + (y << 6))=Val(Mid$(k$,x+1,1)) : Next
Next

' --- Player Setup ---
Dim px = 26.5, py = 45.5, angleIndex = 0
moveSpeed = 0.4 : rotStep = 8

' --- Grafik Setup ---
Dim integer resStep = 2 ' Wichtig bei Pico: 4 oder 6 fr flssige Bewegung
Dim integer scrW = MM.HRES - 96,dex=32
Dim integer scrH = MM.VRES -64,dey=8
Dim integer halfScrH = scrH / 2
Dim invScrW = 2 / scrW
Dim integer needsRedraw = 1
Dim integer walk(8)=(0,1,2,2,1,-1,-1,-2,-1),wlk
create_texture
' --- main loop ---
Do
 If needsRedraw Then
   dx = cosLUT(angleIndex)/MULT : dy = sinLUT(angleIndex)/MULT
   planeX = -dy * 0.66 : planeY = dx * 0.66
   tt = Timer

   ' Sky and ground
   Box 0, 0, scrW, halfScrH, 0, 0,0
   Box 0, halfScrH, scrW, halfScrH, 0, 0, RGB(0,64,0)

   ' Raycasting
   For x = 0 To scrW - 1 Step resStep
     cameraX = x * invScrW - 1
     rayDx = dx + planeX * cameraX
     rayDy = dy + planeY * cameraX

     mx = Int(px) : my = Int(py)
     dDx = Abs(1 / (rayDx + 0.000001))
     dDy = Abs(1 / (rayDy + 0.000001))

     If rayDx < 0 Then
       stepX = -1 : sdX = (px - mx) * dDx
     Else
       stepX = 1 : sdX = (mx + 1 - px) * dDx
     End If
     If rayDy < 0 Then
       stepY = -1 : sdY = (py - my) * dDy
     Else
       stepY = 1 : sdY = (my + 1 - py) * dDy
     End If
     indx% = (my << 6) + mx
     ' DDA Loop
     Do While m(indx%) = 0
       If sdX < sdY Then
         Inc sdX, dDx : Inc indx%, stepX : side = 0
       Else
         Inc sdY, dDy : Inc indx%, (stepY << 6) : side = 1
       End If
     Loop
     nide=Not side
     ' Distance
     pDist = nide*(sdX - dDx)+(sdY - dDy)*side
     lH = Int(scrH / Max(0.1, pDist))
     drawY = halfScrH - (lH >> 1)
     ' paint Texture
     wallX =(py + pDist * rayDy)*nide+(px + pDist * rayDx)*Side
     Inc wallX, - Int(wallX)
     texX = Int(wallX * TEX_SIZE)
     '--keep coordinates within their limits for IMAGE RESIZE_fast
     lh=Min(scrh,lh):drawy=Max(0,drawY):d1y=Max(0,drawY+Walk(wlk)/2)
	 sx=tb(m(indx%),0,side):sy=tb(m(indx%),1,side)
   If mmb4w Then
     IMAGE RESIZE_FAST sx+texX, sy,resStep*(64/lh), 64, x, d1y,resStep,lh
   Else
      Blit RESIZE f,f,sx+texX, sy,resStep*(64/lh), 64, x, d1y,resStep,lh
      'Blit RESIZE f,f,scrw+texX,side<<6,resStep*(64/lh),64,x,d1y,resStep,lh
   End IF
  Next x

   ' Frametime toScreen
   If mmb4w Then
     'Blit section from Page 1 to 0
     PAGE WRITE 0 : Blit 0,0, dex, dey,scrW,scrH,1 :
          Text scrW, 0, "FPS: "+Str$(Int(1000/Max(1,Timer-tt)))
     PAGE WRITE 1
   Else
     FRAMEBUFFER write n
     Text scrW, 0, "FPS: "+Str$(Int(1000/Max(1,Timer-tt)))
     FRAMEBUFFER write f
	' Pico Blit section from Framebuffer F to N
     Blit framebuffer F, N, 0, 0, dex, dey, scrW, scrH
   End If
   needsRedraw = 0
 End If

 ' Steuerung
 k$ = UCase$(Inkey$)
 If k$ <> "" Then
   needsRedraw = 1
   dmx = dx * moveSpeed : dmy = dy * moveSpeed
   Select Case k$
     Case "W": If m(Int(py)*64 + Int(px+dmx))=0 Then Inc px,dmx
               If m(Int(py+dmy)*64 + Int(px))=0 Then Inc py,dmy
               wlk=(wlk+2) Mod 8
     Case "S": If m(Int(py)*64 + Int(px-dmx))=0 Then Inc px,-dmx
               If m(Int(py-dmy)*64 + Int(px))=0 Then Inc py,-dmy
               wlk=(wlk+2) Mod 8
     Case "A": angleIndex = (angleIndex - rotStep + LUTSIZE) Mod LUTSIZE
     Case "D": angleIndex = (angleIndex + rotStep) Mod LUTSIZE
   End Select
 End If
Loop Until k$ = Chr$(27)
If Not mmb4w Then FRAMEBUFFER close
'------------------
Sub create_texture
 cls
 load bmp "tiles.bmp",0,scrh
 Load bmp "tile1.bmp",scrw,0
 Load bmp "tile1d.bmp",scrw,64
 'x,y,dark
 tb(1,0,0)=0  :tb(1,1,0)=scrh: tb(1,0,1)=64 :tb(1,1,1)=scrh
 tb(2,0,0)=scrW:tb(2,1,0)=0:tb(2,0,1)=scrW:tb(2,1,1)=64
 tb(3,0,0)=128:tb(3,1,0)=scrh: tb(3,0,1)=192:tb(3,1,1)=scrh

End Sub


MapData1: '
Data "11111111111111111111111133333333333331111111111111111111"
Data "11111111111111111111111133333333333331111111111111111111"
Data "11111111111111111111111130000000000333331111111111111111"
Data "11111000011111111111111130000000000333331111111111111111"
Data "11111000011111110000000000000000000300331111111111111111"
Data "11111000011111110000000030000000000300331111111111111111"
Data "11111110111111110011111130000000000333331111111111111111"
Data "11111000000001110011111130000000000333331111111111111111"
Data "11111000000001110011111133333003333331111111111111111111"
Data "11111000000001000000111133333003333331111111111111111111"
Data "11111000000000000000111111333003333331111111111111111111"
Data "11111000000001000000111111333003311111111111111111111111"
Data "11111000000001111111111333333003333111111111111111111111"
Data "10011111001111111111111333333003333111111111111111111111"
Data "10011111001111111111111300000000333111111111111111111111"
Data "11011111001111111111111300333003333111111111111111111111"
Data "10011111001111111111111300333003333111111111111111111111"
Data "10000000001111111111111300333003333111111111111111111111"
Data "10010000001111111111111333333003311111111111111111111111"
Data "10011111111111111111111111113003111111111111122222222222"
Data "10011000000001111111111111110000111111111111122222222222"
Data "10011000000001111111111100000000000011222222222000000022"
Data "10011000000001111111111000000000000002222222222000000022"
Data "10001000000001111111111000000000000000000000002000000022"
Data "10000000000001111111111000000000000000000000000000000022"
Data "10001000000001111111111000000000000002000000222000000022"
Data "10011000000001111111111000000000000001222002222000000022"
Data "10011000000001111111111111110000111111122002222000000022"
Data "10011111011111111111111111112202211111122002222222222222"
Data "10011111001111111111111111122002211111122002222222222222"
Data "10011111001111111111111111122002211111122002222222222222"
Data "10010001001111111111111111122002211111122002222222222211"
Data "10000001001111111111111111122002211111122002222020022211"
Data "10011111001111111111111111122002211111122000000000000211"
Data "10011111001111111111111111122002211111122000000000002211"
Data "10000000000000010000011111122002211111122022002020220211"
Data "10000000000000000000000111122002211111122222222222222211"
Data "10000000000000010000011111122002211111111111111111111111"
Data "11111111111111110111111122222002222221111111111111111111"
Data "11111111110010000111111222222202222221111111111111111111"
Data "11111111110000011111111200000000000021111111111111111111"
Data "11111111111011111111111200002002000021111111111111111111"
Data "11111111111001111111111200002002000021111111111111111111"
Data "11111111111001111111111222222002222221111111111111111111"
Data "11111111111111111111111200002002000021111111111111111111"
Data "11111111111111111111111200000000000021111111111111111111"
Data "11111111111111111111111200002002000021111111111111111111"
Data "11111111111111111111111222222002222221111111111111111111"
Data "11111111111111111111111200000000000021111111111111111111"
Data "11111111111111111111111200000000000021111111111111111111"
Data "11111111111111111111111222222222222221111111111111111111"
