Option Base 0
Option Default None
Option Explicit

'Option Device "Colour Maximite 2"

Const WII_I2C = 3
Dim b$, l$, lx$, ly$, r$, rx$, ry$

Controller Classic Open WII_I2C

Cls

Do
  b$ = str.lpad$(Str$(Classic(B, WII_I2C)), 6)
  l$ = str.lpad$(Str$(Classic(L, WII_I2C)), 6)
  lx$ = str.lpad$(Str$(Classic(LX, WII_I2C)), 6)
  ly$ = str.lpad$(Str$(Classic(LY, WII_I2C)), 6)
  r$ = str.lpad$(Str$(Classic(R, WII_I2C)), 6)
  rx$ = str.lpad$(Str$(Classic(RX, WII_I2C)), 6)
  ry$ = str.lpad$(Str$(Classic(RY, WII_I2C)), 6)

  Console Home
  Print "Buttons:                  "; b$
  Print "Left  Analog Button:      "; l$
  Print "Right Analog Button:      "; r$
  Print "Left  Analog Stick:   x = "; lx$; " y = "; ly$
  Print "Right Analog Stick:   x = "; rx$; " y = "; ry$

  Pause 100
Loop

Function str.lpad$(s$, x%)
  str.lpad$ = s$
  If Len(s$) < x% Then str.lpad$ = Space$(x% - Len(s$)) + s$
End Function
