' Copyright (c) 2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

'!remove_if NO_INCLUDE_GUARDS
On Error Skip 1 : Dim sys.VERSION$ = ""
If sys.VERSION$ = "" Then Error "'system.inc' not included"
sys.provides("txtwm")
If sys.err$ <> "" Then Error sys.err$
'!endif

If Mm.Info(Option Base) <> 0 Then Error "expects OPTION BASE 0"

Const twm.BLACK%   = 0
Const twm.RED%     = 1
Const twm.GREEN%   = 2
Const twm.YELLOW%  = 3
Const twm.BLUE%    = 4
Const twm.MAGENTA% = 5
Const twm.CYAN%    = 6
Const twm.WHITE%   = 7

' All these arrays are created by twm.init() and destroyed by twm.free().
' Dim twm.vt$(255)  ' map of attribute values to VT100 control-codes
' Dim twm.fg%(255)  ' map of attribute values to VGA foreground colours
' Dim twm.bg%(255)  ' map of attribute values to VGA background colours
' Dim twm.data%()   ' large block of memory for storing window state.
' Dim twm.ptr%()    ' pointers into twm.data%() such that twm.ptr%(i) is the
                    ' memory address where the data for window 'i' starts.
' Dim twm.c2b%(255) ' map from ASCII chars to box-drawing data.
' Dim twm.b2c%(15)  ' map from box-drawing data to ASCII chars.

Dim twm.max_num%    ' max number of windows allowed
Dim twm.num%        ' current number of windows created
Dim twm.fw%         ' font width
Dim twm.fh%         ' font height
Dim twm.last_at%    ' as an optimisation we keep a record of the last attribute
                    ' / vt100 control-code that the txtwm library wrote to the
                    ' serial output so that we only write a new one when
                    ' necessary. WARNING! if code outside of txtwm is also
                    ' writing vt100 control-codes then this may get out of
                    ' synch. It can be cleared by setting it to -1 or calling
                    ' twm.switch().
Dim twm.cursor_enabled%
Dim twm.cursor_locked%

' For the currently selected window:
Dim twm.id% ' id
Dim twm.a%  ' x-coordinate of left hand side
Dim twm.b%  ' y-coordinate of top
Dim twm.w%  ' width
Dim twm.h%  ' height
Dim twm.x%  ' x cursor-position
Dim twm.y%  ' y cursor-position
Dim twm.at% ' attributes to use for printing
Dim twm.pc% ' pointer to the character data
Dim twm.pa% ' pointer to the attribute data

' Initialises the 'txtwm' library.
'
' @param  max_num%  the maximum number of windows to support.
' @param  mem_sz%   number of bytes of memory to allocate for window data.
'                   Each window requires 7 + 2 * (width * height) bytes.
Sub twm.init(max_num%, mem_sz%)
  If twm.max_num% > 0 Then Error "'txtwm' already initialised"
  If max_num% < 1 Or max_num% > 10 Then Error "invalid max number of windows: " + Str$(max_num%)
  If mem_sz% < 100 Then Error "invalid memory size: " + Str$(mem_sz%)

  twm.max_num% = max_num%
  twm.id%      = -1
  twm.last_at% = -1
  twm.fw%      = Mm.Info(FontWidth)
  twm.fh%      = Mm.Info(FontHeight)

  ' Allocate data buffer.
  Dim twm.data%(mem_sz% - 1)

  ' Allocate array of pointers (one for each window) into the data buffer.
  ' Because MMBasic can't have 1 element arrays we always allocate space for at least 2 windows.
  Dim twm.ptr%(Choice(max_num% = 1, 1, max_num% - 1))

  twm.init_serial_attrs()
  twm.init_screen_attrs()
  twm.init_box_chars()
  twm.enable_cursor(0)
End Sub

Sub twm.init_serial_attrs()
  Local i%, vt$

  Dim twm.vt$(255) Length 20

  For i% = 0 To 255
    ' Clear attributes
    vt$ = Chr$(27) + "[0m"

    ' Foreground (with/out bold)
    Cat vt$, Chr$(27) + "[" + Choice(i% And &b01000000, "1;3", "3") + Str$(i% And &b00000111) + "m"

    ' Background
    Cat vt$, Chr$(27) + "[4" + Str$((i% And &b00111000) >> 3) + "m"

    ' Reverse video
    If i% And &b10000000 Then Cat vt$, Chr$(27) + "[7m"

    twm.vt$(i%) = vt$
  Next

End Sub

Sub twm.init_screen_attrs()
  Local bg%, fg%, i%

  Dim twm.fg%(255)
  Dim twm.bg%(255)

  For i% = 0 To 255
    ' Foreground
    Select Case i% And &b00000111
      Case twm.BLACK%   : fg% = RGB(Black)
      Case twm.RED%     : fg% = RGB(Red)
      Case twm.GREEN%   : fg% = RGB(Green)
      Case twm.YELLOW%  : fg% = RGB(Yellow)
      Case twm.BLUE%    : fg% = RGB(Blue)
      Case twm.MAGENTA% : fg% = RGB(Magenta)
      Case twm.CYAN%    : fg% = RGB(Cyan)
      Case twm.WHITE%   : fg% = RGB(White)
      Case Else Error
    End Select

    ' Bold/Bright text
    If i% And &b01000000 Then
      If fg% <> 0 Then fg% = fg% Or &h404040
    EndIf

    ' Background
    Select Case (i% And &b00111000) >> 3
      Case twm.BLACK%   : bg% = RGB(Black)
      Case twm.RED%     : bg% = RGB(Red)
      Case twm.GREEN%   : bg% = RGB(Green)
      Case twm.YELLOW%  : bg% = RGB(Yellow)
      Case twm.BLUE%    : bg% = RGB(Blue)
      Case twm.MAGENTA% : bg% = RGB(Magenta)
      Case twm.CYAN%    : bg% = RGB(Cyan)
      Case twm.WHITE%   : bg% = RGB(White)
      Case Else Error
    End Select

    If i% And &b10000000 Then
      ' Reverse video
      twm.bg%(i%) = fg%
      twm.fg%(i%) = bg%
    Else
      ' Normal video
      twm.bg%(i%) = bg%
      twm.fg%(i%) = fg%
    EndIf
  Next

End Sub

Sub twm.init_box_chars()
  Dim twm.c2b%(255)
  twm.c2b%(&hB9) = &b1101
  twm.c2b%(&hBA) = &b0101
  twm.c2b%(&hBB) = &b1100
  twm.c2b%(&hBC) = &b1001
  twm.c2b%(&hC8) = &b0011
  twm.c2b%(&hC9) = &b0110
  twm.c2b%(&hCA) = &b1011
  twm.c2b%(&hCB) = &b1110
  twm.c2b%(&hCC) = &b0111
  twm.c2b%(&hCD) = &b1010
  twm.c2b%(&hCE) = &b1111

  Dim twm.b2c%(15)
  twm.b2c%(&b1101) = &hB9
  twm.b2c%(&b0101) = &hBA
  twm.b2c%(&b1100) = &hBB
  twm.b2c%(&b1001) = &hBC
  twm.b2c%(&b0011) = &hC8
  twm.b2c%(&b0110) = &hC9
  twm.b2c%(&b1011) = &hCA
  twm.b2c%(&b1110) = &hCB
  twm.b2c%(&b0111) = &hCC
  twm.b2c%(&b1010) = &hCD
  twm.b2c%(&b1111) = &hCE
End Sub

Sub twm.free()
  If twm.max_num% = 0 Then Exit Sub
  Erase twm.data%()
  Erase twm.ptr%()
  Erase twm.c2b%()
  Erase twm.b2c%()
  Erase twm.vt$()
  Erase twm.fg%()
  Erase twm.bg%()
  twm.max_num% = 0
  twm.num%     = 0
  twm.fw%      = 0
  twm.fh%      = 0
  twm.id%      = 0
  twm.a%       = 0
  twm.b%       = 0
  twm.w%       = 0
  twm.h%       = 0
  twm.x%       = 0
  twm.y%       = 0
  twm.at%      = 0
  twm.pc%      = 0
  twm.pa%      = 0
  twm.last_at% = 0
End Sub

Function twm.new_win%(x%, y%, w%, h%)
  If twm.num% > Bound(twm.ptr%(), 1) Then Error "maximum number of windows reached: " + Str$(twm.num%)

  Local ptr%

  ' Store pointer into the data buffer for the new window.
  If twm.num% = 0 Then
    ptr% = Peek(VarAddr twm.data%())
  Else
    ptr% = twm.ptr%(twm.num% - 1)
    ptr% = ptr% + 7 + Peek(Byte ptr% + 2) * Peek(Byte ptr% + 3) * 2
  EndIf
  twm.ptr%(twm.num%) = ptr%

  Local reqd% = ptr% + 7 + w% * h% * 2 - twm.ptr%(0)
  Local alloc% = Bound(twm.data%(), 1) + 1
  If reqd% > alloc% Then
    Error "out of txtwm memory: " + Str$(alloc%) + " bytes allocated, " + Str$(reqd%) + " required"
  EndIf

  ' Note we add 1 to window x & y positions to account for VT100 using (1, 1) as origin.
  Poke Byte ptr% + 0, x% + 1                          ' window x-position
  Poke Byte ptr% + 1, y% + 1                          ' window y-position
  Poke Byte ptr% + 2, w%                              ' window width
  Poke Byte ptr% + 3, h%                              ' window height
  Poke Byte ptr% + 4, 0                               ' x-cursor position
  Poke Byte ptr% + 5, 0                               ' y-cursor position
  Poke Byte ptr% + 6, twm.WHITE%                      ' current attribute value
  Memory Set ptr% + 7, 32, w% * h%                    ' character buffer
  Memory Set ptr% + 7 + w% * h%, twm.WHITE%, w% * h%  ' attribute buffer

  twm.new_win% = twm.num%
  Inc twm.num%
End Function

Sub twm.switch(id%)
  ' Calling twm.switch() always resets this even if no actually window switch
  ' is required.
  twm.last_at% = -1

  If twm.id% = id% Then Exit Sub

  twm.lock_vga_cursor(1)

  Local ptr%

  If twm.id% > -1 Then
    ptr% = twm.ptr%(twm.id%)
    Poke Byte ptr% + 4, twm.x%
    Poke Byte ptr% + 5, twm.y%
    Poke Byte ptr% + 6, twm.at%
  EndIf

  twm.id% = id%
  ptr%    = twm.ptr%(twm.id%)
  twm.a%  = Peek(Byte ptr% + 0)
  twm.b%  = Peek(Byte ptr% + 1)
  twm.w%  = Peek(Byte ptr% + 2)
  twm.h%  = Peek(Byte ptr% + 3)
  twm.x%  = Peek(Byte ptr% + 4)
  twm.y%  = Peek(Byte ptr% + 5)
  twm.at% = Peek(Byte ptr% + 6)
  twm.pc% = ptr% + 7
  twm.pa% = twm.pc% + twm.w% * twm.h%

  twm.lock_vga_cursor(0)
End Sub

Sub twm.foreground(col%)
  twm.at% = (twm.at% And &b11111000) Or col%
End Sub

Sub twm.background(col%)
  twm.at% = (twm.at% And &b11000111) Or (col% << 3)
End Sub

Sub twm.bold(z%)
  twm.at% = (twm.at% And &b10111111) Or (z% << 6)
End Sub

Sub twm.inverse(z%)
  twm.at% = (twm.at% And &b01111111) Or (z% << 7)
End Sub

' Print s$ at (x%, y%) in the current window.
' Leaves twm.x% and twm.y% one beyond the last printed char position.
Sub twm.print_at(x%, y%, s$)
  twm.lock_vga_cursor(1)
  twm.x% = x%
  twm.y% = y%
  twm.print(s$)
  twm.lock_vga_cursor(0)
End Sub

Function sys.int_bounds_err$(var$, value%, lb%, ub%)
  sys.int_bounds_err$ = "'" + var$ + "' value " + Str$(value%) + " out of bounds (" + Str$(lb%)
  Cat sys.int_bounds_err$, " ... " + Str$(ub%) + ")"
End Function

Sub twm.putc(ch%)
  Local at% = twm.at%
  Local s$ = Chr$(ch%)
  Local of% = twm.y% * twm.w% + twm.x%
  Local ax% = twm.a% + twm.x%
  Local by% = twm.b% + twm.y%

  ' Write ch% and at% to twm.data%()
  Poke Byte twm.pc% + of%, ch%
  Poke Byte twm.pa% + of%, at%

  ' Update serial console:
  If twm.last_at% <> at% Then Print vt$; : twm.last_at% = at%
  Print Chr$(27) "[" Str$(by%) ";" Str$(ax%) "H" s$;

  ' Update VGA display:
  Text (ax%-1)*twm.fw%,(by%-1)*twm.fh%,s$,,,,twm.fg%(at%),twm.bg%(at%)
  If at% And &b01000000 Then Text (ax%-1)*twm.fw%+1,(by%-1)*twm.fh%,s$,,,,twm.fg%(at%),-1
End Sub

Sub twm.print(s$)

'  If twm.x% < 0 Or twm.x% >= twm.w% Then Error sys.int_bounds_err$("x", twm.x%, 0, twm.w% - 1)
'
'  If twm.y% = twm.h% then the window will immediately scroll before printing the first character.
'  If twm.y% < 0 Or twm.y% > twm.h% Then Error sys.int_bounds_err$("y", twm.y%, 0, twm.h%)

  Local is% = 1                         ' current index into s$
  Local ls% = Len(s$)                   ' length of s$
  Local ps% = Peek(VarAddr s$)          ' pointer to s$
  Local nc% = Min(twm.w% - twm.x%, ls%) ' number of characters to print on the current line
  Local of%                             ' offset for writing to twm.data%()
  Local at% = twm.at%
  Local fg% = twm.fg%(at%)
  Local bg% = twm.bg%(at%)
  Local ax%
  Local by%
  Local seg$
  Local vt$ = twm.vt$(at%)

  If nc% = 0 Then Exit Sub

  ' Set serial console VT100 attributes
  If twm.last_at% <> at% Then Print vt$; : twm.last_at% = at%

  twm.lock_vga_cursor(1)

  Do
    If twm.y% = twm.h% Then
      twm.scroll_up(1)
      Inc twm.y%, -1
      ' Restore attributes that may have been changed by scroll_up()
      If twm.last_at% <> at% Then Print vt$ : twm.last_at% = at%
    EndIf

    ' Write text and attributes to twm.data%()
    of% = twm.y% * twm.w% + twm.x%
    Memory Copy ps% + is%, twm.pc% + of%, nc%
    Memory Set twm.pa% + of%, at%, nc%

    seg$ = Mid$(s$, is%, nc%)
    ax% = twm.a% + twm.x%
    by% = twm.b% + twm.y%

    ' Update serial console:
    Print Chr$(27) "[" Str$(by%) ";" Str$(ax%) "H" seg$;

    ' Update VGA display:
    Text (ax% - 1) * twm.fw%, (by% - 1) * twm.fh%, seg$,,,, fg%, bg%
    If at% And &b01000000 Then Text (ax% - 1) * twm.fw% + 1, (by% - 1) * twm.fh%, seg$,,,, fg%, -1

    Inc is%, nc%
    Inc twm.x%, nc%

    ' Otherwise move to the next line of text.
    If twm.x% = twm.w% Then
      twm.x% = 0
      Inc twm.y%
      nc% = Min(twm.w% - twm.x%, ls% - is% + 1)
    EndIf
  Loop While is% <= ls%

  twm.lock_vga_cursor(0)

End Sub

Sub twm.scroll_down(redraw%)
  twm.lock_vga_cursor(1)

  Local pa% = twm.pa% + twm.w% * (twm.h% - 1)
  Local pc% = twm.pc% + twm.w% * (twm.h% - 1)
  Local y%

  Do While pa% > twm.pa%
    ' Copy attribute and character data down one line.
    Memory Copy pa% - twm.w%, pa%, twm.w%
    Memory Copy pc% - twm.w%, pc%, twm.w%
    Inc pa%, -twm.w%
    Inc pc%, -twm.w%
  Loop

  ' Clear the first line using the current attributes.
  Memory Set pa%, twm.at%, twm.w%
  Memory Set pc%, 32, twm.w%

  If redraw% Then twm.redraw()

  twm.lock_vga_cursor(0)
End Sub

Sub twm.scroll_up(redraw%)
  twm.lock_vga_cursor(1)

  Local pa% = twm.pa%
  Local pc% = twm.pc%
  Local y%

  For y% = 1 To twm.h% - 1
    ' Copy attribute and character data from line y + 1 to line y.
    Memory Copy pa% + twm.w%, pa%, twm.w%
    Memory Copy pc% + twm.w%, pc%, twm.w%
    Inc pa%, twm.w%
    Inc pc%, twm.w%
  Next

  ' Clear the last line using the current attributes.
  Memory Set pa%, twm.at%, twm.w%
  Memory Set pc%, 32, twm.w%

  If redraw% Then twm.redraw()

  twm.lock_vga_cursor(0)
End Sub

Sub twm.redraw()
  Local at%, ch$, x%, y%
  Local pa% = twm.pa%
  Local pc% = twm.pc%
  Local vx%, vy%

  vy% = (twm.b% - 1) * twm.fh%
  For y% = 0 To twm.h% - 1
    Print Chr$(27) "[" Str$(twm.b% + y%) ";" Str$(twm.a%) "H";
    vx% = (twm.a% - 1) * twm.fw%
    For x% = 0 To twm.w% - 1
      at% = Peek(Byte pa% + x%)
      ch$ = Chr$(Peek(Byte pc% + x%))

      ' Update serial console:
      If twm.last_at% <> at% Then Print twm.vt$(at%); : twm.last_at% = at%
      Print ch$;

      ' Update VGA display:
      Text vx%, vy%, ch$,,,, twm.fg%(at%), twm.bg%(at%)
      If at% And &b01000000 Then Text vx%+1, vy%, ch$,,,, twm.fg%(at%), -1
      Inc vx%, twm.fw%
    Next x%
    Inc pa%, twm.w%
    Inc pc%, twm.w%
    Inc vy%, twm.fh%
  Next y%
End Sub

Sub twm.cls()
  twm.lock_vga_cursor(1)
  Memory Set twm.pc%, 32, twm.w% * twm.h%
  Memory Set twm.pa%, twm.at%, twm.w% * twm.h%
  twm.redraw()
  twm.lock_vga_cursor(0)
End Sub

Sub twm.box(x%, y%, w%, h%)
  Local i%
  Local pc% = twm.pc%
  Local s$
  Local ad%

  twm.lock_vga_cursor(1)

  ' Top
  Poke Var s$, 0, w%
  ad% = pc% + twm.w% * y% + x%
  Poke Var s$, 1, twm.box_or%(&hC9, Peek(Byte ad%))
  For i% = 2 To w% - 1
    Poke Var s$, i%, twm.box_or%(&hCD, Peek(Byte ad% + i% - 1))
  Next
  Poke Var s$, w%, twm.box_or%(&hBB, Peek(Byte ad% + w% - 1))
  twm.print_at(x%, y%, s$)

  ' Sides
  For twm.y% = y% + 1 To y% + h% - 2
    Inc ad%, twm.w%
    twm.x% = x%
    twm.putc(twm.box_or%(&hBA, Peek(Byte ad%)))
    twm.x% = x% +  w% - 1
    twm.putc(twm.box_or%(&hBA, Peek(Byte ad% + w% - 1)))
  Next

  ' Bottom
  Inc ad%, twm.w%
  Poke Var s$, 1, twm.box_or%(&hC8, Peek(Byte ad%))
  For i% = 2 To w% - 1
    Poke Var s$, i%, twm.box_or%(&hCD, Peek(Byte ad% + i% - 1))
  Next
  Poke Var s$, w%, twm.box_or%(&hBC, Peek(Byte ad% + w% - 1))
  twm.print_at(x%, y% + h% - 1, s$)

  twm.lock_vga_cursor(0)
End Sub

' Combines a new character 'ch%' and an existing character 'ex%'.
' If they are both 'box-drawing' characters then returns an appropriate junction character,
' otherwise returns the new character 'ch%'.
'
' @param ch%  new 'box-drawing' character.
' @param ex%  existing character.
' @return     junction character, or 'ch%'.
Function twm.box_or%(ch%, ex%)
  Local tmp% = twm.c2b%(ch%)
  twm.box_or% = Choice(tmp% = 0, ch%, twm.b2c%(tmp% Or twm.c2b%(ex%)))
End Function

' Sends VT100 control-code to hide/show cursor to the serial console.
Sub twm.enable_cursor(z%)
  Local s$ = LCase$(Mm.Info$(Option Console))
  Select Case s$
    Case "screen" : Exit Sub
    Case "both"
      ' Note MMB4W cannot call OPTION CONSOLE SERIAL if it is on the same line
      ' as a CASE clause. This is because in MMB4W this OPTION checks
      ' 'CurrentLinePtr' to make sure it is only being used from within a
      ' program. However due to a long-standing and gnarly bug in the MMBasic
      ' core 'CurrentLinePtr' is not correctly set when processing a command on
      ' the same line as a CASE clause.
      Option Console Serial
  End Select
  Print Chr$(27) Choice(z%, "[?25h", "[?25l");
  If s$ = "both" Then Option Console Both

  If z% = twm.cursor_enabled% Then Exit Sub
  twm.cursor_enabled% = z%
  If z% Then
    SetTick 500, twm.update_vga_cursor, 4
  Else
    SetTick 0, twm.update_vga_cursor, 4
  EndIf
End Sub

' Sounds the console "bell".
Sub twm.bell()
  Play Tone 800, 800, 200
  Local s$ = LCase$(Mm.Info$(Option Console))
  Select Case s$
    Case "screen" : Exit Sub
    Case "both"
      ' See comment in twm.enable_cursor()
      Option Console Serial
  End Select
  Print Chr$(7);
  If s$ = "both" Then Option Console Both
End Sub

' Toggle whether the VGA cursor is lit or not,
' intended to be called as an ISR via SetTick.
Sub twm.update_vga_cursor()
  Static lit% = 0

  If twm.cursor_locked% Then Exit Sub

  ' Do not draw the cursor if it is out of bounds,
  ' if there are issues may need to check for < 0 too.
  If twm.x% >= twm.w% Or twm.y% >= twm.h% Then Exit Sub

  lit% = Not lit%
  Local of% = twm.y% * twm.w% + twm.x%
  Local ax% = twm.a% + twm.x%
  Local by% = twm.b% + twm.y%
  Local fg% = Choice(lit%, RGB(Black), RGB(White))
  Text (ax%-1)*twm.fw%,(by%-1)*twm.fh%,"_",,,,fg%,-1
  Text (ax%-1)*twm.fw%,(by%-1)*twm.fh% + 1,"_",,,,fg%,-1
End Sub

' Lock/unlock the VGA cursor.
' The cursor should be locked whilst manipulating twm.x% & twm.y%
Sub twm.lock_vga_cursor(lock%)
  Inc twm.cursor_locked%, Choice(lock%, 1, -1))
  If twm.id% = -1 Then Exit Sub

  If lock% < 0 Then Error "Unbalanced cursor lock/unlock."

  ' Character cell contents has already been restored and we are just counting up.
  If lock% > 1 Then Exit Sub

  ' Do not restore character cell contents if cursor is out of bounds,
  ' if there are issues may need to check for < 0 too.
  If twm.x% >= twm.w% Or twm.y% >= twm.h% Then Exit Sub

  ' Restore character cell contents.
  Local ax% = twm.a% + twm.x%
  Local by% = twm.b% + twm.y%
  Local of% = twm.y% * twm.w% + twm.x%
  Local ch% = Peek(Byte twm.pc% + of%)
  Local at% = Peek(Byte twm.pa% + of%)
  Local fg% = twm.fg%(at%)
  Text (ax%-1)*twm.fw%,(by%-1)*twm.fh%,Chr$(ch%),,,,fg%,twm.bg%(at%)
  If at% And &b01000000 Then Text (ax%-1)*twm.fw%+1,(by%-1)*twm.fh%,Chr$(ch%),,,,fg%,-1
End Sub
