' Copyright (c) 2021-2022 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 5.07

#!../build/mmbasic -i

ChDir Mm.Info(Path)
Execute "RUN " + Chr$(34) + "../basic-src/sptest/sptest.bas" + Chr$(34) + ", " + Mm.CmdLine$
End
