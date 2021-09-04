' ta.bas, written by P Lepkowski, 7/8/2018, for public domain use

Cls
Print "TA.BAS determines the running time for repetitive actions"
Print "running under the MMBasic inerpreter on different CPUs."
Print
Print "It was written to establish the comparative processing speeds"
Print "of the MM, MM+, MMX, and DOS environments for intensive string"
Print "and file handling routines which will give you some relative idea"
Print "of the processing speed in the PIC environment after you test"
Print "string intensive routines under DOS."
Print
Print "TAIN.TXT should be placed in the same directory as TA.BAS."
Print "TA.BAS will read input strings repetitively from TAIN.TXT."
Print
Print "TA.BAS will ask you to type in a computer description for identification"
Print "of the output data which will be appended to TAOUT.TXT. Please preserve"
Print "the TAOUT.TXT file and post it back here for mutual analysis."
Print
Print "TA.BAS will write 1,000,000 line into a 47MB file, TAJUNK.TXT, which"
Print "can be discarded after the test. Make sure that the disk or SD card"
Print " has enough free space for this file before starting."
Print
Print "TA.BAS runs five loops as follows:"
Print " 1. 1,000,000 repetitions of an empty loop"
Print " 2. 10,000 repetitions of a loop which reads 100 lines from TAIN.TXT"
Print " 3. writes a new TAJUNK.TXT file containing 1,000,000 lines of text"
Print " 4. 900,000 repetitions of a loop which extracts a MID$() from a string"
Print " 5. 1,000,000 repetitions of a loop which takes a VAL() from a string"
Print
Print "The program runs in about 14 seconds on my Dell I5 3.4 GHz desktop."
Print
Print "Please input a description of this computer or a null string to Cancel."
Line Input " .... ?",cn$
Print cn$
If cn$="" Then Print "Cancelled.":End

On error skip
Open "tain.txt" For input As #1
If MM.Errno>0 Then
Print
Print "I can't continue because I can't open the required TAIN.TXT file."
Print "Please copy the TAIN.TXT file into the current directory and try again."
End
EndIf
Close #1

t$="101.1222.2333.3444.4555.5666.6777.7888.8999.9"
in$="":pp$=" ms per iteration.":ms$="ms or "
Open "taout.txt" For append As #2 'append as #2
Open "tajunk.txt" For output As #3
b$="Running on computer '"+cn$+"' on "+Date$+" at "+Time$
Print b$:printz(b$)
sa=Timer

nam$="1,000,000 iterations of empty loop":printpa
s1=Timer:For i=1 To 1000000:Next:s1t=Timer-s1:s1to=s1t/1000000:printpb

nam$="reading 100 line file 10,000 times":printpa
s1=Timer:readfile:s1t=Timer-s1:s1to=s1t/1000000:printpb

nam$="writing 1,000,000 line file":printpa
s1=Timer:printfile:s1t=Timer-s1:s1to=s1t/1000000:printpb

nam$="parse 9 strings from 100,000 strings":printpa
s1=Timer:parselines:s1t=Timer-s1:s1to=s1t/900000:printpb

nam$="extracting the value from a string 1,000,000 times":printpa
s1=Timer:extractvalues:s1t=Timer-s1:s1to=s1t/1000000:printpb

sat=Timer-sa:sato=sat/4900000
'a$=Str$(sat,2)+ms$+Str$(sato,0,4)+pp$
b$="Finished 4,900,000 iterations on computer '"+cn$+"' in "+Str$(sat,2)+ms$+Str$(sato,0,4)+pp$
Print b$:printz(b$)
b$="==============================================="
Print b$:printz(b$)
Close #2:Close #3
End

Sub printfile
Local t$="101.1222.2333.3444.4555.5666.6777.7888.8999.9"
For i=1 To 1000000
Print #3,t$
Next
End Sub 'printfile

Sub readfile
For i=1 To 10000
Open "tain.txt" For input As #1
For j=1 To 100 '900
Line Input #1, in$
Next j
Close #1
Next i
End Sub 'readfile

Sub parselines
Local t$="101.1222.2333.3444.4555.5666.6777.7888.8999.9"
For i=1 To 100000
For j=1 To 9
v$=Mid$(t$,((j-1)*5)+1,5)
Next j
Next i
End Sub 'parseline

Sub extractvalues
Local t$="222.2", v=0
For i=1 To 1000000
v=Val(t$)
Next
End Sub 'extractvalues

Sub print42
Print #2," A = "; Str$(a,2);" B = "; Str$(b,2);
Print #2," C = "; Str$(c,2);" D = "; Str$(d,2);
Print #2," loop 4 time = ";Str$(sdt,2);ms$;Str$(sdto,0,4);pp$
End Sub 'print42

Sub printpa
b$="Starting "+(nam$)+" ... ":? b$:printz(b$)
End Sub 'printpa

Sub printpb
b$="Finished "+nam$+" in "+Str$(s1t,2)+ms$+Str$(s1to,0,4)+pp$
Print b$:printz(b$):printz("")
End Sub 'printpb

Sub printz(p$)
Print #2,p$:? #3,p$
End Sub 'printz()
