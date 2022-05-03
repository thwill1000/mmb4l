' Copyright (c) 2021-2022 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 5.07

' This file is for testing nano syntax highlighting,
' it's actual contents are nonsense.

Print "4 squares in basic" : Print
1 Print "4 squares in basic" : Print
2 S=1:P=7:U=0:'1 to 7
3 GoSub 9
4 S=3:P=9:U=0:'3 to 9
5 GoSub 9
6 S=0:P=9:U=11'1 to 9 not unique
7 GoSub 9
8 End
9 Timer = 0:Print:I=0:For A=S To P

Dim s$ = "foo ' bar" ' bar "foo"
