Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, 400, 300, , , , 2
  Graphics Write 0
EndIf

Const PATH$ = Mm.Info(Path) + "assets/bmp/valid/"
Const MAX_NAMES = 100

Dim counter% = 0, i% = 0, j% = 0, t% = Timer
Dim filenames$(MAX_NAMES - 1) Length 64
Dim cmd$, f_out$, w%, h%

Dim f$ = Dir$(PATH$ + "*.bmp", File)
' Do While f$ <> ""
'   filenames$(i%) = f$
'   Inc i%
'   f$ = Dir$()
' Loop

' Sort filenames$()

'filenames$(0) = "565-1x1.bmp"
filenames$(0) = "BaboonRGB.bmp"

Dim formats$(4) = ("rgb121", "rgb121_rle4", "rgb222", "rgb332", "24bpp")

For i% = 0 To MAX_NAMES - 1
  f$ = filenames$(i%)
  If Len(f$) = 0 Then Continue For
  get_bmp_size(PATH$ + f$, w%, h%)
  Cls Rgb(Grey)
  ? "LOAD BMP " + Chr$(34) + f$ + Chr$(34)
  On Error Skip
  Load Bmp PATH$ + f$, 20, 20
  If Mm.ErrNo Then Print "ERROR: " + Mm.ErrMsg$
  Text 0, 0, "File: " + f$,,,,, Rgb(Grey)
  ' Do While Inkey$ <> "" : Loop
  ' Do While Inkey$ = "" : Loop
  Pause 2000

  ' Save image in all supported formats.
  For j% = 0 To Bound(formats$(), 1)
    f_out$ = formats$(j%) + "-" + f$
    cmd$ = "SAVE " + UCase$(formats$(j%)) + Chr$(34) + f_out$ + Chr$(34)
    Cat cmd$, " , 20, 20, " + Str$(w%) + ", " + Str$(h%)
    ? cmd$
    Execute cmd$
  Next j%

  ' Load image in all supported formats.
  For j% = 0 To Bound(formats$(), 1)
    Cls Rgb(Grey)
    Pause 100

    f_out$ = formats$(j%) + "-" + f$
    ? "LOAD BMP " + Chr$(34) + f_out$ + Chr$(34)
    Load Bmp f_out$, 20, 20
    If Mm.ErrNo Then Print "ERROR: " + Mm.ErrMsg$
    Text 0, 0, "File: " + f_out$,,,,, Rgb(Grey)

    Pause 1000
  Next

  Inc counter%
Next

? "Time: " + Str$(Timer - t% - 1000 * counter%) + " ms"
? "Press Ctrl-C to exit"
Do : Loop

Sub get_bmp_size(f$, ByRef w%, ByRef v%)
  Open f$ For Input As #1
  Const s$ = Input$(54, #1)
  Close #1
  Const ad% = Peek(VarAddr s$) + 1
  w% = Abs(int32%(peek_unaligned_word%(ad% + 18)))
  h% = Abs(int32%(peek_unaligned_word%(ad% + 22)))
End Sub

Function peek_unaligned_word%(ad%)
  peek_unaligned_word% = Peek(Byte ad%) + (Peek(Byte ad%+1) << 8)
  Inc peek_unaligned_word%, (Peek(Byte ad%+2) << 16) + (Peek(Byte ad%+3) << 24)
End Function

Function int32%(u32%)
  If u32% And &hFFFFFFFF00000000 Then Error "Unsigned 32-bit out of range: " + u32%
  int32% = Choice(u32% And &h80000000, -((u32% Xor &hFFFFFFFF) + 1), u32%)
End Function
