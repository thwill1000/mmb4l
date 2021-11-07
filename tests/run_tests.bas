#!../build/mmbasic -i

ChDir Mm.Info(Path)
Execute "RUN " + Chr$(34) + "sptest/sptest.bas" + Chr$(34) + ", " + Mm.CmdLine$
End
