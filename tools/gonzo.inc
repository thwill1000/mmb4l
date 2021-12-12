' Copyright (c) 2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For MMB4L 2021.01.00

On Error Skip 1 : Dim sys.VERSION$ = ""
If sys.VERSION$ = "" Then Error "'system.inc' not included"
sys.requires("console", "file", "inifile", "map", "string", "spsh")
sys.provides("gonzo")
If sys.err$ <> "" Then Error sys.err$

Const ECHO% = &h01
Const COLLECT% = &h02
Const SKIP_EMPTY% = &h04

Const gonzo.DEVICE_DISCONNECTED% = 0
Const gonzo.DEVICE_CMM2% = 1
Const gonzo.DEVICE_PICOMITE% = 2
Const gonzo.DEVICE_OTHER% = 3

Const gonzo.SERIAL_FNBR% = 1
Const gonzo.IN_FNBR% = 2
Const gonzo.LOG_FNBR% = 3
Const gonzo.INI_FNBR% = 4
Const gonzo.OUT_FNBR% = 5

Dim gonzo.comspec$
Dim gonzo.device_name$ = "DISCONNECTED"
Dim gonzo.device% = gonzo.DEVICE_DISCONNECTED%
Dim gonzo.logfile$ = "gonzo.log"
Dim gonzo.rx_buf$
Dim gonzo.buf%(1000)
Dim gonzo.exit_flag%

Sub gonzo.connect(verbose%)
  con.println("Connecting to " + gonzo.comspec$ + " ...")

  On Error Skip 1
  Close #gonzo.LOG_FNBR%
  On Error Clear

  On Error Skip 1
  Close #gonzo.SERIAL_FNBR%
  On Error Clear

  Open gonzo.logfile$ For Output As #gonzo.LOG_FNBR%

  On Error Skip 1
  Open gonzo.comspec$ As #gonzo.SERIAL_FNBR%
  If Mm.ErrMsg$ <> "" Then
    con.errorln("Failed to connect to: " + gonzo.comspec$)
    gonzo.device_name$ = "DISCONNECTED"
    On Error Clear
  Else
    gonzo.device_name$ = gonzo.get_device$()
  EndIf

  Select Case gonzo.device_name$
    Case "Colour Maximite 2", "Colour Maximite 2 G2" : gonzo.device% = gonzo.DEVICE_CMM2%
    Case "DISCONNECTED" : gonzo.device% = gonzo.DEVICE_DISCONNECTED%
    Case "PicoMite"     : gonzo.device% = gonzo.DEVICE_PICOMITE%
    Case Else           : gonzo.device% = gonzo.DEVICE_OTHER%
  End Select

  If verbose% Then
    If gonzo.device% <> gonzo.DEVICE_DISCONNECTED% Then
      con.println("Connected to " + gonzo.device_name$)
    EndIf
    gonzo.update_title()
  EndIf
End Sub

Sub gonzo.update_title(prefix$)
  If prefix$ = "" Then
    Console SetTitle gonzo.device_name$ + " - " + gonzo.comspec$ + " - gonzo v" + sys.VERSION$
  Else
    Console SetTitle prefix$ + " - " + gonzo.device_name$ + " - " + gonzo.comspec$ + " - gonzo v" + sys.VERSION$
  EndIf
End Sub

' @return  1 if the exit flag is set, otherwise 0.
Function gonzo.do_command%(cmd$, argc%, argv$())
  Local call_cmd$, i%

  ' If cmd$ is "R" or "REMOTE" then suffix the first argument with R and
  ' use that as the cmd$ instead, e.g. "REMOTE TYPE foo" => "RTYPE foo".
  If LCase$(cmd$) = "r" Or LCase$(cmd$) = "remote" And argc% > 0 Then
    cmd$ = "r" + argv$(0)
    For i% = 1 To argc% - 1
      argv$(i% - 1) = argv$(i%)
    Next
    argv$(argc% - 1) = ""
    Inc argc%, -1
  EndIf

  ' Lookup command.
  Restore gonzo_commands
  Local synonyms$, usage$, description$
  Do
    Read synonyms$, usage$, description$
    If synonyms$ = "<END>" Then Exit Do
    If InStr(synonyms$, "|" + LCase$(cmd$) + "|") Then
      call_cmd$ = Mid$(synonyms$, 2, InStr(2, synonyms$, "|") - 2)
    EndIf
  Loop Until call_cmd$ <> ""

  sys.err$ = ""
  If call_cmd$ = "" Then sys.err$ = "Unknown command '" + cmd$ + "'"

  If sys.err$ = "" Then
    On Error Skip 1
    Call "gonzo.cmd_" + call_cmd$, cmd$, argc%, argv$()
    If Mm.ErrMsg$ <> "" Then sys.err$ = Mm.ErrMsg$
  EndIf

  If sys.err$ <> "" Then con.errorln(sys.err$)

  gonzo.do_command% = gonzo.exit_flag%
End Function

Sub gonzo.cmd_break(cmd$, argc%, argv$())
  If argc% <> 0 Then sys.err$ = gonzo.usage$(cmd$) : Exit Sub

  gonzo.break()
  gonzo.read_until_prompt()
End Sub

Sub gonzo.cmd_connect(cmd$, argc%, argv$())
  Local history%(array.new%(128)), s$
  Local devices$(array.new%(5)) = ( "/dev/ttyS0", "/dev/ttyS1", "/dev/ttyACM0", "/dev/ttyACM1", "/dev/ttyACM2" )
  con.history_fill(history%(), devices$())
  con.print("Device? ")
  Local device$ = con.readln$("/dev/ttyS0", history%())
  If sys.break_flag% Then Goto gonzo.cmd_connect_break:

  Local baud_rates$(array.new%(11)) = ( "9600", "19200", "28800", "38400", "57600", "76800", "115200", "230400", "460800", "576000", "921600" )
  con.history_fill(history%(), baud_rates$())
  con.print("Baud? ")
  Local baud$ = con.readln$("115200", history%())

gonzo.cmd_connect_break:

  If sys.break_flag% Then
    Print
  Else
    gonzo.comspec$ = device$ + ": " + baud$
    gonzo.connect(1)
  EndIf
End Sub

Sub gonzo.cmd_device(cmd$, argc%, argv$())
  con.println(gonzo.device_name$)
End Sub

Sub gonzo.cmd_exit(cmd$, argc%, argv$())
  If argc% <> 0 Then sys.err$ = gonzo.usage$(cmd$) : Exit Sub
  gonzo.exit_flag% = 1
End Sub

Sub gonzo.cmd_files(cmd$, argc%, argv$())
  If argc% > 1 Then sys.err$ = gonzo.usage$(cmd$)
  If sys.err$ <> "" Then Exit Sub

  spsh.cmd_files(cmd$, argc%, argv$())
End Sub

Sub gonzo.cmd_help(cmd$, argc%, argv$())
  If argc% > 1 Then sys.err$ = gonzo.usage$(cmd$) : Exit Sub

  If argc% = 1 Then
    con.println(gonzo.usage$(argv$(0)))
    Exit Sub
  EndIf

  con.println("Commands are:")
  Restore gonzo_commands
  Local synonyms$, usage$, description$
  Do
    Read synonyms$, usage$, description$
    If synonyms$ = "<END>" Then Exit Do
    synonyms$ = Mid$(synonyms$, 2, InStr(2, synonyms$, "|") - 2)
    con.println("  " + str.rpad$(synonyms$, 10) + "  " + description$)
  Loop
End Sub

Sub gonzo.cmd_log(cmd$, argc%, argv$())
  If argc% = 1 And UCase$(argv$(0)) = "CLEAR" Then
    Close #gonzo.LOG_FNBR%
    Open gonzo.logfile$ For Output As gonzo.LOG_FNBR%
    Exit Sub
  ElseIf argc% <> 0 Then
    sys.err$ = gonzo.usage$(cmd$)
    Exit Sub
  EndIf

  Open gonzo.logfile$ For Input As #gonzo.IN_FNBR%
  con.foreground("white")
  Local c%, ch$
  Do While Not Eof(#gonzo.IN_FNBR%)
    ch$ = Input$(1, #gonzo.IN_FNBR%)
    c% = Asc(ch$)
    Select Case c%
      Case 0 To 31, 127 To 255
        con.foreground("green")
        Print "<" Hex$(c%, 2) ">";
        con.foreground("white")
        If c% = &h0A Then Print
      Case Else
        Print ch$;
    End Select
  Loop
  Close #gonzo.IN_FNBR%
  con.println()
  con.foreground("default")
End Sub

Sub gonzo.cmd_rchdir(cmd$, argc%, argv$())
  If argc% <> 1 Then sys.err$ = gonzo.usage$(cmd$) : Exit Sub
  Local file$ = str.unquote$(argv$(0))

  gonzo.remote_file_cmd("ChDir " + str.quote$(file$))
End Sub

Sub gonzo.remote_file_cmd(command$)
  If gonzo.device% = gonzo.DEVICE_DISCONNECTED% Then sys.err$ = "Error: no connection" : Exit Sub

  gonzo.break()
  gonzo.read_until_prompt()
  gonzo.writeln(command$)
  gonzo.read_until_match("> " + command$)
  LongString Clear gonzo.buf%()
  gonzo.read_until_prompt(COLLECT% Or SKIP_EMPTY%)
  If LInStr(gonzo.buf%(), "Could not find") Then
    con.println("Error: file or directory not found")
  ElseIf LInStr(gonzo.buf%(), "Access denied") Then
    con.println("Error: file or directory exists")
  Else
    ' con.print(LGetStr$(gonzo.buf%(), 1, 255))
    con.println("OK")
  EndIf
End Sub

Sub gonzo.cmd_rcopy(cmd$, argc%, argv$())
  Local src$, dst$
  Select Case argc%
    Case 2
      src$ = str.unquote$(argv$(0))
      dst$ = str.unquote$(argv$(1))
    Case 3
      If UCase$(argv$(1)) <> "TO" Then
        sys.err$ = "syntax"
      Else
        src$ = str.unquote$(argv$(0))
        dst$ = str.unquote$(argv$(2))
      EndIf
    Case Else
      sys.err$ = gonzo.usage$(cmd$)
      Exit Sub
  End Select

  gonzo.remote_file_cmd("Copy " + str.quote$(src$) + " To " + str.quote$(dst$))
End Sub

Sub gonzo.cmd_rdelete(cmd$, argc%, argv$())
  If argc% <> 1 Then sys.err$ = gonzo.usage$(cmd$) : Exit Sub
  Local file$ = str.unquote$(argv$(0))

  gonzo.remote_file_cmd("Kill " + str.quote$(file$))
End Sub

Sub gonzo.cmd_receive(cmd$, argc%, argv$())
  If gonzo.device% = gonzo.DEVICE_DISCONNECTED% Then sys.err$ = "Error: no connection"
  If argc% <> 1 Then sys.err$ = gonzo.usage$(cmd$)
  If sys.err$ <> "" Then Exit Sub
  Local file$ = str.unquote$(argv$(0))

  Local file_size% = gonzo.get_file_size%(file$)
  If sys.err$ <> "" Then Exit Sub

  ' Send LIST command.
  Local command$ = "List " + str.quote$(file$)
  gonzo.writeln(command$)
  gonzo.read_until_match("> " + command$)

  ' Open the file to receive.
  Open file.get_name$(file$) For Output As #gonzo.OUT_FNBR%

  con.println("Autosave receiving " + Str$(file_size%) + " bytes ...")
  Local t% = Timer

  ' Read lines and echo them to the file.
  Local count% = 0, s$, x%
  Local term$ = Chr$(27) + "[?25h" + Chr$(27) + "[37m" + Chr$(27) + "[m"
  Do While Not sys.break_flag%
    x% = gonzo.getln%(s$)
    If Not x% Then
      If gonzo.rx_buf$ = "> " Then Exit Do
      gonzo.handle_press_any_key()
      Continue Do
    EndIf

    If s$ = term$ Then Exit Do
    If Left$(s$, 8) = "Error : " Then Exit Do
    Print #gonzo.OUT_FNBR%, s$ Chr$(&h0A);
    Inc count%
    con.println(Choice(count% > 1, Chr$(27) + "[1F", "") + "Received " + Str$(count%) + " lines")
  Loop

  Close #gonzo.OUT_FNBR%

  If sys.break_flag% Then sys.err$ = "Interrupted by user"
  If sys.err$ = "" Then con.println("OK, " + Str$((file_size% * 8000) / (Timer - t%), 0, 1) + " bit/s")
End Sub

Sub gonzo.cmd_rfiles(cmd$, argc%, argv$())
  If gonzo.device% = gonzo.DEVICE_DISCONNECTED% Then sys.err$ = "Error: no connection"
  If argc% > 2 Then sys.err$ = gonzo.usage$(cmd$)
  If sys.err$ <> "" Then Exit Sub

  Local d$ = Choice(argv$(0) = "", "*", str.unquote$(argv$(0)))
  If file.is_directory%(d$) Then Cat d$, "/*"

  gonzo.break()
  gonzo.read_until_prompt()
  Local command$ = Choice(gonzo.device% = gonzo.DEVICE_CMM2%, "Ls ", "Files ") + str.quote$(d$)
  gonzo.writeln(command$)
  gonzo.read_until_match("> " + command$)

  Local x%, s$
  Do
    x% = gonzo.getln%(s$)
    If Not x% Then
      gonzo.handle_press_any_key()
    ElseIf s$ <> "" Then
      If Left$(s$, 1) <> ">" Then con.println(str.trim$(s$))
      If Left$(s$, 8) = "Error : " Then Exit Do
    EndIf
  Loop Until file.fnmatch%("* director*file*", s$)
End Sub

Sub gonzo.cmd_rmkdir(cmd$, argc%, argv$())
  If argc% <> 1 Then sys.err$ = gonzo.usage$(cmd$) : Exit Sub
  Local file$ = str.unquote$(argv$(0))

  gonzo.remote_file_cmd("MkDir " + str.quote$(file$))
End Sub

Sub gonzo.cmd_rpwd(cmd$, argc%, argv$())
  If argc% <> 0 Then sys.err$ = gonzo.usage$(cmd$) : Exit Sub

  gonzo.break()
  gonzo.read_until_prompt()
  Local command$ = "? Cwd$"
  gonzo.writeln(command$)
  gonzo.read_until_match("> " + command$)
  gonzo.read_until_prompt(ECHO% Or SKIP_EMPTY%)
End Sub

Sub gonzo.cmd_rrename(cmd$, argc%, argv$())
  Local src$, dst$
  Select Case argc%
    Case 2
      src$ = str.unquote$(argv$(0))
      dst$ = str.unquote$(argv$(1))
    Case 3
      If UCase$(argv$(1)) <> "AS" Then
        sys.err$ = "syntax"
      Else
        src$ = str.unquote$(argv$(0))
        dst$ = str.unquote$(argv$(2))
      EndIf
    Case Else
      sys.err$ = gonzo.usage$(cmd$)
      Exit Sub
  End Select

  gonzo.remote_file_cmd("Rename " + str.quote$(src$) + " As " + str.quote$(dst$))
End Sub

Sub gonzo.cmd_rsize(cmd$, argc%, argv$())
  If argc% <> 1 Then sys.err$ = gonzo.usage$(cmd$) : Exit Sub
  Local file$ = str.unquote$(argv$(0))

  Local size% = gonzo.get_file_size%(file$)
  If sys.err$ = "" Then con.println(Str$(size%))
End Sub

Sub gonzo.cmd_rtype(cmd$, argc%, argv$())
  If argc% <> 1 Then sys.err$ = gonzo.usage$(cmd$) : Exit Sub
  Local file$ = str.unquote$(argv$(0))

  gonzo.break()
  gonzo.read_until_prompt()
  Local command$ = "List " + str.quote$(file$)
  gonzo.writeln(command$)
  gonzo.read_until_match("> " + command$)

  ' Read lines and echo them to the file until we reach a prompt.
  Local s$, x%
  Do While Not sys.break_flag%
    x% = gonzo.getln%(s$)
    If x% Then
      con.println(s$)
    Else
      If Left$(gonzo.rx_buf$, 2) = "> " Then Exit Do
      gonzo.handle_press_any_key()
    EndIf
  Loop
End Sub

Sub gonzo.cmd_send(cmd$, argc%, argv$())
  If gonzo.device% = gonzo.DEVICE_DISCONNECTED% Then sys.err$ = "Error: no connection"
  If argc% <> 1 Then sys.err$ = gonzo.usage$(cmd$)
  If sys.err$ <> "" Then Exit Sub
  Local file$ = str.unquote$(argv$(0))
  If Not file.exists%(file$) Then sys.err$ = "Error: file not found" : Exit Sub

  ' Open the file to send.
  Open file$ For Input As #gonzo.IN_FNBR%

  ' Cleanup for a new command.
  gonzo.break()
  gonzo.read_until_prompt()

  ' Send AUTOSAVE command.
  Local command$ = "Autosave"
  If gonzo.device% = gonzo.DEVICE_CMM2% Then Cat command$, " " + str.quote$(file.get_name$(file$))
  gonzo.writeln(command$)
  gonzo.read_until_match("> " + command$)
  Local s$
  If gonzo.getln%(s$) Then
    sys.err$ = gonzo.extract_error$(s$)
    If sys.err$ <> "" Then Close #gonzo.IN_FNBR% : Exit Sub
  EndIf

  Local file_size% = Mm.Info(FileSize file$)
  con.println("Autosave sending " + Str$(file_size%) + " bytes ...")
  Local t% = Timer

  ' Echo the file to the serial port.
  Local count% = 0, x%
  Do While Not Eof(#gonzo.IN_FNBR%) And Not sys.break_flag%
    Line Input #gonzo.IN_FNBR%, s$
    gonzo.writeln(s$)
    x% = gonzo.getln%(s$)
    Inc count%
    con.println(Choice(count% > 1, Chr$(27) + "[1F", "") + "Sent " + Str$(count%) + " lines")
  Loop
  Close #gonzo.IN_FNBR%

  If sys.break_flag% Then sys.err$ = "Interrupted by user" : Exit Sub

  ' Send F6 or Ctrl-Z to terminate as appropriate.
  gonzo.write(Choice(gonzo.device% = gonzo.DEVICE_CMM2%, Chr$(27) + "[17~", Chr$(26)))
  gonzo.read_until_prompt()

  ' Except on the CMM2 send SAVE command to write from flash to disk.
  If (gonzo.device% <> gonzo.DEVICE_CMM2%) Then
    command$ = "Save " + str.quote$(file.get_name$(file$))
    gonzo.writeln(command$)
    gonzo.read_until_match("> " + command$)
    If (gonzo.getln%(s$)) Then sys.err$ = gonzo.extract_error$(s$)
  EndIf

  If sys.err$ = "" Then con.println("OK, " + Str$((file_size% * 8000) / (Timer - t%), 0, 1) + " bit/s")
End Sub

Sub gonzo.cmd_term(cmd$, argc%, argv$())
  If gonzo.device% = gonzo.DEVICE_DISCONNECTED% Then sys.err$ = "Error: no connection"
  If argc% <> 0 Then sys.err$ = gonzo.usage$(cmd$)
  If sys.err$ <> "" Then Exit Sub

  gonzo.update_title("TERMINAL")

  Do While Inkey$ <> "" : Loop

  On Error Ignore

  Local buf$, break_time%, ch%, s$
  Do
    ' On a Ctrl-C:
    '   - if it is more than 500 ms since the last Ctrl-C then send to remote,
    '   - otherwise exit terminal mode.
    If sys.break_flag% Then
      sys.break_flag% = 0
      If Timer - break_time% > 500 Then
        gonzo.write(Chr$(3))
        break_time% = Timer
      Else
        Exit Do
      EndIf
    EndIf

    ch% = gonzo.getc%(0)
    If ch% > 0 Then con.print(Chr$(ch%));
    If Len(buf$) = 0 Then buf$ = Inkey$
    If Len(buf$) > 0 Then
      gonzo.write(buf$)
      buf$ = ""
    EndIf
  Loop While Mm.ErrNo = 0

  Select Case Mm.ErrNo
    Case 0
      ' Do nothing
    Case 5
      sys.err$ = "Error: connection lost"
      gonzo.device_name$ = "DISCONNECTED"
      gonzo.device% = gonzo.DEVICE_DISCONNECTED%
    Case Else
      sys.err$ = Mm.ErrMsg$
  End Select

  On Error Clear

  If Pos > 1 Then con.println()
  Console ShowCursor

  gonzo.update_title()
End Sub

Sub gonzo.cmd_version(cmd$, argc%, argv$())
  If argc% <> 0 Then sys.err$ = gonzo.usage$(cmd$) : Exit Sub

  ' TODO: duplicated from common/sptools.inc
  con.println("gonzo (SP Tools) Release " + sys.VERSION$ + " for MMB4L 2021.01.00")
  con.println("Copyright (c) 2021 Thomas Hugo Williams")
  con.println("A Toy Plastic Trumpet Production for Sockpuppet Studios.")
  con.println("License MIT <https://opensource.org/licenses/MIT>")
  con.println("This is free software: you are free to change and redistribute it.")
  con.println("There is NO WARRANTY, to the extent permitted by law.")
End Sub

Sub gonzo.cmd_xreceive(cmd$, argc%, argv$())
  If gonzo.device% = gonzo.DEVICE_DISCONNECTED% Then sys.err$ = "Error: no connection"
  If argc% <> 1 Then sys.err$ = gonzo.usage$(cmd$)
  If sys.err$ <> "" Then Exit Sub
  Local remote_file$ = str.unquote$(argv$(0))

  Local file_size% = gonzo.get_file_size%(remote_file$)
  If sys.err$ <> "" Then Exit Sub
  Local local_file$ = file.get_name$(remote_file$)

  ' Send XMODEM SEND command.
  Local command$ = "XModem Send " + str.quote$(remote_file$)
  gonzo.writeln(command$)
  gonzo.read_until_match("> " + command$)
  Local s$
  If gonzo.getln%(s$) Then
    sys.err$ = gonzo.extract_error$(s$)
    If sys.err$ <> "" Then Exit Sub
  EndIf

  con.println("XModem receiving " + Str$(file_size%) + " bytes ...")
  Local t% = Timer
  XModem Receive local_file$, #gonzo.SERIAL_FNBR%

  If sys.err$ = "" Then
    ' Trim padding off received file.
    Open local_file$ For Input As #gonzo.IN_FNBR%
    Local tmp_file$ = file.TMPDIR$ + "/" + local_file$
    Open tmp_file$ For Output As #gonzo.OUT_FNBR%
    Local remaining% = file_size%
    Do While remaining% > 0
      s$ = Input$(Min(255, remaining%), #gonzo.IN_FNBR%)
      Print #gonzo.OUT_FNBR%, s$;
      Inc remaining%, -Min(255, remaining%)
    Loop
    Close #gonzo.OUT_FNBR%
    Close #gonzo.IN_FNBR%
    Copy tmp_file$ To local_file$
  EndIf

  If sys.err$ = "" Then con.println("OK, " + Str$((file_size% * 8000) / (Timer - t%), 0, 1) + " bit/s")
End Sub

Sub gonzo.cmd_xsend(cmd$, argc%, argv$())
  If gonzo.device% = gonzo.DEVICE_DISCONNECTED% Then sys.err$ = "Error: no connection"
  If argc% <> 1 Then sys.err$ = gonzo.usage$(cmd$)
  If sys.err$ = "" Then
    Local local_file$ = str.unquote$(argv$(0))
    If Not file.exists%(local_file$) Then sys.err$ = "Error: file not found"
  EndIf
  If sys.err$ <> "" Then Exit Sub
  Local remote_file$ = file.get_name$(local_file$)

  ' Cleanup for a new command.
  gonzo.break()
  gonzo.read_until_prompt()

  ' Send XMODEM RECEIVE command.
  Local command$ = "XModem Receive " + str.quote$(remote_file$)
  gonzo.writeln(command$)
  gonzo.read_until_match("> " + command$)
  Local s$
  If gonzo.getln%(s$) Then
    sys.err$ = gonzo.extract_error$(s$)
    If sys.err$ <> "" Then Exit Sub
  EndIf

  Local file_size% = Mm.Info(FileSize local_file$)
  con.println("XModem sending " + Str$(file_size%) + " bytes ...")
  Local t% = Timer
  XModem Send local_file$, #gonzo.SERIAL_FNBR%

  If sys.err$ = "" Then
    ' Send and run 'rizzo.bas' on the remote to remove padding from file.
    gonzo.break()
    gonzo.read_until_prompt()
    command$ = "Autosave " + str.quote$("/tmp/rizzo.bas")
    gonzo.writeln(command$)
    gonzo.read_until_match("> " + command$)

    gonzo.writeln("Open " + str.quote$(remote_file$) + " For Input As #1")
    gonzo.writeln("Open " + str.quote$("/tmp/" + remote_file$) + " For Output As #2")
    gonzo.writeln("Dim x% = " + Str$(file_size%))
    gonzo.writeln("Dim s$")
    gonzo.writeln("Do While x% > 0")
    gonzo.writeln("  s$ = Input$(Min(255, x%), #1)")
    gonzo.writeln("  Print #2, s$;")
    gonzo.writeln("  Inc x%, -Min(255, x%)")
    gonzo.writeln("Loop")
    gonzo.writeln("Close #2")
    gonzo.writeln("Close #1")
    gonzo.writeln("Copy " + str.quote$("/tmp/" + remote_file$) + " To " + str.quote$(remote_file$))

    gonzo.write(Choice(gonzo.device% = gonzo.DEVICE_CMM2%, Chr$(27) + "[17~", Chr$(26)))
    gonzo.read_until_prompt()

    gonzo.writeln("Run " + str.quote$("/tmp/rizzo.bas"))
    gonzo.read_until_prompt()
  EndIf

  If sys.err$ = "" Then con.println("OK, " + Str$((file_size% * 8000) / (Timer - t%), 0, 1) + " bit/s")
End Sub

' Writes a Ctrl-C.
Sub gonzo.break()
  gonzo.write(Chr$(3))
End Sub

Function gonzo.extract_error$(s$)
  Local buf%(array.new%(32))
  LongString Append buf%(), s$
  gonzo.extract_error$ = gonzo.extract_error_ls$(buf%())
End Function

Function gonzo.extract_error_ls$(buf%())
  Local st% = LInStr(buf%(), "Error : ")
  If Not st% Then Exit Function
  Inc st%, 8
  Local en% = st%
  For en% = st% To LLen(buf%())
    Select Case LGetByte(buf%(), en%)
      Case 10, 13 : Exit For
    End Select
  Next
  gonzo.extract_error_ls$ = "Remote error: " + LGetStr$(buf%(), st%, en% - st% + 1)
End Function

Function gonzo.get_device$()
  gonzo.break()
  gonzo.read_until_prompt()
  gonzo.writeln("? Mm.Device$")
  gonzo.read_until_match("> ? Mm.Device$")
  LongString Clear gonzo.buf%()
  gonzo.read_until_prompt(COLLECT% Or SKIP_EMPTY%)
  If sys.break_flag% Then
    con.println("Interrupted by user")
    gonzo.get_device$ = "DISCONNECTED"
  Else
    gonzo.get_device$ = LGetStr$(gonzo.buf%(), 1, LInStr(gonzo.buf%(), Chr$(10)) - 1)
  EndIf
End Function

Function gonzo.get_file_size%(file$)
  gonzo.break()
  gonzo.read_until_prompt()
  Local command$ = "? Str$(Mm.Info(FileSize " + str.quote$(file$) + "))"
  gonzo.writeln(command$)
  gonzo.read_until_match("> " + command$)
  LongString Clear gonzo.buf%()
  gonzo.read_until_prompt(COLLECT% Or SKIP_EMPTY%)
  gonzo.get_file_size% = Val(LGetStr$(gonzo.buf%(), 1, 255))
  Select Case gonzo.get_file_size%
    Case -2 : sys.err$ = "Remote error: is a directory"
    Case -1 : sys.err$ = "Remote error: file not found"
  End Select
End Function

' Reads a character from the serial port.
'
' @param  timeout  milliseconds to try before returning a -1.
' @return          ASCII code of read character, or -1 if no character available.
Function gonzo.getc%(timeout%)
  Local t% = Timer + timeout%
  Local ch$
  Do
    ch$ = Input$(1, #gonzo.SERIAL_FNBR%)
    If ch$ <> "" Then
      Print #gonzo.LOG_FNBR%, ch$;
      gonzo.getc% = Asc(ch$)
      Exit Function
    EndIf
    If timeout% > 0 Then Pause 1
  Loop Until Timer >= t%
  gonzo.getc% = -1
End Function

' Reads a line from the serial port.
'
' @param   s$  the read line is returned int this.
' @return  1   if something was read including an empty line, 0 if nothing was read.
Function gonzo.getln%(s$)
  Local ch%
  Do
    ch% = gonzo.getc%(500)
    If ch% > -1 Then
      ' TODO: handle both CR and LF
      If ch% = 10 Then
        s$ = gonzo.rx_buf$
        gonzo.rx_buf$ = ""
        gonzo.getln% = 1
        Exit Function
      ElseIf ch% <> 13 Then
        Cat gonzo.rx_buf$, Chr$(ch%)
      EndIf
    EndIf
  Loop Until Eof(#gonzo.SERIAL_FNBR%)
  s$ = ""
End Function

Sub gonzo.handle_press_any_key()
  If InStr(gonzo.rx_buf$, "PRESS ANY KEY ...") <> 1 Then Exit Sub
  gonzo.rx_buf$ = ""
  gonzo.write(" ")
  ' Ignore krud.
  Do While gonzo.getc%(500) <> &h0D : Loop
  Do While gonzo.getc%(500) <> &h0D : Loop
End Sub

Function gonzo.load_inifile%(f$)
  Local mp$(map.new%(10)), num%

  If file.exists%(f$) Then
    On Error Skip 1
    Open f$ For Input As #gonzo.INI_FNBR%
    If Mm.ErrMsg$ = "" Then
      gonzo.load_inifile% = inifile.read%(gonzo.INI_FNBR%, mp$(), num%)
      Close #gonzo.INI_FNBR%
    EndIf
    If Not gonzo.load_inifile% Then sys.err$ = "Error reading '" + f$ + "'"
  Else
    gonzo.load_inifile% = 1
  EndIf

  gonzo.comspec$ = map.get$(mp$(), "comspec")
  If gonzo.comspec$ = sys.NO_DATA$ Then gonzo.comspec$ = "/dev/ttyS0: 115200"
End Function

Sub gonzo.read_until_match(pattern$)
  Local s$, x%
  Do While Not sys.break_flag%
    x% = gonzo.getln%(s$)
    If file.fnmatch%(pattern$, s$) Then Exit Do
  Loop
End Sub

' Reads input until there is a command-prompt.
Sub gonzo.read_until_prompt(flags%)
  Const CMM2_MESS$ = Chr$(&h1B) + "[?25h" + Chr$(&h1B) + "[37m" + Chr$(&h1B) + "[m"
  Local x%, s$, i%
  Do While Not sys.break_flag%
    x% = gonzo.getln%(s$)
    If x% Then
      If InStr(s$, CMM2_MESS$) = 1 Then s$ = Mid$(s$, Len(CMM2_MESS$) + 1)
      If s$ = "" And (flags% And SKIP_EMPTY%) > 0 Then Continue Do
      If flags% And ECHO% Then con.println(s$)
      If flags% And COLLECT% Then LongString Append gonzo.buf%(), s$ + Chr$(&h0A)
    Else
      If Left$(gonzo.rx_buf$, 2) = "> " Then Exit Do
    EndIf
  Loop
End Sub

Function gonzo.save_inifile%(f$)
  Local mp$(map.new%(10)), num%
  map.put(mp$(), "comspec", gonzo.comspec$)

  On Error Skip 1
  Open f$ For Output As #gonzo.INI_FNBR%
  If Mm.ErrMsg$ = "" Then
    gonzo.save_inifile% = inifile.write%(gonzo.INI_FNBR%, mp$(), num%)
    Close #gonzo.INI_FNBR%
  EndIf
  If Not gonzo.save_inifile% Then sys.err$ = "Error writing '" + f$ + "' - " + Mm.ErrMsg$
End Function

Sub gonzo.write(s$)
  If Not sys.break_flag% Then Print #gonzo.SERIAL_FNBR%, s$;
End Sub

Sub gonzo.writeln(s$)
  ' Note we only send a LF '\n' and not a CRLF this prevents the remote from
  ' treating it as two characters and screwing up commands that show
  ' "PRESS ANY KEY ..." prompts.
  If Not sys.break_flag% Then Print #gonzo.SERIAL_FNBR%, s$ Chr$(&h0A);
End Sub

Function gonzo.usage$(cmd$)
  Restore gonzo_commands
  Local synonyms$, usage$, description$
  Do
    Read synonyms$, usage$, description$
    If synonyms$ = "<END>" Then Error "No usage information for: " + cmd$
    If InStr(synonyms$, "|" + LCase$(cmd$) + "|") Then
      gonzo.usage$ = "Usage: " + cmd$
      If usage$ <> "" Then Cat gonzo.usage$, " " + usage$
      Exit Do
    EndIf
  Loop
End Function

gonzo_commands:
Data "|break|",             "",                      "send a Ctrl-C to the remote"
Data "|connect|",           "",                      "connect to remote"
Data "|device|",            "",                      "report type of remote device"
Data "|exit|quit|",         "",                      "exit the gonzo shell"
Data "|files|dir|ls|",      "[<file|directory>]",    "list local directory contents"
Data "|help|",              "[<command>]",           "display help text"
Data "|log|",               "[clear]",               "show (or clear) receive log"
Data "|rchdir|rcd|",        "<directory>",           "change remote directory"
Data "|rcopy|rcp|",         "<src-file> <dst-file>", "copy a remote file"
Data "|rdelete|rkill|rrm|", "<file|directory>",      "delete a remote file or (empty) directory"
Data "|receive|get|",       "<file>",                "receive a file from the remote using AUTOSAVE"
Data "|rfiles|rdir|rls|",   "[<file|directory>]",    "list remote directory contents"
Data "|rmkdir|",            "<directory>",           "make a directory on the remote"
Data "|rpwd|",              "",                      "display current remote directory"
Data "|rrename|rmv|",       "<old-file> <new-file>", "rename a remote file or directory"
Data "|rsize|",             "<file>",                "display size of a remote file"
Data "|rtype|rcat|rlist|",  "<file>",                "list remote file contents"
Data "|send|put|",          "<file>",                "send a file to the remote using AUTOSAVE"
Data "|term|",              "",                      "open serial terminal"
Data "|version|",           "",                      "display current gonzo version"
Data "|xreceive|xget|",     "<file>",                "receive a file from the remote using XMODEM"
Data "|xsend|xput|",        "<file>",                "send a file from to the remote using XMODEM"
Data "<END>", "", ""

Sub dump(s$)
  Local i%
  For i% = 1 To Len(s$)
    con.print(Hex$(Asc(Mid$(s$, i%, 1)), 2) + " ")
  Next
End Sub
