' Copyright (c) 2020-2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

'!remove_if NO_INCLUDE_GUARDS
On Error Skip 1 : Dim sys.VERSION$ = ""
If sys.VERSION$ = "" Then Error "'system.inc' not included"
sys.provides("list")
If sys.err$ <> "" Then Error sys.err$
'!endif

' Gets the upper-bound that should be used to dimension an array to hold a list
' of the given capacity, irrespective of OPTION BASE.
'
' e.g. To create a string array that can be used as a list of 10 elements:
'        Dim my_list$(list.new%(10))
Function list.new%(capacity%)
  list.new% = capacity% + Mm.Info(Option Base)
End Function

' Gets the capacity of the list.
Function list.capacity%(list$())
  list.capacity% = Bound(list$(), 1) - Bound(list$(), 0)
End Function

' Initialises the list.
Sub list.init(lst$())
  Local i%
  For i% = Bound(lst$(), 0) To Bound(lst$(), 1)
    lst$(i%) = sys.NO_DATA$
  Next
  lst$(Bound(lst$(), 1)) = "0"
End Sub

' Appends an element to the end of the list.
Sub list.add(lst$(), s$)
  Local lb% = Bound(lst$(), 0)
  Local ub% = Bound(lst$(), 1)
  Local sz% = Val(lst$(ub%))
  ' TODO: report error if adding to a full list.
  lst$(lb% + sz%) = s$
  lst$(ub%) = Str$(sz% + 1)
End Sub

' Clears the list and resets its size to 0.
Sub list.clear(lst$())
  Local lb% = Bound(lst$(), 0)
  Local ub% = Bound(lst$(), 1)
  Local sz% = Val(lst$(ub%))
  Local i%
  For i% = lb% To lb% + sz% - 1
    lst$(i%) = sys.NO_DATA$
  Next
  lst$(ub%) = "0"
End Sub

' Prints the contents of the list.
Sub list.dump(lst$())
  Local lb% = Bound(lst$(), 0)
  Local ub% = Bound(lst$(), 1)
  Local sz% = Val(lst$(ub%))
  Local i%
  For i% = lb% To lb% + sz% - 1
    Print "[" Str$(i%) "] " lst$(i%)
  Next
  Print "END"
End Sub

' Gets a list element with bounds checking.
' To get a list element without bounds checking just do s$ = lst$(index%) directly.
Function list.get$(lst$(), index%)
  Local lb% = Bound(lst$(), 0)
  Local ub% = Bound(lst$(), 1)
  Local sz% = Val(lst$(ub%))
  If index% >= lb% + sz% Then Error "index out of bounds: " + Str$(index%) : Exit Function
  list.get$ = lst$(index%)
End Function

' Inserts an element into the list.
Sub list.insert(lst$(), index%, s$)
  Local lb% = Bound(lst$(), 0)
  Local ub% = Bound(lst$(), 1)
  Local sz% = Val(lst$(ub%))
  Local i%
  If index% >= lb% + sz% + 1 Then Error "index out of bounds: " + Str$(index%) : Exit Function
  For i% = lb% + sz% To lb% + index% + 1 Step -1
    lst$(i%) = lst$(i% - 1)
  Next
  lst$(i%) = s$
  lst$(ub%) = Str$(sz% + 1)
End Sub

Function list.is_full%(lst$())
  Local ub% = Bound(lst$(), 1)
  '             = (ub% - lb%) = sz%
  list.is_full% = (ub% - Bound(lst$(), 0)) = Val(lst$(ub%))
End Function

' Returns the element at the end of the list.
' If the list is empty then returns sys.NO_DATA$
Function list.peek$(lst$())
  Local lb% = Bound(lst$(), 0)
  Local ub% = Bound(lst$(), 1)
  Local sz% = Val(lst$(ub%))
  If sz% > 0 Then list.peek$ = lst$(lb% + sz% - 1) Else list.peek$ = sys.NO_DATA$
End Function

' Removes and returns the element at the end of the list.
' If the list is empty then returns sys.NO_DATA$
Function list.pop$(lst$())
  Local lb% = Bound(lst$(), 0)
  Local ub% = Bound(lst$(), 1)
  Local sz% = Val(lst$(ub%))
  If sz% > 0 Then
    list.pop$ = lst$(lb% + sz% - 1)
    lst$(ub%) = Str$(sz% - 1)
  Else
    list.pop$ = sys.NO_DATA$
  EndIf
End Function

' Synonym for add().
Sub list.push(lst$(), s$)
  Local lb% = Bound(lst$(), 0)
  Local ub% = Bound(lst$(), 1)
  Local sz% = Val(lst$(ub%))
  lst$(lb% + sz%) = s$
  lst$(ub%) = Str$(sz% + 1)
End Sub

' Removes an element from the list.
Sub list.remove(lst$(), index%)
  Local lb% = Bound(lst$(), 0)
  Local ub% = Bound(lst$(), 1)
  Local sz% = Val(lst$(ub%))
  Local i%
  If index% >= lb% + sz% Then Error "index out of bounds: " + Str$(index%) : Exit Sub
  For i% = index% To lb% + sz% - 2
    lst$(i%) = lst$(i% + 1)
  Next
  lst$(i%) = sys.NO_DATA$
  lst$(ub%) = Str$(sz% - 1)
End Sub

' Sets a list element with bounds checking.
' To set a list element without bounds checking just do lst$(index%) = s$ directly.
Sub list.set(lst$(), index%, s$)
  Local lb% = Bound(lst$(), 0)
  Local ub% = Bound(lst$(), 1)
  Local sz% = Val(lst$(ub%))
  If index% >= lb% + sz% Then Error "index out of bounds: " + Str$(index%) : Exit Sub
  lst$(index%) = s$
End Sub

' Sorts the list.
Sub list.sort(lst$())
  Local lb% = Bound(lst$(), 0)
  Local ub% = Bound(lst$(), 1)
  Local sz% = Val(lst$(ub%))
  Sort lst$(), , , lb%, sz%
End Sub

' Gets the size of the list.
Function list.size%(list$())
  list.size% = Val(list$(Bound(list$(), 1)))
End Function
