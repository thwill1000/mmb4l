' Copyright (c) 2020-2022 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMBasic 5.07

' LED matrix manipulation library.

' Makes use of a global matrix(8) created by calling matrix_init().
' Elements 0-7 are the 8-bit pattern for the matrix columns.

Const matrix.WRITE_SPI% = 0
Const matrix.WRITE_CONSOLE% = 1

Dim matrix.console_buf$
Dim matrix.console_x% = 0
Dim matrix.console_y% = 6
Dim matrix.redraw% = 0

Sub matrix_init()
  Local i%
  For i% = 1 To 8 : SetPin i%, dout : Next
  For i% = 11 To 18 : SetPin i%, dout : Next
  matrix_nc% = 8
  Dim matrix%(matrix_nc% - 1) ' All elements initalised to 0
  matrix_r% = 0
End Sub

Sub matrix_clear()
  Local i%
  For i% = 0 To matrix_nc% - 1 : matrix%(i%) = 0 : Next
  matrix.redraw% = 1
End Sub

Sub matrix_fill()
  Local i%
  For i% = 0 To matrix_nc% - 1 : matrix%(i%) = &hFF : Next
  matrix.redraw% = 1
End Sub

Function matrix_get%(x%, y%)
  Local x2%, y2%
  x2% = x% + (y% \ 8) * matrix_w%
  y2% = y% Mod 8
  matrix_get% = (matrix%(x2%) And 2^y2%) > 0
End Function

Sub matrix_set(x%, y%, z%)
  Local x2%, y2%
  x2% = x% + (y% \ 8) * matrix_w%
  y2% = y% Mod 8
  matrix%(x2%) = matrix%(x2%) And (&hFF Xor 2^y2%) Or (z% * 2^y2%)
End Sub

Sub matrix_left()
  Local i%
  For i% = 0 To matrix_nc% - 2 : matrix%(i%) = matrix%(i% + 1) : Next
  matrix%(matrix_nc% - 1) = 0
  matrix.redraw% = 1
End Sub

Sub matrix.wipe()
  Local i% = 0, set%, x%, y%
  Do While (i% < matrix_w% \ 2) And (i% < matrix_h% \ 2)
    For set% = 1 To 0 Step -1
      For x% = i% To matrix_w% - 1 - i%
        matrix_set(x%, i%, set%)
      Next
      For y% = 1 + i% To matrix_h% - 2 - i%
        matrix_set(i%, y%, set%)
        matrix_set(matrix_w% - 1 - i%, y%, set%)
      Next
      For x% = i% To matrix_w% - 1 - i%
        matrix_set(x%, matrix_h% - 1 - i%, set%)
      Next
      matrix.redraw% = 1
      If set% Then Pause 100
    Next set%
    Inc i%
  Loop
End Sub

' Updates scanline/row 'matrix_r'.
'
' Assumes that a HIGH is required on the row & column to light the LED.
' If a LOW is required on the row then use the commented out values in the
' calls to Port().
'
' This routine needs to be fast (it is called every 2ms) hence the
' unrolled loop used to calculate 'x'.
'
' A much faster implementation is possible if matrix data is stored by
' rows instead of columns, i.e.
'
'   Port(1, 8, 11, 8) = matrix(matrix_r) Or 2^(matrix_r + 8)
Sub matrix_draw()
  Local m, x
  m = 2^matrix_r
  x = ((matrix(0) And m) > 0) Or 2 * ((matrix(1) And m) > 0)
  x = x Or 4  * ((matrix(2) And m) > 0) Or 8   * ((matrix(3) And m) > 0)
  x = x Or 16 * ((matrix(4) And m) > 0) Or 32  * ((matrix(5) And m) > 0)
  x = x Or 64 * ((matrix(6) And m) > 0) Or 128 * ((matrix(7) And m) > 0)
  Port(1, 8, 11, 8) = 0 ' &hFF00
  Port(1, 8, 11, 8) = x Or 2^(matrix_r + 8) ' &hFF00 - 2^(matrix_r + 8)
  matrix_r = (matrix_r + 1) Mod 8
End Sub

Sub m7219_init(w%, h%)
  Local i%
  If w% Mod 8 <> 0 Then Error "Invalid number of columns: " + Str$(w%)
  If w% Mod 8 <> 0 Then Error "Invalid number of rows: " + Str$(h%)
  Dim matrix_nc% = w% * (h% \ 8)
  Dim matrix_h% = h%
  Dim matrix_w% = w%
  Dim matrix%(matrix_nc% - 1) ' all elements initialised to 0

  Dim m7219_rx% = 1
  Dim m7219_tx% = 9
  Dim m7219_cs% = 4
  Dim m7219_clk% = 10

  If matrix.WRITE_SPI% Then
    SetPin m7219_rx%, din
    SetPin m7219_tx%, dout
    SetPin m7219_cs%, dout
    SetPin m7219_clk%, dout

    Pin(m7219_clk%) = 1 ' Active low - so initially set high

    For i% = 0 To matrix_nc% \ 8 - 1
      m7219_write(&h0900, i%) ' BCD DECODE ALL DIGITS
      m7219_write(&h0A00, i%) ' BRIGHTNESS (0-15)
      m7219_write(&h0B07, i%) ' 8 COLUMNS
      m7219_write(&h0F00, i%) ' TESTS OFF
      m7219_write(&h0C01, i%) ' OPERATING MODE (i.e. NOT SHUTDOWN MODE)
    Next
  EndIf

  If matrix.WRITE_CONSOLE% Then
    Console Foreground Grey
    Local y% = matrix.console_y%
    Console SetCursor matrix.console_x%, y%
    Print "+" String$(matrix_w%, "-") "+"
    Inc y%
    For i% = 1 To matrix_h%
      Console SetCursor matrix.console_x%, y%
      Print "|" String$(matrix_w%, " ") "|"
      Inc y%
    Next
    Console SetCursor matrix.console_x%, y%
    Print "+" String$(matrix_w%, "-") "+"

    For i% = 0 To 7
      Cat matrix.console_buf$, "        " + Chr$(27) + "[1B" + Chr$(27) + "[8D"
    Next
  EndIf

  m7219_draw() ' Clears the matrix - is this necessary?
End Sub

Sub m7219_draw()
  If Not matrix.redraw% Then Exit Sub
  matrix.redraw% = 0
  Local c%
  For c% = 0 To matrix_nc% - 1
    m7219_write((1 + c% Mod 8) * 256 + matrix%(c%), c% \ 8)
  Next
End Sub

Sub m7219_write(x%, idx%)
  If matrix.WRITE_SPI% Then
    Local i%, null%
    Pin(m7219_cs%) = 0
    For i% = matrix_nc% \ 8 - 1 To 0 Step -1
      null% = SPI(m7219_rx%, m7219_tx%, m7219_clk%, x% * (i% = idx%), H, 3, 16)
    Next
    Pin(m7219_cs%) = 1
  EndIf

  If Not matrix.WRITE_CONSOLE% Then Exit Sub

  Local c% = (idx% * 8) + (x% \ 256) - 1
  Local c8% = c% Mod 8
  Local ad% = Peek(VarAddr matrix.console_buf$) + 1 + c8%
  Poke Byte ad%,       Choice(x% And 1,   64, 32)
  Poke Byte ad% +  16, Choice(x% And 2,   64, 32)
  Poke Byte ad% +  32, Choice(x% And 4,   64, 32)
  Poke Byte ad% +  48, Choice(x% And 8,   64, 32)
  Poke Byte ad% +  64, Choice(x% And 16,  64, 32)
  Poke Byte ad% +  80, Choice(x% And 32,  64, 32)
  Poke Byte ad% +  96, Choice(x% And 64,  64, 32)
  Poke Byte ad% + 112, Choice(x% And 128, 64, 32)

  If c8% <> 7 Then Exit Sub

  ' Only once we have processed the 8th column of each 8x8 block do we
  ' write to the console.
  Console Foreground Bright Red
  Console SetCursor (8 * ((c% Mod matrix_w%) \ 8)) + 1 + matrix.console_x%, (8 * (c% \ matrix_w%)) + 1 + matrix.console_y%
  Print matrix.console_buf$
End Sub

Sub digits_init()
  Local i, x
  Dim digits(9, 7) ' 10 digits, 8 columns per character

  ' Each DATA line represents dot-patterns for the 8-columns
  ' (left to right) msb=bottom dot, lsb=top dot

  Data &h7E, &hFF, &hFF, &hC3, &hC3, &hFF, &hFF, &h7E ' 0
  Data &h00, &h00, &h04, &h06, &hFF, &hFF, &hFF, &h00 ' 1
  Data &hE6, &hE7, &hF7, &hD3, &hDB, &hCF, &hCF, &hC6 ' 2
  Data &hDB, &hDB, &hDB, &hDB, &hDB, &hFF, &hFF, &h66 ' 3
  Data &h1F, &h1F, &h1F, &h18, &hFF, &hFF, &hFF, &h18 ' 4
  Data &h5F, &hDF, &hDF, &hDB, &hDB, &hFB, &hFB, &h73 ' 5
  Data &h7E, &hFF, &hFF, &hDB, &hDB, &hFB, &hFB, &h72 ' 6
  Data &h03, &h03, &hE3, &hF3, &hFB, &h1F, &h0F, &h07 ' 7
  Data &h66, &hFF, &hFF, &hDB, &hDB, &hFF, &hFF, &h66 ' 8
  Data &h4E, &hDF, &hDF, &hDB, &hDB, &hFF, &hFF, &h7E ' 9

  For i = 0 To 9
    For x = 0 To 7
      Read digits(i, x)
    Next x
  Next i
End Sub
