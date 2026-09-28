Option Simulate PicoMiteVGA
Mode 2
Font 7
Dim i%
' ? Mm.HRes / Mm.Info(FontWidth)
' ? Mm.VRes / Mm.Info(FontHeight)
' End
Do
  If i% > 0 And i% Mod 52 = 0 Then Print
  Print Chr$(Asc("a") + i% Mod 26);
  Inc i%
  Pause 10
Loop
