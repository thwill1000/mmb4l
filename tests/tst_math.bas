' Copyright (c) 2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07.01
'
' Includes with permission tests derived from code by @Volhout on
' https://www.thebackshed.com

Option Explicit On
Option Default None
Option Base InStr(Mm.CmdLine$, "--base=1")  > 0

#Include "../src/splib/system.inc"
#Include "../src/splib/array.inc"
#Include "../src/splib/list.inc"
#Include "../src/splib/string.inc"
#Include "../src/splib/file.inc"
#Include "../src/splib/vt100.inc"
#Include "../src/sptest/unittest.inc"

Const BASE% = Mm.Info(Option Base)
Const RESOURCE_DIR$ = file.PROG_DIR$ + "/resources/tst_math"

Const NUM_SAMPLES% = 512
Dim sin_amp1!(array.new%(NUM_SAMPLES%))
Dim sin_amp2!(array.new%(NUM_SAMPLES%))
Dim actual!(array.new%(NUM_SAMPLES%))
Dim expected!(array.new%(NUM_SAMPLES%))
Dim delta!(array.new%(NUM_SAMPLES%))

add_test("test_cosh")
add_test("test_sinh")
add_test("test_tanh")
add_test("test_log10")
add_test("test_atan3")
add_test("test_sd")
add_test("test_set")
add_test("test_mean")
add_test("test_add")
add_test("test_sum")
add_test("test_scale")
add_test("test_max")
add_test("test_min")
add_test("test_fft_given_sine_wave")
add_test("test_fft_given_values_from_csv")
add_test("test_fft_inverse")
add_test("test_correl")
add_test("test_interpolate")
add_test("test_insert")
add_test("test_slice")
add_test("test_median")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub setup_test()
  Local i%
  For i% = BASE% to NUM_SAMPLES% + BASE% - 1
    sin_amp1!(i%) = Sin(2 * Pi * (i% - BASE%) / NUM_SAMPLES%)
    sin_amp2!(i%) = 2 * sin_amp1!(i%)
    actual!(i%) = 0
    expected!(i%) = 0
    delta!(i%) = 1e-10
  Next
End Sub

Sub teardown_test()
End Sub

Sub test_sinh()
  ' Test against Wikipedia definition of SINH
  Local x! = 3
  assert_float_equals((Exp(x!) - Exp(-x!)) / 2, Math(Sinh x!), 1e-10)
End Sub

Sub test_cosh()
  ' Test against Wikipedia definition of COSH
  Local x! = 3
  assert_float_equals((Exp(x!) + Exp(-x!)) / 2, Math(Cosh x!), 1e-10)
End Sub

Sub test_tanh()
  ' Test against Wikipedia definition of TANH
  Local x! = 3
  assert_float_equals((Exp(x!) - Exp(-x!)) / (Exp(x!) + Exp(-x!)), Math(Tanh x!), 1e-10)
End Sub

Sub test_log10()
  ' log10 of 1000 is 3
  assert_float_equals(3, Math(Log10 1000), 1e-10)
End Sub

Sub test_atan3()
  ' TODO
End Sub

Sub test_sd()
  ' Standard deviation of a sine wave is equal to the square root of 1/2 (0.707)
  assert_float_equals(Sqr(1/2), Math(Sd sin_amp1!()), 1e-10)
End Sub

Sub test_set()
  Math Set 3, actual!()

  Local i%
  For i% = BASE% To NUM_SAMPLES% + BASE% - 1 : expected!(i%) = 3 : Next
  assert_float_array_equals(expected!(), actual!(), delta!())
End Sub

Sub test_mean()
  Local a!(array.new%(NUM_SAMPLES%))
  Math Set 3, a!()

  ' The MEAN of an array of identical values should be equal to that value.
  assert_float_equals(3, Math(Mean a!()), 1e-10)
End Sub

Sub test_add()
  Local a!(array.new%(NUM_SAMPLES%))
  Math Set 3, a!()

  Math Add a!(), 3, actual!()

  Math Set 6, expected!()
  assert_float_equals(expected!(), actual!(), delta!())
End Sub

Sub test_sum()
  Local a!(array.new%(NUM_SAMPLES%))
  Math Set 3, a!()

  assert_float_equals(3 * NUM_SAMPLES%, Math(Sum a!()))
End Sub

Sub test_scale()
  Math Scale sin_amp1!(), 2, actual!()

  assert_float_array_equals(sin_amp2!(), actual!(), delta!())
End Sub

Sub test_max()
  ' A sine wave with amplitude 2 has a peak of +2, find it with MATH MAX.
  assert_float_equals(2, Math(Max sin_amp2!()), 1e-10)
End Sub

Sub test_min()
  ' A sine wave with amplitude 2 has a lowest value of -2, find it with MATH MIN.
  assert_float_equals(-2, Math(Min sin_amp2!()), 1e-10)
End Sub

Sub test_fft_given_sine_wave()
  ' To test the FFT a pure sine wave amplitude 2 is converted with 512 samples
  ' the output spectrum should contain 2 peaks (symetrical spectrum) each containing
  ' half the samples summed = 512 * 2 / 2 = 512. Other spectral components
  ' should be rounding errors. The phase part of the array has values between -pi and pi
  ' and these will be far lower than the peaks in the amplitude section of the dft!() array.
  Local dft!(array.new%(2), array.new%(NUM_SAMPLES%))

  Math Fft sin_amp2!(), dft!()

  assert_float_equals(NUM_SAMPLES%, Math(Max dft!()), 1e-10)
End Sub

' Test MATH FFT against a file of known values and results.
Sub test_fft_given_values_from_csv()
  Const size% = 1024
  Local signal!(array.new%(size%))
  Local expected_mag!(array.new%(size%))
  Local i% = 0, s$
  Open RESOURCE_DIR$ + "/fft.csv" For Input As #1
  Do While Not Eof(#1)
    Line Input #1, s$
    s$ = str.trim$(s$)
    If Left$(s$, 1) = "#" Then Continue Do ' Ignore comment lines.
    signal!(BASE% + i%) = Val(Field$(s$, 2, ","))
    expected_mag!(BASE% + i%) = Val(Field$(s$, 4, ",")) * (64/2)
    Inc i%
  Loop
  Close #1

  assert_int_equals(size%, i%)

  Dim fft!(array.new%(2), array.new%(size%))
  Math FFT signal!(), fft!()

  Local actual!, x!, y!
  For i% = 0 To size% - 1
    x! = fft!(BASE%, BASE% + i%)
    y! = fft!(BASE% + 1, BASE% + i%)
    actual! = Sqr(x! * x! + y! * y!)
    assert_float_equals(expected_mag!(BASE% + i%), actual!, 1e-6)
  Next

End Sub

Sub test_fft_inverse()
  ' This test uses the output of FFT and converts it back to a signal array actual!().
  ' The array should be a copy of actual!().
  Local dft!(array.new%(2), array.new%(NUM_SAMPLES%))
  Math Fft sin_amp2!(), dft!()

  Math Fft Inverse dft!(), actual!())

  assert_float_equals(sin_amp2!(), actual!(), 1e-10)
End Sub

Sub test_correl()
  Local dft!(array.new%(2), array.new%(NUM_SAMPLES%))
  Math Fft sin_amp2!(), dft!()
  Local a!(array.new%(NUM_SAMPLES%))
  Math Fft Inverse dft!(), a!()

  ' Check how much sin_amp2!() and a!() are alike through correlation.
  assert_float_equals(1, Math(Correl sin_amp2!(), a!()), 1e-10)

  ' Check how much sin_amp2!() and a!() are alike through correlation
  ' when a fault is introduced.
  a!(BASE% + 123)=10
  assert_float_equals(0.9711498696, Math(Correl sin_amp2!(), a!()), 1e-10)
End Sub

Sub test_interpolate()
  Math Interpolate sin_amp1!(), sin_amp2!(), 3, actual!()

  Local i%
  For i% = BASE% To NUM_SAMPLES% + BASE% - 1
    expected!(i%) = (sin_amp2!(i%) - sin_amp1!(i%)) * 3 + sin_amp1!(i%)
  Next
  assert_float_equals(expected!(), actual!(), 1e-10)
End Sub

Sub test_insert()
  Local a!(array.new%(2), array.new%(NUM_SAMPLES%))

  Math Insert a!(), BASE%, ,sin_amp2!()

  Local i%
  For i% = BASE% To NUM_SAMPLES% + BASE% - 1
    assert_float_equals(sin_amp2!(i%), a!(BASE%, i%), 1e-10)
    assert_float_equals(0.0, a!(BASE% + 1, i%), 1e-10)
  Next
End Sub

Sub test_slice()
  Local a!(array.new%(2), array.new%(NUM_SAMPLES%))
  Math Fft sin_amp2!(), a!()

  Math Slice a!(), BASE%, , actual!()

  Local i%
  For i% = BASE% To NUM_SAMPLES% + BASE% - 1
    expected!(i%) = a!(BASE%, i%)
  Next
  assert_float_equals(expected!(), actual!(), delta!())
End Sub

Sub test_median()
  Local a!(array.new%(NUM_SAMPLES%))
  a!(BASE% + 123) = 1

  ' All samples are 0 except one that is 1.
  ' The mean value will be 1 / NUM_SAMPLES%.
  ' The median value will be 0 (most samples are 0).
  assert_float_equals(0.0, Math(Median a!()), 1e-10)
End Sub
