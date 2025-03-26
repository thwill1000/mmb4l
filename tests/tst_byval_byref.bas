' Copyright (c) 2025 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>

Option Explicit On

Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1")  > 0

#Include "../sptools/src/splib/system.inc"
#Include "../sptools/src/splib/array.inc"
#Include "../sptools/src/splib/list.inc"
#Include "../sptools/src/splib/string.inc"
#Include "../sptools/src/splib/file.inc"
#Include "../sptools/src/splib/vt100.inc"
#Include "../sptools/src/sptest/unittest.inc"

Const BASE% = Mm.Info(Option Base)

add_test("test_int_byval")
add_test("test_int_byref")
add_test("test_float_byval")
add_test("test_float_byref")
add_test("test_string_byval")
add_test("test_string_byref")
add_test("test_array_byval")
add_test("test_array_byref")
add_test("test_byval_given_mismatched_type")
add_test("test_byref_given_mismatched_type")
add_test("test_byval_given_not_a_var")
add_test("test_byref_given_not_a_var")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub test_int_byval()
  Local i% = 42

  assert_int_equals(43, int_byval%(i%))
  assert_int_equals(42, i%)
End Sub

Function int_byval%(ByVal i As Integer)
  Inc i
  int_byval% = i
End Function

Sub test_int_byref()
  Local i% = 42

  assert_int_equals(43, int_byref%(i%))
  assert_int_equals(43, i%)
End Sub

Function int_byref%(ByRef i As Integer)
  Inc i
  int_byref% = i
End Function

Sub test_float_byval()
  Local f! = 3.142

  assert_float_equals(3.242, float_byval!(f!))
  assert_float_equals(3.142, f!)
End Sub

Function float_byval!(ByVal f As Float)
  Inc f, 0.1
  float_byval! = f
End Function

Sub test_float_byref()
  Local f! = 3.142

  assert_float_equals(3.242, float_byref!(f!))
  assert_float_equals(3.242, f!)
End Sub

Function float_byref!(ByRef f As Float)
  Inc f, 0.1
  float_byref! = f
End Function

Sub test_string_byval()
  Local s$ = "Hello"

  assert_string_equals("Hello World", string_byval$(s$))
  assert_string_equals("Hello", s$)
End Sub

Function string_byval$(ByVal s As String)
  Cat s, " World"
  string_byval$ = s
End Function

Sub test_string_byref()
  Local s$ = "Hello"

  assert_string_equals("Hello World", string_byref$(s$))
  assert_string_equals("Hello World", s$)
End Sub

Function string_byref$(ByRef s As String)
  Cat s, " World"
  string_byref$ = s
End Function

Sub test_array_byval()
  Local a%(BASE% + 3) = (2, 4, 6, 8)

  On Error Ignore
  array_byval(a%())
  assert_raw_error("Dimensions")
  On Error Abort
End Sub

Sub array_byval(ByVal a() As Integer)
  Local i%
  For i% = Bound(a(), 0) To Bound(a(), 1)
    Inc a(i%)
  Next
End Sub

Sub test_array_byref()
  Local a%(BASE% + 3) = (2, 4, 6, 8)

  array_byref(a%())
  Local expected%(BASE% + 3) = (3, 5, 7, 9)
  assert_int_array_equals(expected%(), a%())
End Sub

Sub array_byref(ByRef a() As Integer)
  Local i%
  For i% = Bound(a(), 0) To Bound(a(), 1)
    Inc a(i%)
  Next
End Sub

Sub test_byval_given_mismatched_type()
  Local f! = 3.142, i% = 42, s$ = "Hello", tmp$

  assert_int_equals(4, int_byval%(f!))
  assert_float_equals(42.1, float_byval!(i%))

  On Error Ignore
  tmp$ = string_byval$(i%)
  assert_raw_error("Incompatible type: I%")
  On Error Abort

  On Error Ignore
  tmp$ = string_byval$(f!)
  assert_raw_error("Incompatible type: F!")
  On Error Abort

  On Error Ignore
  i% = int_byval%(s$)
  assert_raw_error("Incompatible type: S$")
  On Error Abort

  On Error Ignore
  f! = float_byval!(s$)
  assert_raw_error("Incompatible type: S$")
  On Error Abort
End Sub

Sub test_byref_given_mismatched_type()
  Local f! = 3.142, i% = 42, s$ = "Hello", tmp$

  On Error Ignore
  i% = int_byref%(f!)
  assert_raw_error("BYREF requires same types: F!")
  On Error Abort

  On Error Ignore
  f! = float_byref!(i%)
  assert_raw_error("BYREF requires same types: I%")
  On Error Abort

  On Error Ignore
  tmp$ = string_byref$(i%)
  assert_raw_error("BYREF requires same types: I%")
  On Error Abort

  On Error Ignore
  tmp$ = string_byref$(f!)
  assert_raw_error("BYREF requires same types: F!")
  On Error Abort

  On Error Ignore
  i% = int_byref%(s$)
  assert_raw_error("BYREF requires same types: S$")
  On Error Abort

  On Error Ignore
  f! = float_byref!(s$)
  assert_raw_error("BYREF requires same types: S$")
  On Error Abort
End Sub

Sub test_byval_given_not_a_var()
  assert_int_equals(5, int_byval%(4))
  assert_float_equals(42.1, float_byval!(42.0))
  assert_string_equals("Hello World", string_byval$("Hello"))
End Sub

Sub test_byref_given_not_a_var()
  On Error Ignore
  Local i% = int_byref%(4)
  assert_raw_error("Variable required for BYREF")
  On Error Abort

  On Error Ignore
  Local f! = float_byref!(42.0)
  assert_raw_error("Variable required for BYREF")
  On Error Abort

  On Error Ignore
  Local s$ = string_byref$("Hello")
  assert_raw_error("Variable required for BYREF")
  On Error Abort
End Sub
