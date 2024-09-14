Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, 400, 300, , , , 2
  Graphics Write 0
EndIf

Const PATH$ = Mm.Info(Path) + "/assets/bmp/valid/"
Const MAX_NAMES = 100

Dim counter% = 0, i% = 0, t% = Timer
Dim filenames$(MAX_NAMES - 1) Length 64

Dim f$ = Dir$(PATH$ + "*.bmp", File)
Do While f$ <> ""
  filenames$(i%) = f$
  Inc i%
  f$ = Dir$()
Loop

Sort filenames$()

For i% = 0 To MAX_NAMES - 1
  f$ = filenames$(i%)
  If Len(f$) = 0 Then Continue For
  Cls Rgb(Grey)
  ? "LOAD BMP " + Chr$(34) + f$ + Chr$(34)
  On Error Skip
  Load Bmp PATH$ + f$, 20, 20
  If Mm.ErrNo Then Print "ERROR: " + Mm.ErrMsg$
  ' Do While Inkey$ <> "" : Loop
  ' Do While Inkey$ = "" : Loop
  Pause 1000
  Inc counter%
Next

? "Time: " + Str$(Timer - t% - 1000 * counter%) + " ms"
? "Press Ctrl-C to exit"
Do : Loop
