'benchmark.bas"
Print "Maximite Benchmark tests"
Print " "
Dim float tm(8)
Print "Benchmark 1"
Timer = 0
For j = 1 To 1000
Next j
tm(1)=Timer / 1000
Print tm(1)
Print " "

Print "Benchmark 2"
Timer = 0
j = 0
BM2:
j = j+1
If j < 1000 GoTo BM2
tm(2)=Timer / 1000
Print tm(2)
Print " "

Print "Benchmark 3"
Timer = 0
j = 0
BM3:
j = j+1
a = j/j*j+j-j
If j < 1000 GoTo BM3
tm(3)=Timer / 1000
Print tm(3)
Print " "

Print "Benchmark 4"
Timer = 0
j = 0
BM4:
j = j+1
a = j/2*3+4-5
If j < 1000 GoTo BM4
tm(4)=Timer / 1000
Print tm(4)
Print " "

Print "Benchmark 5"
Timer = 0
j = 0
BM5:
j = j+1
m = j/2*3+4-5
GoSub 4000
If j < 1000 GoTo BM5
tm(5)=Timer / 1000
Print tm(5)
Print " "

Print "Benchmark 6"
Timer = 0
j = 0
Dim ray(5)
BM6:
j = j+1
m = j/2*3+4-5
GoSub 4000
For q = 1 To 5
Next q
If j < 1000 GoTo BM6
tm(6)=Timer / 1000
Print tm(6)
Print " "

Print "Benchmark 7"
Timer = 0
j = 0
Dim ray2(5)
BM7:
j = j+1
m = j/2*3+4-5
GoSub 4000
For q = 1 To 5
ray2(q) = m
Next q
If j < 1000 GoTo BM7
tm(7)=Timer / 1000
Print tm(7)
Print " "

Print "Benchmark 8"
Timer = 0
j = 0
BM8:
j = j+1
m = j^2
blog = Log(j)
csin = Sin(j)
If j < 1000 GoTo BM8
tm(8)=Timer / 1000
Print tm(8)

For i=1 To 8: Print Str$(tm(i),0,4);"  ";: Next i

Flash run 1

End

4000 Return
