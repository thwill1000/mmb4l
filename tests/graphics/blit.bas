Option Base 0
Option Default None
Option Explicit

If Mm.Device$ <> "MMB4L" Then Error "Test for MMB4L only"

Graphics Window 0, 640, 480
Graphics Write 0

' Test CLOSE and CLOSE ALL.

On Error Skip
Blit Close 0
'If Not InStr(Mm.ErrMsg$, "Use GRAPHICS DESTROY to close windows") Then Error "Test failed"
If Not InStr(Mm.ErrMsg$, "Invalid graphics surface") Then Error "Test failed"

Blit Close All

check_surface_closed(1)
check_surface_closed(2)
check_surface_closed(3)

Blit Read 1, 0, 0, 100, 100
Blit Read #2, 0, 0, 100, 100
Blit Read 3, 0, 0, 100, 100

Blit Close 1
Blit Close #2
Blit Close 3

check_surface_closed(1)
check_surface_closed(2)
check_surface_closed(3)

Blit Read 1, 0, 0, 100, 100
Blit Read #2, 0, 0, 100, 100
Blit Read 3, 0, 0, 100, 100

Blit Close All

check_surface_closed(1)
check_surface_closed(2)
check_surface_closed(3)

Print "SUCCESS - press any key to exit"
Do While Inkey$ = "" : Loop

Sub check_surface_closed(i%)
  On Error Skip
  Blit Close i%
  If Not InStr(Mm.ErrMsg$, "Invalid graphics surface") Then Error "Test failed"
End Sub
