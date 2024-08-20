Option Base 0
Option Default None
Option Explicit

Const ENABLE_INTERRUPT = 1

Dim play_mode$ = ""

If InStr(Mm.CmdLine$, "flac") Then
  play_mode$ = "flac"
ElseIf InStr(Mm.CmdLine$, "mod") Then
  play_mode$ = "mod"
ElseIf InStr(Mm.CmdLine$, "mp3") Then
  play_mode$ = "mp3"
ElseIf InStr(Mm.CmdLine$, "wav") Then
  play_mode$ = "wav"
Else
  Error "Play mode argument missing"
EndIf

If ENABLE_INTERRUPT Then ? "Interrupt enabled"

Execute "test_" + play_mode$
? "Press Ctrl-C to exit"
Do : Loop

Sub test_flac()
  If ENABLE_INTERRUPT Then
    Play Flac "stereo-test.flac", on_interrupt
  Else
    Play Flac "stereo-test.flac"
  EndIf
End Sub

Sub test_mod()
  If ENABLE_INTERRUPT Then
    Play ModFile "gems-n-rocks.mod", 44100, on_interrupt
  Else
    Play ModFile "gems-n-rocks.mod", 44100
  EndIf
End Sub

Sub test_mp3()
  If ENABLE_INTERRUPT Then
    Play Mp3 "stereo-test.mp3", on_interrupt
  Else
    Play Mp3 "stereo-test.mp3"
  EndIf
End Sub

Sub test_wav()
  If ENABLE_INTERRUPT Then
    Play Wav "stereo-test.wav", on_interrupt
  Else
    Play Wav "stereo-test.wav"
  EndIf
End Sub

Sub on_interrupt()
  ? "Interrupt fired"
End Sub
