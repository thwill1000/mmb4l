' Copyright (c) 2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

'!remove_if NO_INCLUDE_GUARDS
On Error Skip 1 : Dim sys.VERSION$ = ""
If sys.VERSION$ = "" Then Error "'system.inc' not included"
sys.requires("map", "string")
sys.provides("inifile")
If sys.err$ <> "" Then Error sys.err$
'!endif

' Reads key/value pairs from a .INI file into a map.
'
' @param       fd%    file descriptor of open file to read from.
' @param[out]  mp$()  map to add key/value pairs to, this is not cleared first.
' @param[out]  num%   the number of key/value pairs read.
' @return             1 on success, 0 on failure, see sys.err$ for details of failure.
' @errors             "too many values" if the map overflows;
'                     num% will contain the TOTAL number of key/value pairs in the file.
'                     "missing key"     if a value appears without a key.
'                     "missing value"   if a key appears without a value.
Function inifile.read%(fd%, mp$(), num%)
  Local lc%, s$, key$, value$, p%
  Const free% = map.capacity%(mp$()) - map.size%(mp$())

  ' TODO: support quoted string values.
  ' TODO: support escapes for = and ; characters.
  num% = 0
  Do While Not Eof(fd%)
    Line Input #fd%, s$
    If free% >= 0 Then Inc lc%
    p% = InStr(s$, ";")
    If p% > 0 Then s$ = Left$(s$, p% - 1)
    s$ = str.trim$(s$)
    If s$ = "" Then Continue Do
    p% = InStr(s$, "=")
    key$ = Choice(p% = 0, s$, str.trim$(Left$(s$, p% - 1)))
    value$ = Choice(p% = 0, "", str.trim$(Mid$(s$, p% + 1)))

    If key$   = "" Then sys.err$ = "missing key"
    If value$ = "" And p% = 0 Then sys.err$ = "missing value"
    If sys.err$ <> "" Then Exit Do

    Inc num%
    If num% <= free% Then map.put(mp$(), key$, value$)
  Loop

  If num% > free% Then sys.err$ = "too many values"

  If sys.err$ <> "" Then Cat sys.err$, " (line " + Str$(lc%) + ")"

  inifile.read% = (sys.err$ = "")
End Function

' Writes key/value pairs from a map into a .INI file.
'
' @param       fd%    file descriptor of open file to write to.
' @param       mp$()  map of key/value pairs to write.
' @param[out]  num%   the number of key/value pairs written.
' @return             1 on success, 0 on failure, see sys.err$ for details of failure.
Function inifile.write%(fd%, mp$(), num%)
  Local i%
  Local lb% = Bound(mp$(), 0)
  Local ub% = map.size%(mp$()) - 1 + lb%

  num% = 0
  For i% = lb% To ub%
    Print #fd%, mp$(i%) " = " map.get$(mp$(), mp$(i%))
    Inc num%
  Next

  inifile.write% = 1
End Function
