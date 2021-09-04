' speedtest.bas lines per second
' will test and report the speed of MMBasic
'
SetPin 34,2 : SetPin 32, 8
lines = 5
loops = 1000000
Timer = 0
'
' start of the timed section (5 lines)
For i = 1 To loops
 inp = Pin(34)
 Pin(32) = inp
 cnt = cnt + 1
Next
'
Print "Speed is"; Int((loops * lines)/(Timer /1000)); " lines per second."

Flash run 1

End
