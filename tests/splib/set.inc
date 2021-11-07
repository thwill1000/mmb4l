' Copyright (c) 2020-2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

'!remove_if NO_INCLUDE_GUARDS
On Error Skip 1 : Dim sys.VERSION$ = ""
If sys.VERSION$ = "" Then Error "'system.inc' not included"
sys.requires("array")
sys.provides("set")
If sys.err$ <> "" Then Error sys.err$
'!endif

' Gets the upper-bound that should be used to dimension an array to hold a set
' of the given capacity, irrespective of OPTION BASE.
'
' e.g. To create a string array that can be used as a set of 10 elements:
'        Dim my_set$(set.new%(10))
Function set.new%(capacity%)
  set.new% = capacity% + Mm.Info(Option Base)
End Function

' Gets the capacity of the set.
Function set.capacity%(set$())
  set.capacity% = Bound(set$(), 1) - Bound(set$(), 0)
End Function

' Initialises the set.
Sub set.init(set$())
  Local lb% = Bound(set$(), 0)
  Local ub% = Bound(set$(), 1)
  Local i%
  For i% = lb% To ub% : set$(i%) = sys.NO_DATA$ : Next
  set$(ub%) = "0"
End Sub

' Clears the set and resets its size to 0.
Sub set.clear(set$())
  Local lb% = Bound(set$(), 0)
  Local ub% = Bound(set$(), 1)
  Local sz% = Val(set$(ub%))
  Local i%
  For i% = lb% To lb% + sz% - 1 : set$(i%) = sys.NO_DATA$ : Next
  set$(ub%) = "0"
End Sub

' Prints the contents of the set.
Sub set.dump(set$())
  Local lb% = Bound(set$(), 0)
  Local ub% = Bound(set$(), 1)
  Local sz% = Val(set$(ub%))
  Local i%
  For i% = lb% To lb% + sz% - 1 : Print "[" Str$(i%) "] " set$(i%) : Next
  Print "END"
End Sub

' Gets the index of a value in the set, or -1 if not present.
Function set.get%(set$(), s$)
  set.get% = array.bsearch%(set$(), s$, "", Bound(set$(), 0), Val(set$(Bound(set$(), 1))))
End Function

Function set.is_full%(set$())
  Local ub% = Bound(set$(), 1)
  '             = (ub% - lb%) = sz%
  set.is_full% = (ub% - Bound(set$(), 0)) = Val(set$(ub%))
End Function

' Adds a value to the set.
Sub set.put(set$(), s$)
  Local lb% = Bound(set$(), 0)
  Local ub% = Bound(set$(), 1)
  Local sz% = Val(set$(ub%))

  If array.bsearch%(set$(), s$, "", lb%, sz%) <> -1 Then Exit Sub

  If sz% = ub% - lb% Then Error "set full"

  set$(lb% + sz%) = s$
  sz% = sz% + 1
  set$(ub%) = Str$(sz%)

  If sz% <= 1 Then Exit Sub
  If set$(lb% + sz% - 1) >= set$(lb% + sz% - 2) Then Exit Sub

  Sort set$(), , , lb%, sz%
End Sub

' Removes a value from the set if present.
Sub set.remove(set$(), s$)
  Local lb% = Bound(set$(), 0)
  Local ub% = Bound(set$(), 1)
  Local sz% = Val(set$(ub%))

  Local i% = array.bsearch%(set$(), s$, "", lb%, sz%)
  If i% = -1 Then Exit Sub

  set$(i%) = sys.NO_DATA$
  set$(ub%) = Str$(sz% - 1)

  Sort set$(), , , lb%, sz%
End Sub

' Gets the size of the set.
Function set.size%(set$())
  set.size% = Val(set$(Bound(set$(), 1)))
End Function
