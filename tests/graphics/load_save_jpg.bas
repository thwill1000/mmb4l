Option Base 0
Option Default None
Option Explicit

If Mm.Device$ = "MMB4L" Then
  Graphics Window 0, 512, 512
  Graphics Write 0
EndIf

Const PATH$ = Mm.Info(Path) + "/assets/jpg"

? "Loading original ..."
Load Jpg PATH$ + "/baboon.jpg"
Pause 1000

? "Saving copy ..."
Save Jpg "baboon-save.jpg"

? "Clearing screen ..."
Cls
Pause 1000

? "Loading copy..."
Load Jpg "baboon-save.jpg"

Do : Loop

