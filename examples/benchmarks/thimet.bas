' Thimet Calculator Performance Index
' https://thimet.de/CalcCollection/CalcPerformance.html

timer=0
loops=60000 ' Adjust # of loops so runtime is at least 5 seconds.
for i=0 to loops-1
 r0=10
 do
   x=r0
   x=x+1
   x=x-4.567e-4
   x=x+70
   x=x-69
   x=x*7
   x=x/11
   r0=r0-1
 loop while r0>0
 x=log(x)
 x=sin(x)
 x=sqr(x)
 x=sqr(x)
next
print x
t=timer/1000
print format$(t,"Time:% 3.3f seconds")
print format$(34/t*loops,"Index:% 4.2f")
