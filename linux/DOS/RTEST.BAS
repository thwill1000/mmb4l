' MMBasic regression test

Print "MMBasic regression test"
Print mm.device$ "  Ver" mm.ver
if mm.device$ <> "Micromite MkII" and mm.device$ <> "Micromite Plus" and mm.device$ <> "DOS" then
  error "Incompatible device: " + mm.device$
endif
Print
Print "Testing: Comments, PRINT and INPUT statements"
' test of a comment
Rem a test : of a " remark
aaaaa = 3        ' another : test of print a comment
     ' final test of a comment.  the next tests an empty line

Print "Print test...", "The number"; " at the end of this line should be three:";
i3 = 3 :Print i3
Input "Enter 1,2,3 or press return to skip the test: ", i1, i2%, i3
If i1 = 0 And i2% = 0 And i3 = 0 Then GoTo SkipThis
If i1 <> 1 Or i2% <> 2 Or i3 <> 3 Then Error
SkipThis: Input "There should be a ? at the end of this prompt.  Press return"; a$
If a$ <> "" Then Error
Line Input "Test of LINE INPUT.  Enter ttt or just press return to skip: ", a$
If Not (LCase$   (a$) = "ttt" Or a$ = "") Then Error      ' note extra spaces
Print



Print "Testing: Variables, assignment and the expression engine"

b = 6  : c = 8
d$ = "Hello"  :  e$ = "World"
_this_is_a_32_CHARACTER_test.var = 1234             ' comment
If _this_is_a_32_character_test.var <> 1234 Then Error
If 4 + 5 - 2 * 6 - 20 / 5 <> -7 Then Error
If d$ + " " + e$ <> "Hello World" Then Error
ltest1 = 4 = 40/10 And (34 < 35 Or 100 < 50)        ' comment
If 4 = 40/10 And (34 < 35 Or 100 < 50) Then ltest2 = 1 Else Error
If ltest1 <> ltest2 Then Error
If Not(ltest1 = ltest2) Then Error
If &B1000 <> 8 Or &O10 <> 8 Or &H0A <> 10 Then Error
If -Int(5.6) * -(2 + 3) + -1 <> 24 Then Error
If 1.6E+4 <> 16000 Then Error
If 1.6E-4 <> 0.00016 Then Error
If .001 <> 00.001 Then Error
if 12-3 <> 9 Then Error
if 12+3 <> 15 Then Error
if +12+3 <> 15 Then Error
if -12+3 <> -9 Then Error
if 12++3 <> +15 Then Error
if 12-+3 <> +9 Then Error
if 12+-3 <> 9 Then Error

' note extra spaces
dim fl  (4) as float, in( 5 ) as integer, st (3) length 24 as string
fl (2)=222
in    (4)=444
st(1) = "one"
st(2) = "twotwotwotwotwotwotwotwo"

If mm.device$ <> "DOS" Then
  option autorun on
  var restore

  clear
  f = 1.2 : s$ = "Hello" : i% = 777
  if f <> 1.2 or i% <> 777 or s$ <> "Hello" then error
  var save f, i%, s$
  clear
  if f <> 0 or i% <> 0 or s$ <> "" then error
  var restore
  if f <> 1.2 or i% <> 777 or s$ <> "Hello" then error
  f = 1.22 : i% = 777 : s$ = "Hello"
  var save f
  clear
  option autorun off
  var restore
  if f <> 1.22 or i% <> 777 or s$ <> "Hello" then error
  s$ = "Hello World"
  option autorun on
  var save s$
  clear
  var restore
  if f <> 1.22 or i% <> 777 or s$ <> "Hello World" then error
  xx = 1234
  var save xx
  clear
  ss$ = "Test"
  var restore
  option autorun off
  if f <> 1.22 or i% <> 777 or s$ <> "Hello World" or xx <> 1234 or ss$ <> "Test" then error
  clear
  var clear

  ' note extra spaces
  dim fl  (4) as float, in( 5 ) as integer, st (3) length 24 as string
  fl (2)=222
  in    (4)=444
  st(1) = "one"
  st(2) = "twotwotwotwotwotwotwotwo"

        aa = 1234 : bb$ = "This test" : c = 56789 : x% = 123456789012345
        VAR save aa, fl(), bb$, in(), x%
        bb$ = "This is a test"
        in(4) = 4
        VAR save aa, bb$, c, in(), st()
        Clear
        dim fl(4) as float, in(5) as integer, st(3) length 24 as string
        in  (4) = 0
        st(1) = "xxxx"
        st( 2) = "yyyy"
        If aa <> 0 Or bb$ <> "" Or c <> 0 or x% <> 0 Then Error
        VAR restore
        If aa <> 1234 Or bb$ <> "This is a test" Or  c <> 56789 or x% <> 123456789012345  Then Error
        if in(4) <> 4 or st(1) <> "one" or st(2) <> "twotwotwotwotwotwotwotwo" Then Error
        clear
        var clear
        dim fl(4) as float, in(5) as integer, st(3) length 24 as string
        var restore
        If aa <> 0 Or bb$ <> "" Or  c <> 0 or x% <> 0 Then Error
        clear
        aa = 1234 : bb$ = "This is a test" : c = 56789 : x% = 123456789012345
        VAR save aa, bb$, x%, c
endif
Clear


Print "Testing: GOTO, IF and CASE statements"

b = 6  : c = 8

GoTo TestSkip
Error
TestSkip:             ' destination of a goto can be a comment

GoTo TestSkip2
Error
100 TestSkip2: ' a label can be preceeded with a line number

GoTo PrintT           ' a label cannot be the same as a command name
Error
PrintT:

GoTo 200              ' a line number must have something after it, eg '
Error
200 '

If b < c Then 520                                   ' comment
510 T1: Error                                               ' comment
520 If b > c Then 510 Else 530                          ' comment
530 If b > c Then Error                                 ' comment
If b < c Then f$ = "If test" + " ok" Else f$ = "If test" + " failed"
If f$ <> "If test ok" Then Error                    ' comment
If B < c Then f$ = "foo" : If f$ <> "foo" Then Error' comment
If b > C Then Error Else f$ = "boo" : If f$<>"boo"Then Error'comment
If b = c GoTo t1                                   ' comment
If b < c GoTo T3                                   ' comment
Error
   t3: a = 0
For i = 1 To 6
  If i Mod 2 Then                     ' structured IF stmt
    If i = 4 Or i = 6 Then Error      ' comment
  ElseIf i > 3 Then                   ' structured ELSEIF stmt
   If i = 4 Or i = 6 Then a = a + 1 Else Error
  EndIf                               ' comment
Next i                                ' comment
If a <> 2 Then                        ' comment
  Error                               ' comment
Else                                  ' comment
  i = i + 1                           ' comment
EndIf                                 ' comment
If a = 2 Then : a = a + 2 : Else : Error : EndIf
If a <> 4 Then Error
b = 5 : c = 6
If a = 4 Then If b <> c Then b = b + 1 Else Error
If a <> b Then If b <> c Then Error Else b = b + 1

Clear
TestCase "1" : If b$ <> "one" Then Error
TestCase "2" : If b$ <> "two" Then Error
TestCase "3" : If b$ <> "3 to 9" Then Error
TestCase "5" : If b$ <> "3 to 9" Then Error
TestCase "9" : If b$ <> "3 to 9" Then Error
TestCase "m" : If b$ <> ">= m" Then Error
TestCase "x" : If b$ <> ">= m" Then Error
TestCase "c" : If b$ <> "????" Then Error

TestCase "a", 1 : If b$ <> "one" Then Error
TestCase "a", 2 : If b$ <> "two" Then Error
TestCase "a", 3 : If b$ <> "3 to 9" Then Error
TestCase "a", 5 : If b$ <> "3 to 9" Then Error
TestCase "a", 9 : If b$ <> "3 to 9" Then Error
TestCase "a", 10 : If b$ <> ">= 10" Then Error
TestCase "a", 99 : If b$ <> ">= 10" Then Error
TestCase "a", -8 : If b$ <> "aggh" Then Error

TestCase "a", 1.0 : If b$ <> "one" Then Error
TestCase "a", 2.0 : If b$ <> "two" Then Error
TestCase "a", 3.0 : If b$ <> "3 to 9" Then Error
TestCase "a", 5.4 : If b$ <> "3 to 9" Then Error
TestCase "a", 9.1 : If b$ <> "3 to 9" Then Error
TestCase "a", 10 : If b$ <> ">= 10" Then Error
TestCase "a", 99.8 : If b$ <> ">= 10" Then Error
TestCase "a", -8.3 : If b$ <> "aggh" Then Error

Sub TestCase a$, i   ' comment
    Select Case a$   ' comment
        Case "1"   ' comment
            b$ = "one"   ' comment
        Case "2"   ' comment
            b$ = "two"
        Case "3" To "9"   ' comment
            b$ = "3 to 9"   ' comment
        Case "a"
            Select Case i
                Case 1
                    b$ = "one"
                Case 2.0
                    b$ = "two"
                Case 3 To 9.1
                    b$ = "3 to 9"
                Case is >= 10
                    b$ = ">= 10"
                Case Else
                    b$ = "aggh"
            End Select
        Case is >= "m"
            b$ = ">= m"
        Case Else   ' comment
            b$ = "????"   ' comment
    End Select   ' comment
End Sub



Print "Testing: Operators"

If 30 * 3 / 9 + 1 - 8 Mod 3 <> 9 Then Error
If 3 <> 3 Then Error
If 3 >= 4 Then Error
If 4 <= 3 Then Error
If 3 > 3 Then Error
If 3 < 3 Then Error
If 4 = 3 Then Error
If (7 And 2) <> 2 Then Error
If (4 Or 2) <> 6 Then Error
If (4 Xor 2) <> 6 Then Error



Print "Testing: FOR, WHILE and DO loops"

Clear
a = 1
For i = 23 To 1
  a = 2
Next i
If a = 2 Then Error
tmp = 0                   ' comment
For i = 1 To 5            ' comment
   For y = 2 To 6 Step 2  ' comment
      tmp = tmp + 1       ' comment
   Next y                 ' comment
Next                      ' comment
If i <> 6 Or y <> 8 Or tmp <> 15 Then Error
a = 0
For i = 10 To 1 Step -2
  a = a + 1
Next i
If a <> 5 Then Error
tmp = 0
1130 ' target of goto
For i = 1 To 5
    tmp = tmp + 1
    If i > 2 And tmp < 100 Then GoTo 1130
    If i > 2 And tmp > 100 Then GoTo T1200
Next
Error
T1200: For i = 1 To 5
   For j = 6 To 0 Step -1
Next :Next
If i<>6 Or j<>-1 Then Error
tmp = 0
For i = 1 To 5 : If tmp > 100 Then GoTo LongLabel01234567890123456789012
   For j = 6 To 0 Step -1
       For k = 2 To 6 :tmp=tmp+1 :Next
   GoTo x
   Next
x: Next
LongLabel01234567890123456789012: If tmp<>25 Then Error

' test FOR loops with a floating point step value
a = 0
FOR i = 1 to 10.05 STEP 0.1
  a = a + 1
NEXT i
IF a <> 91 THEN Error

a = 0
FOR i = 5 to -5 STEP -0.2
  a = a + 1
NEXT i
IF a <> 51 and a <> 50 THEN Error

' test FOR loops with integer variable
a = 0
FOR ii% = 1 to 10.3 STEP 0.9
  a = a + 1
NEXT ii%
IF a <> 10 THEN Error

a = 0
FOR ii% = 5 to -5 STEP -1
  a = a + 1
NEXT ii%
IF a <> 11 THEN Error

' test while loops
a = 0
Do While a < 10
 a = a + 1
Loop
a = 0
Do
 a = a + 1
If a > 10 Then Error
Loop Until a >= 10
a = 0  : b = 0
1430 Do
 a = a + 1  : b = b + 1
 If b < 30 Then GoTo 1430
Loop While a < 10
If a <> 30 Then Error
b = 0 : c = 0
Do While b < 60                     ' comment
   b = b + 1
   Do                               ' comment
       Do
           c = c + 1
           If c > 5 Then GoTo LLL
       Loop                         ' comment
   Loop While b < 30                ' comment
LLL: Loop
If b<>60 Or c<>65 Then Error
a = 0
While a < 30
   a = a + 1
Wend
If a <> 30 Then Error
a = 0
Do While a < 30
   a = a + 1
   For i = 1 To 20
       If i > 10 Then Exit For
   Next i
   If a > 20 Then Exit
Loop
If a<>21 Or i<>11 Then Error
tmp = 0
For i = 1 To 5
    For j = 1 To 5
           tmp = tmp + 1
Next i, j
If tmp <> 25 Then Error

for i = 1 to 20
  if i = 10 then continue for
  if i = 10 then error
next i
if i <> 21 then error

i = 0
do
  i = i + 1
  if i = 10 then continue Do
  if i = 10 then error
loop while i < 20
if i <> 20 then error

' test single line loops
 tn = -36
 range = 24
 Do While tn >= range : tn = tn - range
 Loop
 if tn <> -36 then error
 tn = -36
 Do While tn >= range
 tn = tn - range : Loop
 if tn <> -36 then error
 tn = -36
 Do While tn >= range
 tn = tn - range
 Loop
 if tn <> -36 then error
 tn = -36
 Do While tn >= range : tn = tn - range : Loop
 if tn <> -36 then error
 tn = 36
 Do While tn >= range : tn = tn - range
 Loop
 if tn <> 12 then error
 tn = 36
 Do While tn >= range
 tn = tn - range : Loop
 if tn <> 12 then error
 tn = 36
 Do While tn >= range
 tn = tn - range
 Loop
 if tn <> 12 then error
 tn = 36
 Do While tn >= range : tn = tn - range : Loop
 if tn <> 12 then error

' these tests try to catch errors where exiting a DO or FOR can skip one line
'
i = 0: do while i < 4: i = i + 1:  loop: goto next1 : error
next1:
if i <> 4 then error
i = 0: do : i = i + 1 : if i = 4 then exit do
loop: goto next2 : error
next2:
if i <> 4 then error
i = 0: do : i = i + 1:  loop while i < 4: goto next3 : error
next3:
if i <> 4 then error

i = 0: do while i < 4
  i = i + 1
loop
goto next4
error
next4:
if i <> 4 then error

i = 0
  do
  i = i + 1
  if i = 4 then exit do
loop
goto next5
error
next5:
if i <> 4 then error

i = 0
  do
  i = i + 1
  if i = 4 then continue do
  if i = 8 then exit do
  if i = 8 or i = 4 then error
loop
goto next5a
error
next5a:
if i <> 8 then error

i = 0
do
  i = i + 1
loop while i < 4
goto next6
error
next6:
if i <> 4 then error

for i = 1 to 4:next i:goto next7
error
next7:
if i <> 5 then error

for i = 1 to 10
  if i = 4 then exit for
next i
goto next8
error
next8:
if i <> 4 then error

for i = 1 to 10
  if i = 4 then continue for
  if i = 8 then exit for
  if i = 8 or i = 4 then error
next i
goto next8a
error
next8a:
if i <> 8 then error

for i = 1 to 4
next i
goto next9
error
next9:
if i <> 5 then error


'
Print "Testing: Functions"
'
If MM.Ver < 2 Then Error
If Left$(MM.Device$, 9) = "Micromite" Then
  If MM.Watchdog Then Error
  If MM.OneWire Then Error
endif

If Abs(7*(-5)) <> 35 Then Error
If Asc("Ten") <> 84 Then Error
r = Atn(3) - 1.24905  :If Abs(r) > 0.001 Then Print Atn(3) :Error
If Chr$(66) <> "B" Then Error
If Cint(45.67) <> 46 Then Error
If 2*Cos(.4) < 1.842 or 2*Cos(.4) > 1.8425 Then Print 2*Cos(.4) :Error
If Exp(   4) < 54.598 or Exp(4) > 54.599 Then Print Exp(4) : Error  ' note extra spaces
If Hex$(32) <> "20" Then Error
If Hex$(12, 2) <> "0C" Then Error
If Oct$(8) <> "10" Then Error
If Oct$(8, 4) <> "0010" Then Error
If Bin$(12345) <> "11000000111001" Then Error
If Bin$  (12345, 64) <> "0000000000000000000000000000000000000000000000000011000000111001" Then Error
If Instr    (3, "ABCDEBXYZ", "B") <> 6 Then Error' note extra tabs (spaces)
If Int(-12.11) <> -13 Then Error
If Fix(-12.11) <> -12 Then Error
If Left$("testing", 4) <> "test" Then Error
If Len("hello") <> 5 Then Error
If Mid$("Good", 2, 2) <> "oo" Then Error
If Mid$("Hello World", 7) <> "World" Then Error
If Right$("Hello", 3) <> "llo" Then Error
If Rnd < 0 Or Rnd(1) > 1 Then Error
If Sgn(34) <> 1 Or Sgn(0) <> 0 Or Sgn(-34) <> -1 Then Error
If Abs(Sin(1.5) - 0.99749) > 0.001 Then Error
If Sqr(25) <> 5 Then Error
If Abs(Tan(+0.5) - 0.546302) > 0.001 Then Error
If Val("&H3A") <> 58 Then Error
If Val("&O11") <> 9 Then Error
If Val("&B110111") <> 55 Then Error
If Val("10.22") <> 10.22 Then Error
If Val("10.22E1") <> 102.2 Then Error
If Val("10.22hEllo.there") <> 10.22 Then Error
If Val("0hEllo.there") <> 0 Then Error
If Val("1234567890") <> 1234567890 Then Error
If Val("1234567890hEllo.there") <> 1234567890 Then Error
aaaa$ = "3a" : If Val("&H" + aaaa$) <> 58 Then Error
If Space$(4) <> "    " Then Error
Randomize 4
tmp = Rnd  : If Rnd = tmp Then Error
Randomize 4
If Rnd <> tmp Then Error

If Str$(100) <> "100" Then Error
If Str$(123.456) <>          "123.456" Then Error
If Str$(123.456, 6) <>       "   123.456" Then Error
If Str$(-123.456, 6) <>      "  -123.456" Then Error
If Str$(-123.456,  6,  5) <> "  -123.45600" Then Error
If Str$(-123.456, 6, 5, "0") <> "-00123.45600" Then Error
If Str$(53, 6) <>            "    53" Then Error
If Str$(53, 6, 2) <>         "    53.00" Then Error
If Str$(53, 6, 2, "*") <>    "****53.00" Then Error
If Str$(53, -6, 2, "*") <>   "***+53.00" Then Error
If Str$(0.44, 1) <>          "0.44" Then Error
If Str$(-0.44, 0) <>         "-0.44" Then Error
If Str$(0.99) <>             "0.99" Then Error
If Str$(-0.99) <>            "-0.99" Then Error
If Str$(0.01, 1) <>          "0.01" Then Error
If Str$(-0.01, 0) <>         "-0.01" Then Error
If str$(32, 1, -6) <>        "3.200000e+01" Then Error
If str$(Pi, 1, -6) <>        "3.141593e+00" Then Error

Clear
dim integer z
dim float y
z = -5
if str$(z, -4, 0, "0") <> "-005" then error
if str$(z, -3, 0) <>      " -5" then error
if str$(z, -2, 0, "0") <> "-5" then error
if str$(z, -1, 0, "0") <> "-5" then error
if str$(z,  0, 0, "0") <> "-5" then error

if str$(z,  4, 0, "*") <> "**-5" then error
if str$(z,  4, 6) <>      "  -5.000000" then error

z = 5
if str$(z, -4, 0, "0") <> "+005" then error
if str$(z, -3, 0, "*") <> "*+5" then error
if str$(z, -2, 0, "0") <> "+5" then error
if str$(z, -1, 0) <>      "+5" then error
if str$(z,  0) <>         "5" then error
if str$(z) <>             "5" then error

if str$(z,  4, 0, "0") <> "0005" then error
if str$(z,  4, 1, "0") <> "0005.0" then error
if str$(z,  4, 0) <>      "   5" then error
if str$(z,  4) <>         "   5" then error
if str$(z,  4, 1) <>      "   5.0" then error

z = 0
if str$(z,  4, 0) <>      "   0" then error
if str$(z,  4) <>         "   0" then error
if str$(z,  4, 1) <>      "   0.0" then error

y = -5.1234
if str$(y, -4, 0, " ") <> "  -5" then error
if str$(y, -3, 0) <>      " -5" then error
if str$(y, -2, 0, "0") <> "-5" then error
if str$(y, -1, 0) <>      "-5" then error
if str$(y,  0, 0, "0") <> "-5" then error

if str$(y,  4, 0, "0") <> "-005" then error
if str$(y,  4, 5, "*") <> "**-5.12340" then error
if str$(y,  4, 4, "*") <> "**-5.1234" then error
if str$(y,  4, 3, "*") <> "**-5.123" then error
if str$(y,  4, 2, "*") <> "**-5.12" then error
if str$(y,  4, 1, "*") <> "**-5.1" then error

y = 5.1234
if str$(y, -4, 0, "0") <> "+005" then error
if str$(y, -3, 0) <>      " +5" then error
if str$(y, -2, 0, "0") <> "+5" then error
if str$(y, -1, 0) <>      "+5" then error
if str$(y,  0, 0, "0") <> "5" then error

if str$(y,  4, 0, "0") <> "0005" then error
if str$(y,  4, 1, "0") <> "0005.1" then error
if str$(y,  4, 0) <> "   5" then error
if str$(y,  4, 1) <> "   5.1" then error
if str$(y,  -4, 0) <> "  +5" then error
if str$(y,  -4, 1) <> "  +5.1" then error

y = 0
if str$(y,  4, 0) <>      "   0" then error
if str$(y,  -4, 0) <>     "  +0" then error
if str$(y,  4) <>         "   0" then error
if str$(y,  4, 1) <>      "   0.0" then error

Clear

If String$(5, "ABC") <> "AAAAA" Or String$(5, 55) <> "77777" Then Error
a = 35
b% = 35
s$ = "#"
r$ = String$(5, "#")+String$(5, 35)+String$(5, 35.34)+String$(5, a)+String$(5, b%)+String$(5, s$)
If r$ <> "##############################" Then Error
If UCase$("fooBOO") <> "FOOBOO" Then Error
If LCase$("fooBOO") <> "fooboo" Then Error
If Abs(Pi - 3.14159) > 0.0001 Then Error
If Int(Deg(Rad(30))) <> 30 and Int(Deg(Rad(30))) <> 29 Then Error

If mm.device$ <> "Micromite MkII" Then
  tststr$ = "1*(1+2+3+4+5+6+7+8+9+10+11+12+13)"
  If Eval("1*(1+2+3+4+5+6+7+8+9+10+11+12+13)") <> 91 Then Error
  If Eval(tststr$) <> 91 Then Error
  If Eval("LCase$(" + chr$(34) + "fooBOO" + chr$(34) + ") + UCase$(" + chr$(34) + "fooBOO" + chr$(34) + ")") <> "foobooFOOBOO" Then Error
End If

''
Print "Testing: 64 bit integers"
''
i64% = &H7fffffffffffffff
If i64% <> 9223372036854775807 Then Error
i64% = i64% \ 100
If i64% <> 92233720368547758 Then Error
If Hex$(i64%) <> "147AE147AE147AE" Then Error
If Oct$(i64%) <> "5075341217270243656" Then Error
If Bin$(i64%) <> "101000111101011100001010001111010111000010100011110101110" Then Error
If i64% + 100 <> 92233720368547858 Then Error
If i64% - 100 <> 92233720368547658 Then Error
If i64% + 1000000000 <> 92233721368547758 Then Error
If i64% - 1000000000 <> 92233719368547758 Then Error
x64% = i64%
x64% = x64% * 6
If x64% <> 553402322211286548 Then Error
If x64% Mod 1000 <> 548 Then Error
If x64% <> 553402322211286548 Then Error
If x64% = 553402322211286549 Then Error
If x64% < 553402322211286547 Then Error
If x64% <= 553402322211286547 Then Error
If x64% > 553402322211286549 Then Error
If x64% >= 553402322211286549 Then Error
If (x64% And 1000000) <> &B1110000001000000000 Then Error
If (x64% Or 1000000) <> 553402322211827284 Then Error
If (x64% Xor 1000000) <> &O36560507534122166124 Then Error
If 2^63 <> &H8000000000000000 Then Error
For ii64% = 10000000000 To 10000000010 Step 2
  xx64% = xx64% + 1
Next ii64%
If xx64% <> 6 Then Error
If Val("1923456789045") <> 1923456789045 Then Error
If Str$(1234567890, 15, 5, "*") <> "*****1234567890.00000" Then Error
If Str$(-1234567890, 15, 5, "*") <> "****-1234567890.00000" Then Error
If Hex$(&HFFFFFFFFFFFFFFFF) <> "FFFFFFFFFFFFFFFF" Then Error
If Oct$(&O1777777777777777777777) <> "1777777777777777777777" Then Error
If Bin$(&B1111111111111111111111111111111111111111111111111111111111111111) <> "1111111111111111111111111111111111111111111111111111111111111111" Then Error
If Hex$(&H800000000000000F And &H8000000000000000) <> "8000000000000000" Then Error
If Hex$(&H000000000000000F Or &H800000000000000F) <> "800000000000000F" Then Error
If Hex$(&HF00000000000000F Xor &HFFFFFFFFFFFFFFF0, 16) <> "0FFFFFFFFFFFFFFF" Then Error
If val("&b1111111111111111111111111111111111111111111111111111111111111110") <> -2 Then Error
If val("&hffffffffffffffff") <> -1 Then Error


''
Print "Testing: GOSUB, DATA, DIM and misc statements"
''
tmp = 0
GoSub Sub1
GoTo Sub2
2210 Sub1: If tmp > 28 Then GoTo Ret2
tmp = tmp + 1
GoSub 2210
Ret2: Return
Error
Sub2: ' target of a goto

' test option base
Option base 1
Option base 0

' test locals
tmp = 77 : ttmp = 44
GoSub ltest
If tmp <> 77 Or foo$ <> "" Or ttmp <> 55 Then Error
GoTo SkipLtest
ltest:
  Local tmp, foo$
  tmp = 4 : foo$ = "goo"
  If tmp <> 4 Or foo$ <> "goo" Or ttmp <> 44 Then Error
  ttmp = 55
  Return
SkipLtest:

Data 12, "foo"
restore
Read axx, bxx$, cxx, dxx$
datatest: Data -(28 * 2),ttest, 89, &H0100003E5B4F5B3E, 1234567890123
If axx<>12 Or bxx$<>"foo" Or cxx<>-56 Or dxx$ <> "ttest" Then Error
Restore
Read xxx, yxx$, zxx
If xxx<> 12 Or yxx$<>"foo" Or zxx<> -56 Then Error
Read dxx$, xxx, xjx%, xyx%
If xxx <> 89 Or xjx% <> &H0100003E5B4F5B3E Or xyx% <> 1234567890123 Then Error
restore datatest
Read zxx
If zxx<> -56 Then Error
t = 3
2350 OnGotoLoop: On t GoTo 2360, 2380, OnGoto3, 2360
2360 Error

'no space between the line number and the label
2370OnGoto3: t = 2  : GoTo OnGotoLoop
2380 Dim azz(10,10,10), bzz$(4) length 20, czz$(5)
d = 0
For i = 0 To 10
   For j = 0 To 10
       For k = 0 To 10
           azz(i,j,k) = i*j*k
Next k, j, i
bzz$(1) = "1234567890"
czz$(1) = "1234567890123456789012345678901234567890"
If azz(0,0,0) <> 0 Or azz(5,5,5) <> 125 Or azz(10,10,10) <> 1000 Then Error
Erase azz, bzz$, czz$
Dim azz(10), bzz$(3)
If azz(5)<>0 Then Error

Clear
Dim integer o1, o3
Dim o4$ As string
Dim o2!, o5 As float
Dim o6% As integer
o1 = 1000000000000
o2 = 1000000000000
o3 = 1000000000000
o4 = "Hi
'"
o5 = 1000000000000
o6 = 1000000000000
If Str$(o1) <> "1000000000000" Then Error
If Str$(o2) <> "1e+12" Then Error
If Str$(o3) <> "1000000000000" Then Error
If o4 <> "Hi" Then Error
If Str$(o5) <> "1e+12" Then Error
If Str$(o6) <> "1000000000000" Then Error
Clear
Dim integer o1 = 1234567890
Dim As float o2! = 1.23
Dim As integer o3 = 12.4
Dim As string o4$ = "Hi"
If Str$(o1) <> "1234567890" Then Error
If Str$(o2) <> "1.23" Then Error
If Str$(o3) <> "12" Then Error
If o4 <> "Hi" Then Error
If o4$ <> "Hi" Then Error
Clear
Dim days(4) LENGTH 3 As STRING = ("Mon","Tue","Wed","Thu","Fri")
If days(2) <> "Wed" Then Error
Dim nbr(3,4) As float = (0,1,2,3,4,10,11,12,13,14,20,21,22,23,24,30,31,32,33,34)
If nbr(2,1) <> 11 Then Error
Dim As integer intg(5) = (0,1,2,3,4,5)
If intg(4) <> 4 Then Error
Dim o1 As integer = 1234567890, o2! As float = 1.23, o3 As integer = 12.4, o4$ As string = "Hi"
If Str$(o1) <> "1234567890" Then Error
If Str$(o2) <> "1.23" Then Error
If Str$(o3) <> "12" Then Error
If o4$ <> "Hi" Then Error
Clear
Dim ff$(5)
Dim integer fff(5), ffff(8), fffff(8)
Erase ff$, fff, ffff(), fffff%
Dim jj%, jjj$, jjjj!
Erase jj, jjj, jjjj
Dim as Integer a, b
a = 7: b = 5
Erase a,b
If a + b <> 0 then error
Option default integer
o1 = 1000000000000
o2! = 1000000000000
o3 = 1000000000000
o4$ = "Hi
if str$(o1) <> "1000000000000" then error
if str$(o2!) <> "1e+12" then error
if str$(o3) <> "1000000000000" then error
if o4$ <> "Hi" then error
clear
option default float
o1 = 1000000000000
o2% = 1000000000000
o3 = 1000000000000
o4$ = "Hi
'"
if str$(o1) <> "1e+12" then error
if str$(o2%) <> "1000000000000" then error
if str$(o3) <> "1e+12" then error
if o4$ <> "Hi" then error
clear
const o1 = 1000000000000
const o2 = 1000000000000.0
if str$(o1) <> "1000000000000" then error
if str$(o2) <> "1e+12" then error
const a = 23, b = 45.7, c = "fff", d = 23
if a <> 23 or b <> 45.7 or c <> "fff" or d <> 23 then error

Clear
if mm.device$ <> "DOS" then
        Dim x%(10);
        x%(0) = 3
        a% = &HFFFF111189AB1213
        If Peek(var a%, 0) <> &H13 Then Error
        If Peek(var a%, 1) <> &H12 Then Error
        If Peek(var x%(), 0) <> 3 Then Error
        Poke var a%, 0, &H34
        If a% <> &HFFFF111189AB1234 Then Error
        If Peek(var a%, 0) <> &H34 Then Error
        If Peek(var a%, 1) <> &H12 Then Error
        Poke var x%(), 0, 4
        If Peek(var x%(0), 0) <> 4 Then Error
        addr% = Peek(VARADDR a%)
        If Peek(byte addr%) <> &H34 Then Error
        If Peek(byte addr% + 1) <> &H12 Then Error
        If Peek(word addr% + 10 - 10) <> &H89AB1234 Then Error
        Poke byte addr%, &H78
        If a% <> &HFFFF111189AB1278 Then Error
        Poke word addr%, &H12344321
        If a% <> &HFFFF111112344321 Then Error
        Poke addr%\&H100000000, addr% And &Hffffffff, &HAA
        If Peek(addr%\&H100000000, addr% And &Hffffffff) <> &HAA Then Error
        If Peek(progmem, 0) <> 1 Then Error
        Clear
        a = 1234
        If Peek(vartbl, 0) <> 65 Then Error  ' this might change in future builds
        Poke vartbl, 0, 65
endif

Clear
Option default none
Const a=8
Const b=4/a
If b <> 0.5 Then Error
Clear
Option default integer
Const a=8
Const b=4/a
If b <> 0.5 Then Error
Dim c = 8
Dim d = 4.0
If d/c <> 0.5 Then Error
e = 8
f = 4.0
If f/e <> 0.5 Then Error
Clear
Option default string
Const a=8
Const b=4/a
If b <> 0.5 Then Error
Dim c = "a"
Dim d = "b"
If c+d <> "ab" Then Error
e = "a"
f = "b"
If e+f <> "ab" Then Error
Clear
Option default float

on error ignore
f = "hhh"
on error abort
if mm.errno = 0 or mm.errmsg$ <> "Expected a number" then error
on error skip
gkhjghkjg
if mm.errno = 0 or mm.errmsg$ <> "Unknown command" then error
if mm.errno = 0 or mm.errmsg$ <> "Unknown command" then error
on error clear
f = 123
if mm.errno <> 0 or mm.errmsg$ <> "" then error
If mm.device$ <> "DOS" Then
  on error skip
  pin(999) = 0
  if mm.errno = 0 or mm.errmsg$ <> "Pin 999 is invalid" then error
EndIf
on error skip
erase fffggghhh
if mm.errno = 0 or mm.errmsg$ <> "Cannot find FFFGGGHHH" then error
on error skip
&&&
if mm.errno = 0 or mm.errmsg$ <> "Invalid character: &" then error

''
Print "Testing: Defined subroutines, functions and locals"
''

clear

' test passing pointers
i% = 12345
tstsub1 i%
if i% <> 76.0 then error
tstsub2 i%
if i% <> 76.0 then error

sub tstsub1 x as integer
  x = 76
end sub

sub tstsub2 x as float
  if x <> 76.0 then error
  x = x / 3
end sub

clear

sub1 "aaa"+fun1$(44)+"bbb"

Function fun1$(t As integer)
  fun1$=Str$(t)
End       Function

Sub sub1(a$)
  If a$ <> "aaa44bbb" Then Error
End Sub

Sub TestSub1 a, b$, c
  If a <> c Then Error
  If b$ <> "Hello" Then Error
  b$ = UCase$(b$)
  TestSub2 xxxyyy
  If Not(xxxyyy = 5 Or xxxyyy = 55) Then Error
End         Sub               ' multiple spaces between END and SUB

TestSub1 5, "Hello", 5
zzs$ = "Hello"
TestSub1 (0, zzs$)
If zzs$ <> "HELLO" Then Error

Sub TestSub2 (c)
  Local i
  For i = 1 To 50
    c = c + 1
    If (c = 5) Then Exit      Sub       ' multiple spaces between EXIT and SUB
  Next i
End Sub

x$ = "Hello"
xxxsub x$
If x$ <> "Hello World" Then Error

Sub xxxsub y$:yyy y$:End Sub

Sub yyy z$
  z$ = z$ + " "
  zzz z$
End Sub

Sub zzz x$
  x$ = x$ + "World"
End               Sub ' note multiple spaces

If (12 - (FNtstx(6) / 3)) <> 0 Then Error
If "xx" + FNtst$(12, "yy") <> "xxYYyy12" Then Error

Function FNtst$   (a, b$)   ' note extra spaces in this function
  Local x$
  x$ = UCase$(b$) + b$
  FNtst$ = x$ + Str$(FNtstx   (a) / 12)
  Exit           Function ' note multiple spaces
  FNtst$ = ""
End            Function ' note multiple spaces

Function FNtstx(a)
  Local b
  b = a - 1 + 1
  FNtstx = b * a
End Function


' STATIC test
nx = 9
if nx <> 9 Then Error
StaticTest
if nx <> 9 Then Error
nx = nx + 4
StaticTest
if nx <> 13 Then Error

Sub StaticTest
  Static nx = 3
  Static nnx = 3
  if not (nx = 3 or nx = 7) Then Error
  nx = nx + 4
  if not (nx = 7 or nx = 11) Then Error
  if not (nnx = 3 or nnx = 8) Then Error
  nnx = nnx + 5
  if not (nnx = 8 or nnx = 13) Then Error
End Sub


If trim$("  00hello  00", " 0") <> "hello" Then Error

Function TrimL$  (s$, c$)
  TrimL$ = s$
  If c$ = "" Then c$ = " "
  Do While Instr(c$, Left$(TrimL$, 1))
    TrimL$ = Mid$  (TrimL$, 2)
  Loop
End Function

Function TrimR$(s$, c$)
  TrimR$ = s$
  If c$ = "" Then c$ = " "
  Do While Instr  (c$, Right$(TrimR$, 1))
    TrimR$ = Mid$(TrimR$, 1, Len (TrimR$) - 1)
  Loop
End Function

Function Trim$(s$, c$)
  Trim$ = TrimR$(TrimL$(s$, c$), c$)
End Function


If tstx(10, 3) <> 9 Then Error

Function tsty%(a%, b%)
 tsty% = a%*b%
End Function

Function tstz(a, b)
  tstz = a/b
End Function

Function tstx(a,b)
  tstx = tsty%(tstz(a, b), b)
End Function

Clear
Dim a(2,4)
Dim b%(2,4)
Dim s$(2,4)

a(1, 2) = 1234
a(2, 1) = 4321
If fun(a()) <> 5555 Then Error

b%(1, 2) = 1234
b%(2, 1) = 4321
If funi%(b%()) <> 5555 Then Error

s$(1, 2) = "Hello"
s$(2, 1) = "World"
If funs$(s$()) <> "Hello World" Then Error

subxx 2, 1, s$()
If funs$(s$()) <> "Hello Foo" Then Error


Function fun    ( z(  ) )        ' note extra spaces
  fun = z(1,2) + z(2, 1)
End Function

Function funi%(z%())
  funi% = z%(1,2) + z%(2, 1)
End Function

Function funs$(z$())
  funs$ = z$(1,2) + " " + z$(2, 1)
End Function

Sub subxx a, b, x$()
  x$(a, b) = "Foo"
End Sub

' test functions with the AS <type> qualifier
Clear
Function asint(x% As integer, y As float, z As string) As integer
  If x <> 1234567890 Or y <> 12.34 Or z <> "test" Then Error
  asint = 987654321
End Function

If asint(1234567890, 12.34, "test") <> 987654321 Then Error
a% = 1234567890
b! = 12.34
c$ = "test"

If asint(a%, b!, c$) <> 987654321 Then Error

n=0
m= testfunc!(n)
If m <> 1.02 And n <> 0.03 Then Error
Function testfunc!(returnthisway As float) As float
  testfunc=1.02
  returnthisway=0.03
End Function


a% = 1234
atst1 a%, b!, c$
if not (a% = 10 and b! = 22.34 and c$ = "this test") then error

sub atst1 a%, b!, c$
  if not (a% = 1234 and b! = 12.34 and c$ = "test") then error
  atst2 a%, b!, c$
  if not (a% = 5 and b! = 22.34 and c$ = "this test") then error
  a% = a% + a%
end sub

sub atst2  x%, y!, z$
  if not (x% = 1234 and y! = 12.34 and z$ = "test") then error
  atst3  x%, y!, z$
  if not (x% = 1234 and y! = 12.34 and z$ = "test") then error
  x% = 5 : y! = y! + 10 : z$ = "this " + z$
end sub

sub atst3 l!, m%, n$
  if not (l! = 1234.0 and m% = 12) then error
  l! = 1: m% = 2
end sub

' test type conversion with function arguments
clear
DIM FLOAT pf(24), tf
dim integer  xi(24), ti

pf(24)=1012.9
xi(24)=1013
tf=1012.9
ti=1013

If ppf(pf(24)) <> "1012.9" Then Error
If ppi(pf(24)) <> "1013" Then Error
If ppf(xi(24)) <> "1013" Then Error
If ppi(xi(24)) <> "1013" Then Error
If ppf(tf) <> "1012.9" Then Error
If ppi(tf) <> "1013" Then Error
If ppf(ti) <> "1013" Then Error
If ppi(ti) <> "1013" Then Error

function ppi(p AS INTEGER) as string
  ppi = str$(p)
END function

function ppf(p AS float) as string
  ppf = str$(p)
END function

' test for string corruption when using a string array as an argument to a SUB
Dim String S$(4)
S$(0) = "0"
S$(1) = "25"
S$(2) = "50"
S$(3) = "75"
S$(4) = "100"
ArrayTest()

Sub ArrayTest()
  Local Integer I
  For I = 0 To 4
    PassArray(S$())
    CheckArrayValues()
  Next I
End Sub

Sub CheckArrayValues()
  If S$(0) <> "0" or S$(1) <> "25" or S$(2) <> "50" or S$(3) <> "75" or S$(4) <> "100" Then error
End Sub

Sub PassArray(TestArray() As String)
  'do nothing
End Sub

clear

if mm.device$ = "DOS" then goto DosSkipHardwareTests

''
Print "Testing: Micromite specific statements and functions"
''
'tron
Clear
' do not run this test if we are connected via USB console
if not (mm.device$ = "Micromite Plus" And (peek(word &HBF885060) and 1) = 1) then
  CPU 10
  CPU 48
  if mm.device$ = "Micromite Plus" then CPU 100
endif
Timer = 1000
Pause 200
If Abs(Timer - 1200) > 6 Then Error
dsss$ = Date$
tsss$ = Time$
Date$ = "1-2-2003" : Time$ = "4:5:6"
If Date$ <> "01-02-2003" Then Error
If Not (Time$ = "04:05:06" Or Time$ = "04:05:07") Then Error
Date$ = dsss$
Time$ = tsss$
SetTick 100, 2660
Timer = 0  : abort = 0
Do While Not abort  : Loop
If Timer < 0 Or Timer > 102 Then Error
GoTo St1
2660 SetTick 0, 0               ' interrupt routine
abort = 1
IReturn
St1: '  target of a goto
SetTick 100, St2
a = 0
Pause 1000
If a < 9 Or a > 10 Then Error
GoTo St3
St2: a = a + 1
Pause 50
IReturn
St3: SetTick 0, 0
If Pos <> 1 Then Error
WatchDog 4000
WatchDog 2000
WatchDog off
Option autorun off
Option baudrate 38400
Option break 3
Option case title
Option display 24
Option display 24, 80
Option tab 2

Clear
Dim String Temp1, Temp2
Dim Integer Result

Result = CFun_One("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz",Temp1)
Result = CFun_Two(Temp1,Temp2)
If Temp2 <> "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz" Then Error
If Result <> 62 Then Error

If Peek(CFUNADDR CFun_One) + 46 * 4 <> Peek(CFUNADDR CFun_Two) Then Error

CFunction CFun_One  00000000 ' this one is un optimised
  27bdfff0 afbe000c 03a0f021 afc40010 afc50014 8fc40010 90840000 afc40004
  8fc40004 308500ff 8fc40014 a0850000 24040001 afc40000 1000000f 00000000 ' test
  8fc50014 8fc40000 00a42021 8fc60004 8fc50000 00c52823 24a50001 8fc60010
  00c52821 90a50000 a0850000 8fc40000 24840001 afc40000 8fc40004 24850001
  8fc40000 0085202b 1480ffed 00000000 8fc20004 00001821 03c0e821 8fbe000c
  27bd0010 03e00008 00000000
End          CFunction

CFunction CFun_Two
  00000000
  90820000 24480001 2d030002 1460000a a0a20000 00822021 24030001 00a33021
  90870000 a0c70000 24630001 0068302b 14c0fffa 2484ffff 03e00008 00001821
End CFunction

clear
DIM S$(2) LENGTH 20
S$(0) = "John"
S$(1) = "Aaron"
S$(2) = "Eric"
StringSort( S$(), 3, 20 )
if S$(2) <> "John" or S$(0) <> "Aaron" or S$(1) <> "Eric" then error

CSub StringSort
     00000000
     27bdfff0 afb3000c afb20008 afb10004 afb00000 8cc30000 24630001 8cb90000
     2b250002 14a00041 00603021 10000038 00838021 01a09821 918e0000 91af0000
     01cf502b 01c01021 01ea100a 10400011 01809021 25850001 25a90001 004c5021
     90a80000 91270000 0107102b 00e8382b 0162380b 10aa0005 00e01021 14e00009
     24a50001 1000fff6 25290001 14e00005 00000000 01cf282b 14a00014 01601021
     01ee102b 54430012 27180001 01ee282b 01e5700a 11cb000d 00608821 25850001
     00ae7021 02603821 02402821 90a80000 90e90000 a0a90000 a0e80000 24a50001
     14aefffa 24e70001 00608821 27180001 01866021 0319282a 14a0ffce 01a66821
     56200005 00806021 10000009 00021fc3 240bffff 00806021 02006821 00008821
     24180001 1000ffc3 24030001 00021fc3 8fb3000c 8fb20008 8fb10004 8fb00000
     03e00008 27bd0010
End          CSub



''
' test of CFunctions with the type of parameters and arguments specified
'
if str$(Tst1(), 1, 5) <> "3.14159" then error
if Tst2("12345678", 5) <> 13 then error
if Tst3(8, "abcdefghij")  <> "abcdefgh" then error
if str$(catan2(1.5,1.6), 1, 5) <> "0.75315" then error

' // returns the value of Pi (thanks Peter Mather)
' float mypi(void){
'     return LoadFloat(0x40490FDA); //pi
' }
CFunction Tst1() float
    00000000
    27BDFFE8 AFBF0014 3C029D00 8C42009C 3C044049 0040F809 24840FDA 8FBF0014
    03E00008 27BD0018
End CFunction


' // returns an integer being the sum of i and length of s
' long long int Tst2(char* s, long long int *i) {
'     return *i + *s;
' }
CFunction Tst2(string, integer) integer
    00000000
    80840000 8CA20000 8CA30004 00821021 00042FC3 00A31821 0044202B 03E00008
    00831821
End CFunction


' // returns the first nbr chars in the string str
' char *Tst3(long long int *nbr, char *str) {
'     char *d, *s, *buff;
'     int i = *nbr + 1;
'     d = buff = GetTempMemory(256);
'     s = str;
'     while(i--) *d++ = *s++;
'     *buff = *nbr;
'     return buff;
' }
CFunction Tst3(integer, string) string
    00000000
    3C029D00 27BDFFD8 8C420040 AFB10020 AFB0001C 8C910000 AFBF0024 AFA50010
    00808021 0040F809 24040100 00401821 2402FFFF 12220009 8FA50010 26310001
    00718821 00601021 90A40000 A0440000 24420001 1451FFFC 24A50001 8E040000
    00601021 A0640000 8FBF0024 8FB10020 8FB0001C 03E00008 27BD0028
End CFunction


' // calc atan2
'  float catan2(float *yy,float *xx){
'   float x=*xx,y=*yy;
'   float PI = LoadFloat(0x40490FDA);
'   float PIby2 = LoadFloat(0x3FC90FDA);
'   float PIbyneg2 = LoadFloat(0xBFC90FDA);
'   float twoPI = LoadFloat(0x40C90FDA);
'   float R_0=LoadFloat(0x00000000);
'   float atan2ret=LoadFloat(0x00000000);
'   char p[10];
'   if (FCmp(x , 0)==1 ){
'     atan2ret = atanf(FDiv(y,x));
'   }
'   else if( (FCmp(y , R_0)!=-1) && (FCmp(x , R_0)==-1)){
'     atan2ret = FAdd(PI , atanf(FDiv(y,x))) ;
'   }
'   else if((FCmp(y , R_0)==-1) && (FCmp(x , R_0)==-1)){
'     atan2ret = FSub(atanf(FDiv(y,x)) , PI) ;
'   }
'   else if ((FCmp(y , R_0)==1) && (FCmp(x , R_0)==0) ){
'     atan2ret = PIby2 ;
'   }
'   else if ( (FCmp(y , R_0)==-1) && (FCmp(x , R_0)==0)){
'     atan2ret = PIbyneg2  ;
'   }
'   if ( FCmp(atan2ret , R_0)==-1) {
'     atan2ret = FAdd(atan2ret , twoPI );
'   }
'   FloatToStr(p,atan2ret,2,6,' ');
'   MMPrintString(p);
'   putConsole(13);
'   return atan2ret;
' }
CFunction catan2(FLOAT,FLOAT) FLOAT
    00000000
    27BDFFB0 AFBF004C AFBE0048 AFB70044 AFB60040 AFB5003C AFB20030 AFB1002C
    AFB00028 AFB40038 3C109D00 AFB30034 8E02009C 8C930000 3C044049 24840FDA
    0040F809 8CB40000 0040F021 8E02009C 3C043FC9 0040F809 24840FDA 0040A821
    8E02009C 3C04BFC9 0040F809 24840FDA 0040B821 8E02009C 3C0440C9 0040F809
    24840FDA 0040B021 8E02009C 0040F809 00002021 00408821 8E02009C 0040F809
    00002021 00409021 8E020068 02802021 0040F809 00002821 24030001 1443000B
    3C029D00 3C039D00 8C620064 8C700078 02802821 0040F809 02602021 0200F809
    00402021 10000056 00409021 8C420068 02602021 0040F809 02202821 2403FFFF
    10430016 3C029D00 8C420068 02802021 0040F809 02202821 2403FFFF 1443000F
    3C029D00 3C039D00 8C620064 8C720078 02802821 02602021 0040F809 8C70005C
    0240F809 00402021 00402821 0200F809 03C02021 1000003A 00409021 8C420068
    02602021 0040F809 02202821 2403FFFF 14430016 3C029D00 8C420068 02802021
    0040F809 02202821 2403FFFF 1443000F 3C029D00 3C039D00 8C620064 8C720078
    02802821 02602021 0040F809 8C700060 0240F809 00402021 00402021 0200F809
    03C02821 1000001E 00409021 8C420068 02602021 0040F809 02202821 24030001
    14430008 3C029D00 8C420068 02802021 0040F809 02202821 50400011 02A09021
    3C029D00 8C420068 02602021 0040F809 02202821 2403FFFF 1443000A 3C029D00
    8C420068 02802021 0040F809 02202821 50400003 02E09021 10000002 3C029D00
    3C029D00 8C420068 02402021 0040F809 02202821 2403FFFF 14430008 24020020
    3C029D00 8C42005C 02402021 0040F809 02C02821 00409021 24020020 3C109D00
    AFA20010 8E020034 02402821 24060002 24070006 0040F809 27A40018 8E02002C
    0040F809 27A40018 8E020008 0040F809 2404000D 8FBF004C 02401021 8FBE0048
    8FB70044 8FB60040 8FB5003C 8FB40038 8FB30034 8FB20030 8FB1002C 8FB00028
    03E00008 27BD0050
End CFunction



''
Print "Testing: Micromite device control statements and functions"
''
' the only digital pins that are free on all varients
'       are: 3, 4, 5, 14, 21, 22, 23 and 24
' and the only analog pins are 23 and 24

ds18b20 start 14, 0
a = DS18B20(14)
IR dev, key, 2660
IR close
if mm.device$ = "Micromite Plus" Then
        KeyPad var, 2660, 8, 29, 14, 21, 22, 23, 32
        KeyPad CLOSE
        LCD INIT 29, 14, 21, 22, 23, 32
        LCD 2, 3, "foo"
        LCD CMD 12, 13, 14
        LCD DATA 12, 13, 14
        LCD CLOSE
else
        KeyPad var, 2660, 4, 5, 14, 21, 22, 23, 24
        KeyPad CLOSE
        LCD INIT 5, 14, 21, 22, 23, 24
        LCD 2, 3, "foo"
        LCD CMD 12, 13, 14
        LCD DATA 12, 13, 14
        LCD CLOSE
endif
On KEY 2660
On KEY 0
SetPin 14, din
a = Pulsin(14, 1, 100)
If a <> -1 Then Error
a = Pulsin(14, 1, 100, 100)
If a <> -1 Then Error
setpin 14, 0

''
Print "Testing: I/O, Serial, SPI and PWM"
''
if mm.device$ = "Micromite Plus" Then
        SetPin 49, din
        SetPin 50, din, PULLUP
        SetPin 51, din, PULLDOWN
        SetPin 21, dout
        SetPin 22, dout, OC
        SetPin 23, dout
        Pin  (21) = Pin(49)
        Port   (22, 2) = Port(50, 2)
        Port(21, 2, 23, 1) = Port(50, 2, 49, 1)
        Pulse 22, 1      ' runs in the foreground
        Pulse 23, 10     ' runs in the background
        pause 12         ' allow the background to finish
        SetPin 49, OFF
        SetPin 50, OFF
        SetPin 51, OFF
        SetPin 21, OFF
        SetPin 22, OFF
        SetPin 23, OFF
        SetPin 21, inth, testl, PULLUP
        SetPin 22, intb, testl, PULLDOWN
        SetPin 21, OFF
        SetPin 22, OFF
else
        SetPin 3, din
        SetPin 4, din, PULLUP
        SetPin 5, din, PULLDOWN
        SetPin 21, dout
        SetPin 22, dout, OC
        SetPin 23, dout
        Pin(21) = Pin(3)
        Port(22, 2) = Port(4, 2)
        Port(21, 2, 23, 1) = Port(4, 2, 3, 1)
        Pulse 22, 1      ' runs in the foreground
        Pulse 23, 10     ' runs in the background
        pause 12         ' allow the background to finish
        SetPin 3, OFF
        SetPin 4, OFF
        SetPin 5, OFF
        SetPin 21, OFF
        SetPin 22, OFF
        SetPin 23, OFF
        SetPin 21, inth, testl, PULLUP
        SetPin 22, intb, testl, PULLDOWN
        SetPin 21, OFF
        SetPin 22, OFF
endif

sub testl
end sub

Open "COM1:230400, 512, INV, OC" As #1
axd$ = Input$(200, 1)
If Not Eof(#1) Then Error
If Not Eof(1) Then Error
If Lof(#1) <> 256 Then Error
Print #1, "Hello"

Open "COM2:" As #2
Print #2, "Hello"
Close #1, #2


Open "COM2:19200, 512" As #10
axd$ = Input$(200, 10)
If Not Eof(#10) Then Error
If Not Eof(10) Then Error
If Lof(#10) <> 256 Then Error
Print #10, "Hello"
Close #10

if mm.device$ = "Micromite Plus" then
    ' test COM2 and 3
    Open "COM2:1000000, 512, INV" As #10
    Open "COM3:" As #2
    Print #2, "Hello"
    zzss$ = Input$(4, 10):zzss$ = Input$(4, 2)
    Close #10, #2
endif

PWM 1, 500000, 50
PWM 2, 500, 25
PWM 1, stop
PWM 2, stop

Servo 1, 1.2, 1.3
Servo 1, 50, 0.8, 0.9
Servo 1, stop
Servo 2, stop

Function spitst(a)
 spitst = 77
End Function

SPI open 100000, 3, 8
junk = SPI(&H80)          ' send the command and ignore the return
byte1 = SPI(0)            ' get the first byte from the slave
byte2 = SPI(0)            ' get the second byte from the slave
SPI write 1, 44
SPI write 3, 44, 45, 46
Clear
a$ = "MN"
SPI write 1, a$
SPI write 2, a$
Dim foo(2) As integer = (44, 55, 66)
SPI write 1, foo()
SPI write 2, foo()
SPI write 3, foo()
SPI write 1, foo(1)
SPI write 2, foo(1)
SPI write 1, spitst(44)
doo = 88
SPI write 1, doo
SPI close

if mm.device$ = "Micromite Plus" then
    if peek(byte peek(word &H9D000090) + 48) = 0 and peek(byte peek(word &H9D000090) + 22) = 0 then
        ' SD card or touch is not configured - so we can test SPI2
        SPI2 open 100000, 3, 8
        junk = SPI2(&H80)          ' send the command and ignore the return
        byte1 = SPI2(0)            ' get the first byte from the slave
        byte2 = SPI2(0)            ' get the second byte from the slave
        SPI2 write 1, 44
        SPI2 write 3, 44, 45, 46
        Clear
        a$ = "MN"
        SPI2 write 1, a$
        SPI2 write 2, a$
        Dim foo(2) As integer = (44, 55, 66)
        SPI2 write 1, foo()
        SPI2 write 2, foo()
        SPI2 write 3, foo()
        SPI2 write 1, foo(1)
        SPI2 write 2, foo(1)
        SPI2 write 1, spitst(44)
        doo = 88
        SPI2 write 1, doo
        SPI2 close
    endif
endif


''
'    Testing: SD card file input/output.
''
Clear
if mm.device$ = "Micromite Plus" Then
   If peek(byte peek(word &H9D000090) + 48) = 0 then
      Print "         SD card not configured - skiping file input/output tests"
   Else
        ON ERROR SKIP
        Open "NOSUCHFI" For INPUT As #1
        If MM.Errno = 6 Then
            Print "Testing: SD card file input/output.   ";
            DoTestFileIO
        Else
            Print "         No SD card - skiping file input/output tests"
        EndIf
    Endif
EndIf
Sub DoTestFileIO
            Timer = 0
            Open "TEST.TXT" For output As #1
            Print #1, 1234; ","; "hello" "," 9876
            Close #1
            Open "test.txt" For INPUT As #4
            Input #4, a, b$, c
            If a <> 1234 Or b$ <> "hello" Or c <> 9876 Then Error
            Close #4
            Open "TEST.TXT" For output As #10
            Print #10, "abababab";
            Close #10
                                                For i = 1 To 10
                                                  Open "TEST.TXT" For input As i
                                                Next i
                                                For i = 1 To 10
                                                  If Input$(2, i) <> "ab" Then Error
                                                Next i
                                                For i = 10 To 1 Step -1
                                                  Close i
                                                Next i
            Open "test.txt" For INPUT As 3
            a = 0
            While Not Eof(3)
              a = a + 1
              t$ = Input$(2, 3)
              If t$ <> "ab" Then Error
            Wend
            Close 3
            If a <> 4 Then Error
                  mkdir "testdir1"
                  chdir "testdir1"
            mkdir "testdir2"
            chdir "testdir2"
            '? cwd$
                  if right$(CWD$, 17) <> "testdir1/testdir2" and right$(CWD$, 17) <> "testdir1\testdir2" Then Error
            chdir ".."
                  if right$(CWD$, 8) <> "testdir1" Then Error
            Kill "..\test.txt"
                  thisdir$ = CWD$
            chdir "\"
                  chdir thisdir$
                  chdir ".."
            rmdir "testdir1\testdir2"
            rmdir "testdir1"
            Open "Long Test File Name Hello This Is a Long File Name.tst" For output As #3
                  if mm.device$ <> "DOS" Then
                    If (peek(word &HBF885060) and 1) = 0 then CPU 30
                  End If
            Print #3, 34 "," 67 ",ppoo," 6.7
                  if mm.device$ <> "DOS" then CPU 100
            Close #3
            Open "Long Test File Name Hello This Is a Long File Name.tst" For INPUT As #3
            Input #3, a, x, s$, c
            Close #3
            If s$ <> "ppoo" Then Error
            Open "Long Test File Name Hello This Is a Long File Name.tst" For append As #3
            Print #3, 12 "," 89 ", ZZoo ," 10.7
            Close #3
            Open "Long Test File Name Hello This Is a Long File Name.tst" For INPUT As #3
            Line Input #3, s$
            If s$ <> " 34, 67,ppoo, 6.7" Then Error
            Input #3, a, x, s$, c
            Close #3
            If s$ <> "ZZoo" Then Error
            Kill "Long Test File Name Hello This Is a Long File Name.tst"
            Open "tt.dat" For output As #5
            Print #5, "test"
            Close #5
                  if mm.device$ <> "DOS" then
                    t$ = Dir$("*.*", FILE)
                    Do While ucase$(t$) <> "TT.DAT"
                        If t$ = "" Then Error
                        t$ = Dir$()
                    Loop
                  end if
            Kill "tt.dat"
            on error ignore
            on error abort

            ' test random file access
            ' create the test file
            Open "test.dat" For output As #1
            For i = 0 To 50
            Print #1, Str$(i) + Space$(30-Len(Str$(i)))
            Next i
            Close #1

            ' test random access
            Open "test.dat" For random As #5
            If Lof(#5) <> 32*51 Then Error
            If Loc(#5) -1 <> Lof(5) Then Error
            Seek #5, 321
            If Loc(5) <> 321 Then Error
            If Val(Input$(32, 5)) <> 10 Then Error
            Seek 5, 641
            If Val(Input$(32, 5)) <> 20 Then Error
            Seek 5, 321
            Print #5, Str$(999) + Space$(27)
            Seek 5, 961
            If Val(Input$(32, 5)) <> 30 Then Error
            Seek #5, 321
            If Val(Input$(32, 5)) <> 999 Then Error
            Seek 5, Lof(5) + 1
            Print #5, Str$(444) + Space$(27)
            If Lof(#5) <> 32*52 Then Error
            Seek #5, 32*51 + 1
            If Val(Input$(32, 5)) <> 444 Then Error
            Close #5
            Kill "test.dat"
            Print Timer " mSec"
end sub

'
Clear
Option explicit
Dim aa, bb$, c, x%, y%(1024)
VAR restore
If aa <> 1234 Or bb$ <> "This is a test" Or  c <> 56789 Or x% <> 123456789012345 Then Error

DosSkipHardwareTests:
if mm.device$ = "DOS" then
    Print "Testing: File input/output.   ";
    DoTestFileIO
endif
Print :Memory
Print :Print "*** All tests finished OK ***"