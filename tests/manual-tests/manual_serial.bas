' Copyright (c) 2021-2022 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 5.07

' Semi-automated test for serial I/O.

Option Base 0
Option Default None
Option Explicit On

If Mm.Device$ = "PicoMite" Then
  Const DEVICE$ = "COM1"
Else
  Const DEVICE$ = "/dev/ttyS1"
  ' Const DEVICE$ = "/dev/ttyUSB0"
EndIf

Dim BAUDS%(3) = (2400, 9600, 115200, 921600)
Dim ISIZES%(5) = (1, 2, 3, 4, 60, 120)

' The Chr$(152) is just so there is some non-ASCII data being sent.
Const MESSAGE$ = "Hello World" + Chr$(152)

Dim LONG_MESSAGE$ = "Moses supposes his toeses are roses, but"
Cat LONG_MESSAGE$,  " Moses supposes eroneously, for nobodies"
Cat LONG_MESSAGE$,  " toeses are roses as Moses supposes his "
Cat LONG_MESSAGE$,  "toeses to be. Moses supposes his toeses "
Cat LONG_MESSAGE$,  "are roses, but Moses supposes eroneously"
Cat LONG_MESSAGE$,  ", for nobodies toeses are roses as Mose" + Chr$(152)

If Len(LONG_MESSAGE$) <> 240 Then Error "Wrong message length"
Dim g_count%, g_size%, g_received$

main()
End

Sub main()

  Local i%, s$

  Do While Inkey$ <> "" : Loop
  Print "Start [R]eceiving or [T]ransmitting first ?"
  Do
    s$ = UCase$(Inkey$)
  Loop Until s$ = "R" Or s$ = "T"

  If s$ = "R" Then
    For i% = Bound(BAUDS%(), 0) To Bound(BAUDS%(), 1)
      do_receives_with_interrupts(BAUDS%(i%))
      do_transmits_for_interrupts(BAUDS%(i%))
      do_receives(BAUDS%(i%))
      do_transmits(BAUDS%(i%))
    Next
  Else
    For i% = Bound(BAUDS%(), 0) To Bound(BAUDS%(), 1)
      do_transmits_for_interrupts(BAUDS%(i%))
      do_receives_with_interrupts(BAUDS%(i%))
      do_transmits(BAUDS%(i%))
      do_receives(BAUDS%(i%))
    Next
  EndIf

  Print "DONE"

End Sub

Sub do_receives(baud%)
  receive(Str$(baud%))
  receive(Str$(baud%) + ",S2")
  receive(Str$(baud%) + ",EVEN")
  receive(Str$(baud%) + ",ODD")
  receive(Str$(baud%) + ",S2,EVEN")
  receive(Str$(baud%) + ",ODD,7BIT",  "Hello World" + Chr$(152 And &h7F))
  receive(Str$(baud%) + ",7BIT,EVEN", "Hello World" + Chr$(152 And &h7F))
  receive(Str$(baud%) + ",7BIT",      "Hello World" + Chr$(152 And &h7F))
End Sub

Sub do_transmits(baud%)
  transmit(Str$(baud%))
  transmit(Str$(baud%) + ",S2")
  transmit(Str$(baud%) + ",EVEN")
  transmit(Str$(baud%) + ",ODD")
  transmit(Str$(baud%) + ",S2,EVEN")
  transmit(Str$(baud%) + ",ODD,7BIT")
  transmit(Str$(baud%) + ",7BIT,EVEN")
  transmit(Str$(baud%) + ",7BIT")
End Sub

Sub do_receives_with_interrupts(baud%)
  Local i%
  For i% = Bound(ISIZES%(), 0) To Bound(ISIZES%(), 1)
    receive_with_interrupt(baud%, ISIZES%(i%))
  Next i%
End Sub

Sub do_transmits_for_interrupts(baud%)
  Local i%
  For i% = Bound(ISIZES%(), 0) To Bound(ISIZES%(), 1)
    transmit_for_interrupt(baud%)
  Next
End Sub

Sub receive(spec$, expected$)
  Pause 500

  Local expected_$ = Choice(expected$ = "", MESSAGE$, expected$)
  Print "RECEIVE from " DEVICE$ ":" spec$ " ... ";
  If DEVICE$ = "COM1" Then SetPin GP1, GP0, COM1
  Open DEVICE$ + ":" + spec$ As #1
  Local msg$

  Do
    Line Input #1, msg$
    If msg$ <> "" Then Exit Do
    Pause 100
  Loop

  Print Choice(msg$ = expected_$, "OK", "FAIL: " + format_received$(msg$))
  send_ack()
  Pause 100
  Close #1
End Sub

Function format_received$(s$)
  Local ch%, i%
  For i% = 1 To Len(s$)
    ch% = Peek(Var s$, i%)
    Select Case ch%
      Case 32 To 127
        Cat format_received$, Chr$(ch%)
      Case Else
        Cat format_received$, "<" + Str$(ch%) + ">"
    End Select
  Next
End Function

Sub transmit(spec$)
  Pause 1000

  Print "TRANSMIT to " DEVICE$ ":" spec$
  If DEVICE$ = "COM1" Then SetPin GP1, GP0, COM1
  Open DEVICE$ + ":" + spec$ As #1
  Print #1, MESSAGE$
  receive_ack()
  Close #1
End Sub

Sub receive_with_interrupt(baud%, size%)
  Pause 500

  Local spec$ = Str$(baud%) + ",4096,my_interrupt," + Str$(size%)
  Print "RECEIVE (with interrupt) from " DEVICE$ ":" spec$
  If DEVICE$ = "COM1" Then SetPin GP1, GP0, COM1
  Open DEVICE$ + ":" + spec$ As #1
  g_count% = 0
  g_size% = size%
  g_received$ = ""
  Local timeout% = Timer + 5000
  Do While (Len(g_received$) < 240) And (Timer < timeout%) : Loop

  Local ok% = (g_received$ = LONG_MESSAGE$)
  If Not ok% Then Print "FAIL: " + format_received$(g_received$)
  If ok% Then
    ok% = (g_count% = 240 \ size%)
    If Not ok% Then Print "FAIL: expected " + (240 \ size%) + " interrupts, but got " + Str$(g_count%)
  EndIf
  If ok% Then Print "OK, " Str$(g_count%) " interrupts"

  send_ack()
  Pause 100
  Close #1
End Sub

Sub transmit_for_interrupt(baud%)
  Pause 1000

  Local spec$ = Str$(baud%)
  Print "TRANSMIT to " DEVICE$ ":" spec$
  If DEVICE$ = "COM1" Then SetPin GP1, GP0, COM1
  Open DEVICE$ + ":" + spec$ As #1
  Print #1, LONG_MESSAGE$;
  receive_ack()
  Close #1
End Sub

Sub my_interrupt()
  Cat g_received$, Input$(g_size%, #1)
  Inc g_count%
End Sub

Sub send_ack()
  Print "Sent ACK"
  Print #1, "ACK"
End Sub

Sub receive_ack()
  Print "Waiting for ACK"
  Local msg$, s$
  Do While Not InStr(msg$, "ACK")
    Pause 100
    s$ = Input$(255, #1)
    Cat msg$, s$
  Loop
  Print "Received ACK: " format_received$(msg$)
End Sub
