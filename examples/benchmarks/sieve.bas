Print "Sieve of Eratosthenes Speed Test"
Print
Start = Timer

10 W=500:Dim F(W):P=1:A=3
20 F(P)=A:P=P+1:If P>W Then GoTo DONE
30 A=A+2:X=1
40 S=A/F(X):If S=Int(S) Then 30
50 X=X+1:If X<P And F(X)*F(X)<=A Then 40
60 GoTo 20

DONE:
Elapsed = (Timer-Start)/1000
Print Elapsed " seconds"
Print

I = 0 ' statement counter
J = 0 ' line counter
Erase F

110 I=I+4: J=J+1: W=500:Dim F(W):P=1:A=3
120 I=I+3: J=J+1: F(P)=A:P=P+1:If P>W Then GoTo FINI
130 I=I+2: J=J+1: A=A+2:X=1
Print ,P Chr$(13);
140 I=I+2: J=J+1: S=A/F(X):If S=Int(S) Then I=I+1: GoTo 130
150 I=I+2: J=J+1: X=X+1:If X<P And F(X)*F(X)<=A Then I=I+1: GoTo 140
160 I=I+1: J=J+1: GoTo 120

FINI:
Print I " statements"
Print J " lines"
Print
Print I / Elapsed " statements per second"
Print J / Elapsed " lines per second"
Print

flash run 1
