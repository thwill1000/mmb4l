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
  Graphics Window count%, w%, h%, x%, y%, "Window " + Str$(count%), 1, on_window_event
Next

On Key on_key

Do While count% > 0
  If call_depth% <> Mm.Info(CALLDEPTH) Then
    call_depth% = Mm.Info(CALLDEPTH)
    If call_depth% <> 0 Then Print "Non zero call depth: "; Str$(call_depth%)
  EndIf
Loop

End

Sub on_key()
  Print "Entering on_key() interrupt"
  If call_depth% <> Mm.Info(CALLDEPTH) Then
    call_depth% = Mm.Info(CALLDEPTH)
    Print "  Call depth: "; Str$(call_depth%)
  EndIf
  Do While Inkey$ <> "" : Loop
  Print "Exiting interrupt"
End Sub

Sub on_window_event(window_id%, event_id%)
  Print "Entering on_window_event() interrupt"
  If call_depth% <> Mm.Info(CALLDEPTH) Then
    call_depth% = Mm.Info(CALLDEPTH)
    Print "  Call depth: "; Str$(call_depth%)
  EndIf

  Local event_string$
  Select Case event_id%
    Case WINDOW_EVENT_CLOSE: event_string$ = "CLOSE"
    Case WINDOW_EVENT_FOCUS_GAINED: event_string$ = "FOCUS_GAINED"
    Case WINDOW_EVENT_FOCUS_LOST: event_string$ = "FOCUS_LOST"
    Case WINDOW_EVENT_MINIMISED: event_string$ = "MINIMISED"
    Case WINDOW_EVENT_MAXIMISED: event_string$ = "MAXIMISED"
    Case WINDOW_EVENT_RESTORED: event_string$ = "RESTORED"
    Case Else: event_string$ = Str$(event_id%)
  End Select

  Print "  Window Id: "; Str$(window_id%)
  Print "  Event Id: "; event_string$
  If event_id% = WINDOW_EVENT_CLOSE Then
    Graphics Destroy window_id%
    Inc count%, -1
  EndIf
  Print "Exiting interrupt"
End Sub
