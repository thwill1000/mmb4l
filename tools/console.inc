' Copyright (c) 2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

On Error Skip 1 : Dim sys.VERSION$ = ""
If sys.VERSION$ = "" Then Error "'system.inc' not included"
sys.requires("array")
sys.provides("console")
If sys.err$ <> "" Then Error sys.err$

Sub con.bell()
  Console Bell
End Sub

Sub con.cls()
  Cls
End Sub

Sub con.cursor_previous()
  Print Chr$(27) "[F";
End Sub

Sub con.errorln(s$)
  Print s$
End Sub

Sub con.foreground(fg$)
  Select Case LCase$(fg$)
    Case "black"   : ? Chr$(27) "[30m";
    Case "red"     : ? Chr$(27) "[31m";
    Case "green"   : ? Chr$(27) "[32m";
    Case "yellow"  : ? Chr$(27) "[33m";
    Case "blue"    : ? Chr$(27) "[34m";
    Case "magenta", "purple" : ? Chr$(27) "[35m";
    Case "cyan"    : ? Chr$(27) "[36m";
    Case "white"   : ? Chr$(27) "[37m";
    Case "default", "reset" : ? Chr$(27) "[0m";
    Case Else      : Error : "Unknown ANSI colour: " + fg$
  End Select
End Sub

Sub con.print(s$)
  Print s$;
End Sub

Sub con.println(s$)
  Print s$
End Sub

' Gets a string element from the command history.
'
' @param  h%()  buffer containing the command history.
' @param  idx%  index of the element to retrieve, the first element is always
'               idx% = 0 irrespective of OPTION BASE.
' @return       string element, or empty string if idx% >= number of elements.
Function con.history_get$(h%(), idx%)
  If idx% < 0 Then Error "index out of bounds: " + Str$(idx%)

  Local h_addr% = Peek(VarAddr h%())
  Local h_size% = (Bound(h%(), 1) - Bound(h%(), 0) + 1) * 8
  Local i%, p% = h_addr%
  For i% = 0 To idx%
    If Peek(Byte p%) = 0 Then Exit Function ' Return empty string.
    If i% < idx% Then Inc p%, Peek(Byte p%) + 1
  Next
  If p% + Peek(Byte p%) < h_addr% + h_size% Then
    Memory Copy p%, Peek(VarAddr con.history_get$), Peek(Byte p%) + 1
  EndIf
End Function

Sub con.history_fill(h%(), items$())
  Local h_addr% = Peek(VarAddr h%())
  Local h_size% = (Bound(h%(), 1) - Bound(h%(), 0) + 1) * 8
  Memory Set h_addr%, &h00, h_size%
  Local i%
  For i% = Bound(items$(), 1) To Bound(items$(), 0) Step -1
    con.history_put(h%(), items$(i%))
  Next
End Sub

Function con.history_find%(h%(), needle$)
  If needle$ = "" Then Error "invalid argument: needle$"

  ' TODO: could be faster by not calling con.history_get()
  Local i% = -1, s$
  Do
    Inc i%
    s$ = con.history_get$(h%(), i%)
  Loop Until s$ = "" Or s$ = needle$

  con.history_find% = Choice(s$ = "", -1, i%)
End Function

Sub con.history_put(h%(), s$)
  If s$ = "" Then Error "invalid empty string"

  Local h_addr% = Peek(VarAddr h%())
  Local h_size% = (Bound(h%(), 1) - Bound(h%(), 0) + 1) * 8
  If Peek(Byte h_addr%) > 0 Then
    Memory Copy h_addr%, h_addr% + 1 + Len(s$), h_size% - 1 - Len(s$)
  EndIf
  Memory Copy Peek(VarAddr s$), h_addr%, Len(s$) + 1
End Sub

Function con.readln$(initial$, history%())
  Local ch$, hidx%, p%, old$, overwrite%, s$

  con.readln$ = initial$
  p% = Len(initial$) + 1
  Print initial$;
  hidx% = Choice(initial$ = "", -1, con.history_find%(history%(), initial$))

  Do While Not sys.break_flag%
    ch$ = Inkey$
    If ch$ = "" Then Continue Do

    Select Case Asc(ch$)
      Case 8 ' Backspace
        If p% = 1 Then
          con.bell()
        Else
          ' Delete the character immediately to the left.
          s$ = Mid$(con.readln$, p%)
          con.readln$ = Left$(con.readln$, p% - 2) + s$
          Print Chr$(&h08) s$ " " String$(Len(s$) + 1, &h08);
          Inc p%, -1
        EndIf

      Case 10, 13 ' Enter
        Print
        If con.readln$ <> "" Then con.history_put(history%(), con.readln$)
        Exit Do

      Case 127 ' Delete
        If p% = Len(con.readln$) + 1 Then
          con.bell()
        Else
          ' Delete the current character.
          s$ = Mid$(con.readln$, p% + 1)
          con.readln$ = Left$(con.readln$, p% - 1) + s$
          Print s$ " " String$(Len(s$) + 1, &h08);
        EndIf

      Case 128, 129 ' Up, Down
        If hidx% = -1 Then old$ = con.readln$
        Inc hidx%, Choice(Asc(ch$) = 128, 1, -1)
        If hidx% <= -1 Then
          hidx% = -1
          s$ = old$
        Else
          s$ = con.history_get$(history%(), hidx%)
          If s$ = "" Then s$ = con.readln$ : Inc hidx%, -1
        EndIf

        Print String$(p% - 1, &h08) String$(p% - 1, " ") String$(p% - 1, &h08) s$;
        con.readln$ = s$
        p% = Len(s$) + 1

      Case 130 ' Left
        If p% > 1 Then
          Inc p%, -1
          Print Chr$(27) "[D";
        EndIf

      Case 131 ' Right
        If p% <= Len(con.readln$) Then
          Inc p%, 1
          Print Chr$(27) "[C";
        EndIf

      Case 132 ' Insert
        overwrite% = Not overwrite%

      Case 134 ' Home
        Print String$(p% - 1, &h08);
        p% = 1

      Case 135 ' End
        Do While p% <= Len(con.readln$)
          Print Chr$(27) "[C";
          Inc p%
        Loop

      Case < 32, > 126
        ' Print "<" Str$(Asc(ch$)) ">";
        con.bell()

      Case Else
        If Len(con.readln$) = 255 Then
          con.bell()
        ElseIf p% > Len(con.readln$) Then
          Cat con.readln$, ch$
          Print ch$;
          Inc p%
        ElseIf overwrite% Then
          con.readln$ = Left$(con.readln$, p% - 1) + ch$ + Mid$(con.readln$, p% + 1)
          Print ch$;
          Inc p%
        Else
          s$ = Mid$(con.readln$, p%)
          con.readln$ = Left$(con.readln$, p% - 1) + ch$ + s$
          Print ch$ s$ String$(Len(s$), &h08);
          Inc p%
        EndIf
    End Select
  Loop

  If sys.break_flag% Then con.readln$ = ""
End Function
