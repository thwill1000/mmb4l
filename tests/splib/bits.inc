' Copyright (c) 2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

'!remove_if NO_INCLUDE_GUARDS
On Error Skip 1 : Dim sys.VERSION$ = ""
If sys.VERSION$ = "" Then Error "'system.inc' not included"
sys.provides("bits")
If sys.err$ <> "" Then Error sys.err$
'!endif

' Sets bit i% of x%.
Sub bits.set(x%, i%)
  If i% < 0 Or i% > 63 Then Error "i% out of 0 .. 63 range"
  x% = x% Or 1 << i%
End Sub

' Clears bit i% of x%.
Sub bits.clear(x%, i%)
  If i% < 0 Or i% > 63 Then Error "i% out of 0 .. 63 range"
  x% = x% And Inv (1 << i%)
End Sub

' Gets bit i% of x%.
Function bits.get%(x%, i%)
  If i% < 0 Or i% > 63 Then Error "i% out of 0 .. 63 range"
  bits.get% = (x% And 1 << i%) > 0
End Function

