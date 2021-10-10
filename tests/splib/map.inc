' Copyright (c) 2020-2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

'!remove_if NO_INCLUDE_GUARDS
On Error Skip 1 : Dim sys.VERSION$ = ""
If sys.VERSION$ = "" Then Error "'system.inc' not included"
sys.requires("array")
sys.provides("map")
If sys.err$ <> "" Then Error sys.err$
'!endif

' Gets the upper-bound that should be used to dimension an array to hold a map
' of the given capacity, irrespective of OPTION BASE.
'
' e.g. To create a string array that can be used as a map of 10 elements:
'        Dim my_map$(map.new%(10))
Function map.new%(capacity%)
  map.new% = 2 * capacity% + Mm.Info(Option Base)
End Function

' Initialises the map.
Sub map.init(mp$())
  Local lb% = Bound(mp$(), 0)
  Local ub% = Bound(mp$(), 1)
  Local i%
  For i% = lb% To ub% : mp$(i%) = sys.NO_DATA$ : Next
  mp$(ub%) = "0"
End Sub

' Gets the capacity of the map.
Function map.capacity%(mp$())
  map.capacity% = (Bound(mp$(), 1) - Bound(mp$(), 0)) \ 2
End Function

' Clears the keys and values and resets the size to 0.
Sub map.clear(mp$())
  Local lb% = Bound(mp$(), 0)
  Local ub% = Bound(mp$(), 1)
  Local off% = (ub% - lb%) \ 2
  Local sz% = Val(mp$(ub%))
  Local i%
  For i% = lb% To lb% + sz% - 1
    mp$(i%) = sys.NO_DATA$
    mp$(i% + off%) = sys.NO_DATA$
  Next
  mp$(ub%) = "0"
End Sub

' Prints the contents of the map.
Sub map.dump(mp$())
  Local lb% = Bound(mp$(), 0)
  Local ub% = Bound(mp$(), 1)
  Local off% = (ub% - lb%) \ 2
  Local sz% = Val(mp$(ub%))
  Local i%, length%
  For i% = lb% To lb% + sz% - 1 : length% = Max(length%, Len(mp$(i%))) : Next
  For i% = lb% To lb% + sz% - 1
    Print "[" Str$(i%) "] ";
    Print mp$(i%);
    Print Space$(length% - Len(mp$(i%)));
    Print " => ";
    Print mp$(i% + off%)
  Next
  Print "END"
End Sub

' Gets the value corresponding to a key, or sys.NO_DATA$ if the key is not present.
Function map.get$(mp$(), k$)
  Local lb% = Bound(mp$(), 0)
  Local ub% = Bound(mp$(), 1)
  Local off% = (ub% - lb%) \ 2
  Local sz% = Val(mp$(ub%))

  Local i% = array.bsearch%(mp$(), k$, "", lb%, sz%)
  If i% > -1 Then
    map.get$ = mp$(i% + off%)
  Else
    map.get$ = sys.NO_DATA$
  EndIf
End Function

Function map.get_key_index%(mp$(), k$)
  map.get_key_index% = array.bsearch%(mp$(), k$, "", Bound(mp$(), 0), Val(mp$(Bound(mp$(), 1))))
End Function

Function map.is_full%(map$())
  Local ub% = Bound(map$(), 1)
  '             = (ub% - lb%) \ 2 = sz%
  map.is_full% = (ub% - Bound(map$(), 0)) \ 2 = Val(map$(ub%))
End Function


' Adds a key/value pair.
Sub map.put(mp$(), k$, v$)
  Local lb% = Bound(mp$(), 0)
  Local ub% = Bound(mp$(), 1)
  Local off% = (ub% - lb%) \ 2
  Local sz% = Val(mp$(ub%))

  Local i% = array.bsearch%(mp$(), k$, "", lb%, sz%)
  If i% <> -1 Then mp$(i% + off%) = v$ : Exit Sub

  If sz% = off% Then Error "map full" : Exit Sub

  sz% = sz% + 1
  mp$(lb% + sz% - 1) = k$
  mp$(lb% + sz% + off% - 1) = v$
  mp$(ub%) = Str$(sz%)
  If sz% > 1 Then
    If k$ < mp$(lb% + sz% - 2) Then map.sort(mp$())
  EndIf
End Sub

' Removes a key/value pair.
Sub map.remove(mp$(), k$)
  Local lb% = Bound(mp$(), 0)
  Local ub% = Bound(mp$(), 1)
  Local off% = (ub% - lb%) \ 2
  Local sz% = Val(mp$(ub%))

  Local i% = array.bsearch%(mp$(), k$, "", lb%, sz%)
  If i% = -1 Then Exit Sub

  mp$(i%) = sys.NO_DATA$
  mp$(i% + off%) = sys.NO_DATA$
  If sz% > 1 Then map.sort(mp$())
  mp$(ub%) = Str$(sz% - 1)
End Sub

' Gets the size / number of entries in the map.
Function map.size%(mp$())
  map.size% = Val(mp$(Bound(mp$(), 1)))
End Function

' Sorts the map to place the keys in ascending order.
Sub map.sort(mp$())
  Local lb% = Bound(mp$(), 0)
  Local ub% = Bound(mp$(), 1)
  Local off% = (ub% - lb%) \ 2
  Local sz% = Val(mp$(ub%))
  Local idx%(map.new%(off%))

  ' Sort keys.
  Sort mp$(), idx%(), , lb%, sz%

  ' Copy values to tmp$().
  Local i%
  Local tmp$(map.new%(sz%))
  ub% = lb% + sz% - 1
  For i% = lb% To ub% : tmp$(i%) = mp$(i% + off%) : Next

  ' Copy re-ordered values back into mp$().
  For i% = lb% To ub% : mp$(i% + off%) = tmp$(idx%(i%)) : Next
End Sub
