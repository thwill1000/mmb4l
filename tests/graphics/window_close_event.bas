Option Base 0
Option Default None
Option Explicit

Dim call_depth% = -1
Dim count% = 0
Dim x%, y%, w%, h%

For count% = 0 To 19
  x% = Int(1000 * Rnd()) + 20
  y% = Int(1000 * Rnd()) + 20
  w% = Int(300 * Rnd()) + 100
  h% = Int(300 * Rnd()) + 100
  Graphics Window count%, x%, y%, w%, h%, 1, on_window_close
Next

On Key on_key

Do While count% > 0
  If call_depth% <> Mm.Info(CALLDEPTH) Then
    call_depth% = Mm.Info(CALLDEPTH)
    Print "Call depth: "; Str$(call_depth%)
  EndIf
Loop

on_key:

'Sub on_key()
  Print "Entering on_key() interrupt"
  If call_depth% <> Mm.Info(CALLDEPTH) Then
    call_depth% = Mm.Info(CALLDEPTH)
    Print "Call depth: "; Str$(call_depth%)
  EndIf
  Do While Inkey$ <> "" : Loop
  Print "Exiting interrupt"
'End Sub
IReturn

Sub on_window_close(window_id%, event_id%)
  Print "Entering on_window_close() interrupt"
  If call_depth% <> Mm.Info(CALLDEPTH) Then
    call_depth% = Mm.Info(CALLDEPTH)
    Print "Call depth: "; Str$(call_depth%)
  EndIf
  Print "Window Id: "; Str$(window_id%); ", Event Id:"; Str$(event_id%)
  Graphics Destroy window_id%
  Inc count%, -1
  Print "Exiting interrupt"
End Sub
