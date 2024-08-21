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
ElseIf InStr(Mm.CmdLine$, "sound") Then
  play_mode$ = "sound"
ElseIf InStr(Mm.CmdLine$, "tone") Then
  play_mode$ = "tone"
ElseIf InStr(Mm.CmdLine$, "wav") Then
  play_mode$ = "wav"
Else
  Error "Play mode argument missing"
EndIf

If ENABLE_INTERRUPT Then ? "Interrupt enabled"

Execute "test_" + play_mode$
? "Press Ctrl-C to exit"
Do
  If UCase$(Inkey$) = "W" And play_mode$ = "mod" Then test_wav_effect()
Loop

Sub test_wav_effect()
  ? "Effect!"
  If ENABLE_INTERRUPT Then
    Play Effect "CantinaBand3.wav", on_effect_interrupt
  Else
    Play Effect "CantinaBand3.wav"
  EndIf
End Sub

Sub test_flac()
  If ENABLE_INTERRUPT Then
    Play Flac "stereo-test.flac", on_interrupt
  Else
    Play Flac "stereo-test.flac"
  EndIf
End Sub

Sub test_mod()
  If ENABLE_INTERRUPT Then
    Play ModFile "gems-n-rocks.mod", 22050, on_interrupt
  Else
    Play ModFile "gems-n-rocks.mod", 22050
  EndIf
  ? "Press W to play WAV sound effect"
End Sub

Sub test_mp3()
  If ENABLE_INTERRUPT Then
    Play Mp3 "stereo-test.mp3", on_interrupt
  Else
    Play Mp3 "stereo-test.mp3"
  EndIf
End Sub

Sub test_sound()
  Const MIDDLE_C = 261.63
  Local i%

  For i% = 1 To 4
    ? "Sound " + Str$(i%) + " - BOTH"
    Play Sound i%, B, S, MIDDLE_C
    Pause 1000
  Next

  For i% = 1 To 4
    Play Sound i%, B, O
  Next
  Pause 1000

  For i% = 1 To 4
    ? "Sound " + Str$(i%) + " - LEFT"
    Play Sound i%, L, S, MIDDLE_C
    Pause 1000
  Next

  For i% = 1 To 4
    Play Sound i%, B, O
  Next
  Pause 1000

  For i% = 1 To 4
    ? "Sound " + Str$(i%) + " - RIGHT"
    Play Sound i%, R, S, MIDDLE_C
    Pause 1000
  Next

  For i% = 1 To 4
    Play Sound i%, B, O
  Next
  Pause 1000
End Sub

Sub test_tone()
  Const MIDDLE_C = 261.63
  If ENABLE_INTERRUPT Then
    Play Tone MIDDLE_C, MIDDLE_C, 1000, on_interrupt
  Else 
    Play Tone MIDDLE_C, MIDDLE_C, 1000
  EndIf
End Sub

Sub test_wav()
  If ENABLE_INTERRUPT Then
    Play Wav "CantinaBand3.wav", on_interrupt
  Else
    Play Wav "stereo-test.wav"
  EndIf
End Sub

Sub on_interrupt()
  ? "Main interrupt fired"
End Sub

Sub on_effect_interrupt()
  ? "Effect interrupt fired"
End Sub
