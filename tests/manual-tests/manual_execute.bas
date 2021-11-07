Option Explicit On
Option Default None
Option Base 0

Print "Hello World: " Mm.CmdLine$
Dim s$ = Mm.Info(Current)

Execute("RUN " + Chr$(34) + s$ + Chr$(34) + ", --foo")