#!../build/mmbasic -i

ChDir Mm.Info(Path)
Execute "RUN " + Chr$(34) + "../basic-src/sptest/sptest.bas" + Chr$(34) + ", " + Mm.CmdLine$
End
