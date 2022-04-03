Option Explicit On
Option Default None
Option Base 0

Dim codepages$(5) = ( "None", "CMM2", "CP437", "CP1252", "MMB4L", "None" )
Dim i%, j%

For i% = 0 To Bound(codepages$(), 1)
  Print codepages$(i%)
  Option Codepage codepages$(i%)
  For j% = &h20 To &hFF
    Print Chr$(j%) " ";
    If (j% + 1) Mod 32 = 0 Then Print
  Next
  Print : Print
Next
