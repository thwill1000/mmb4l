' Copyright (c) 2021 Thomas Hugo Williams
' For MMB4L 2021.01

' LED scrolling text demo.

Option Base 0
Option Default None
Option Explicit On

#Include "matrix.inc"
#Include "text.inc"

Cls

Dim s$ = MM.CmdLine$
If Len(s$) = 0 Then Line Input "Message: ", s$

Console HideCursor
Console Foreground Bright Red

m7219_init(8,8)
text_init()
SetTick 50, m7219_draw
text_scroll(s$)
End
