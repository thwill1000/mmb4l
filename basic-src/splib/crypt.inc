' Copyright (c) 2021 Thomas Hugo Williams
' License MIT <https://opensource.org/licenses/MIT>
' For Colour Maximite 2, MMBasic 5.07

' MD5 algorithm from https://en.wikipedia.org/wiki/MD5
' XXTEA algorithm from https://en.wikipedia.org/wiki/XXTEA

'!remove_if NO_INCLUDE_GUARDS
On Error Skip 1 : Dim sys.VERSION$ = ""
If sys.VERSION$ = "" Then Error "'system.inc' not included"
sys.requires("array")
sys.provides("crypt")
If sys.err$ <> "" Then Error sys.err$
'!endif

' Calculates the MD5 hash of bytes held in a buffer.
'
' To calculate the MD5 hash of a string s$:
'   ok% = crypt.md5%(Peek(VarAddr s$) + 1, Len(s$), out%())
'
' To calculate the MD5 hash of a long string ls%():
'   ok% = crypt.md5%(Peek(VarAddr ls%()) + 8, LLen(ls%()), out%())
'
' @param  ad%     address of the buffer.
' @param  size%   number of bytes to process.
' @param  out%()  on return the MD5 hash in little-endian format (2 elements = 16 bytes = 128-bits)
' @return         1 on success, 0 on failure, see sys.err$ for details of failure.
Function crypt.md5%(ad%, size%, out%())

  If Bound(out%(), 1) - Bound(out%(), 0) <> 1 Then Error "out%() should have 2 elements"

  Local block%(array.new%(64))
  Const b_addr% = Peek(VarAddr block%())

  ' Storage for hash, each element treated as 32-bit unsigned integer.
  Local h%(array.new%(4))

  ' Storage for constants, each element treated as 32-bit unsigned integer.
  Local r%(array.new%(64))
  Local k%(array.new%(64))

  Local cur% = 0, num%

  Do
    ' Process blocks of 64 bytes / 512 bits at a time.
    num% = Min(size% - cur%, 64)
    Memory Set b_addr%, 0, 64
    Memory Copy ad% + cur%, b_addr%, num%

    If num% < 64 Then
      ' Append '1' bit to block%, the remainder will already be padded with zeros.
      Poke Byte b_addr% + num%, &h80

      If num% > 55 Then
        crypt.md5_block(block%(), h%(), r%(), k%())
        Memory Set b_addr%, 0, 64
      EndIf

      ' Last 64-bits of block% should be data length in bits as a 64-bit little-endian integer.
      Poke Integer b_addr% + 56, size% * 8
    EndIf

    crypt.md5_block(block%(), h%(), r%(), k%())

    Inc cur%, 64
  Loop While num% = 64

  Const BASE% = Mm.Info(Option Base)
  out%(BASE%)     = h%(BASE%)     Or h%(BASE% + 1) << 32
  out%(BASE% + 1) = h%(BASE% + 2) Or h%(BASE% + 3) << 32
  crypt.md5% = 1

End Function

' Formats 128-bit MD5 hash as a string.
'
' @param  md5%()  the MD5 hash in little-endian format (2 elements = 16 bytes = 128-bits)
' @return         the formatted MD5 hash.
Function crypt.md5_fmt$(md5%())
  If Bound(md5%(), 1) - Bound(md5%(), 0) <> 1 Then Error "md5%() should have 2 elements"
  Local i%, s$
  Const md5_addr% = Peek(VarAddr md5%())
  For i% = 0 To 15 : Cat s$, Hex$(Peek(Byte md5_addr% + i%), 2) : Next
  crypt.md5_fmt$ = LCase$(s$)
End Function

' Calculates the MD5 hash of a file.
'
' @param  fd%     file-descriptor for a file Open For Input.
' @param  out%()  on return the MD5 hash in little-endian format (2 elements = 16 bytes = 128-bits)
' @return         1 on success, 0 on failure, see sys.err$ for details of failure.
Function crypt.md5_file%(fd%, out%())

  If Bound(out%(), 1) - Bound(out%(), 0) <> 1 Then Error "out%() should have 2 elements"

  Local block%(array.new%(64))
  Const b_addr% = Peek(VarAddr block%())

  ' Storage for hash, each element treated as 32-bit unsigned integer.
  Local h%(array.new%(4))

  ' Storage for constants, each element treated as 32-bit unsigned integer.
  Local r%(array.new%(64))
  Local k%(array.new%(64))

  Local s$, num%, size%

  Do
    s$ = Input$(64, #fd%)
    num% = Len(s$)
    Inc size%, num%
    Memory Set b_addr%, 0, 64
    Memory Copy Peek(VarAddr s$) + 1, b_addr%, num%

    If num% < 64 Then
      ' Append '1' bit to block%, the remainder will already be padded with zeros.
      Poke Byte b_addr% + num%, &h80

      If num% > 55 Then
        crypt.md5_block(block%(), h%(), r%(), k%())
        Memory Set b_addr%, 0, 64
      EndIf

      ' Last 64-bits of block% should be file length in bits as a 64-bit little-endian integer.
      Poke Integer b_addr% + 56, size% * 8
    EndIf

    crypt.md5_block(block%(), h%(), r%(), k%())
  Loop While num% = 64

  Const BASE% = Mm.Info(Option Base)
  out%(BASE%)     = h%(BASE%)     Or h%(BASE% + 1) << 32
  out%(BASE% + 1) = h%(BASE% + 2) Or h%(BASE% + 3) << 32
  crypt.md5_file% = 1

End Function

' Process 512-bit block in MD5 hash calculation.
'
' @param  block%()  512-bit block to process (64 elements).
' @param  h%()      hash-values (4 elements).
' @param  r%()      per-round shift amounts (64 elements).
' @param  k%()      binary integer part of the sines of integers (Radians) as constants (64 elements).
Sub crypt.md5_block(block%(), h%(), r%(), k%())
  Const BASE% = Mm.Info(Option Base)

  ' Note that all variables should be treated as unsigned 32-bit integers
  ' and wrap modulo 2^32 when calculating.

  ' Initialise hash and constants if this is the first call.
  If r%(BASE%) = 0 Then
    Local data_addr% = Peek(CFunAddr crypt.md5_data)
    Memory Copy data_addr%,       Peek(VarAddr h%()), 64
    Memory Copy data_addr% + 32,  Peek(VarAddr r%()), 512
    Memory Copy data_addr% + 544, Peek(VarAddr k%()), 512
  EndIf

  ' Split block into 16 x 32-bit words.
  Local b_addr% = Peek(VarAddr block%())
  Local i%, w%(array.new%(16))
  For i% = 0 To 15
     w%(i% + BASE%) = Peek(Word b_addr% + 4 * i%)
  Next

  ' Main loop.
  Local a% = h%(BASE%), b% = h%(BASE% + 1), c% = h%(BASE% + 2), d% = h%(BASE% + 3), f%, g%, tmp%
  For i% = 0 To 63
    Select Case i%
      Case 0 To 15
        f% = d% Xor (b% And (c% Xor d%))
        g% = i%
      Case 16 To 31
        f% = c% Xor (d% And (b% Xor c%))
        g% = (5 * i% + 1) Mod 16
      Case 32 To 47
        f% = b% Xor c% Xor d%
        g% = (3 * i% + 5) Mod 16
      Case Else:
        f% = c% Xor (b% Or (d% Xor &hFFFFFFFF))
        g% = (7 * i%) Mod 16
    End Select

    tmp% = d%
    d% = c%
    c% = b%
    b% = (a% + f%) And &hFFFFFFFF
    b% = (b% + k%(i% + BASE%)) And &hFFFFFFFF
    b% = (b% + w%(g% + BASE%)) And &hFFFFFFFF
    b% = (b% << r%(i% + BASE%)) Or (b% >> (32 - r%(i% + BASE%)))
    b% = (b% + c%) And &hFFFFFFFF
    a% = tmp%

  Next

  ' Add this block's hash to the result.
  h%(BASE%)     = (h%(BASE%)     + a%) And &hFFFFFFFF
  h%(BASE% + 1) = (h%(BASE% + 1) + b%) And &hFFFFFFFF
  h%(BASE% + 2) = (h%(BASE% + 2) + c%) And &hFFFFFFFF
  h%(BASE% + 3) = (h%(BASE% + 3) + d%) And &hFFFFFFFF

End Sub

' Not a real CSUB, this provides data for initialising MD5 constants.
CSub crypt.md5_data()
  00000000

  ' 4 x 8 byte values to initialise hash 'h'
  67452301 00000000 EFCDAB89 00000000 98BADCFE 00000000 10325476 00000000

  ' 64 x 8 byte values for 'r'
  00000007 00000000 0000000C 00000000 00000011 00000000 00000016 00000000
  00000007 00000000 0000000C 00000000 00000011 00000000 00000016 00000000
  00000007 00000000 0000000C 00000000 00000011 00000000 00000016 00000000
  00000007 00000000 0000000C 00000000 00000011 00000000 00000016 00000000
  00000005 00000000 00000009 00000000 0000000E 00000000 00000014 00000000
  00000005 00000000 00000009 00000000 0000000E 00000000 00000014 00000000
  00000005 00000000 00000009 00000000 0000000E 00000000 00000014 00000000
  00000005 00000000 00000009 00000000 0000000E 00000000 00000014 00000000
  00000004 00000000 0000000B 00000000 00000010 00000000 00000017 00000000
  00000004 00000000 0000000B 00000000 00000010 00000000 00000017 00000000
  00000004 00000000 0000000B 00000000 00000010 00000000 00000017 00000000
  00000004 00000000 0000000B 00000000 00000010 00000000 00000017 00000000
  00000006 00000000 0000000A 00000000 0000000F 00000000 00000015 00000000
  00000006 00000000 0000000A 00000000 0000000F 00000000 00000015 00000000
  00000006 00000000 0000000A 00000000 0000000F 00000000 00000015 00000000
  00000006 00000000 0000000A 00000000 0000000F 00000000 00000015 00000000

  ' 64 x 8 byte values for 'k'
  D76AA478 00000000 E8C7B756 00000000 242070DB 00000000 C1BDCEEE 00000000
  F57C0FAF 00000000 4787C62A 00000000 A8304613 00000000 FD469501 00000000
  698098D8 00000000 8B44F7AF 00000000 FFFF5BB1 00000000 895CD7BE 00000000
  6B901122 00000000 FD987193 00000000 A679438E 00000000 49B40821 00000000
  F61E2562 00000000 C040B340 00000000 265E5A51 00000000 E9B6C7AA 00000000
  D62F105D 00000000 02441453 00000000 D8A1E681 00000000 E7D3FBC8 00000000
  21E1CDE6 00000000 C33707D6 00000000 F4D50D87 00000000 455A14ED 00000000
  A9E3E905 00000000 FCEFA3F8 00000000 676F02D9 00000000 8D2A4C8A 00000000
  FFFA3942 00000000 8771F681 00000000 6D9D6122 00000000 FDE5380C 00000000
  A4BEEA44 00000000 4BDECFA9 00000000 F6BB4B60 00000000 BEBFBC70 00000000
  289B7EC6 00000000 EAA127FA 00000000 D4EF3085 00000000 04881D05 00000000
  D9D4D039 00000000 E6DB99E5 00000000 1FA27CF8 00000000 C4AC5665 00000000
  F4292244 00000000 432AFF97 00000000 AB9423A7 00000000 FC93A039 00000000
  655B59C3 00000000 8F0CCC92 00000000 FFEFF47D 00000000 85845DD1 00000000
  6FA87E4F 00000000 FE2CE6E0 00000000 A3014314 00000000 4E0811A1 00000000
  F7537E82 00000000 BD3AF235 00000000 2AD7D2BB 00000000 EB86D391 00000000
End CSub

' Encrypts/decrypts file using XXTEA/CBC/PKCS#7 algorithm.
'
' @param  cmd$       command to perform, either "encrypt" or "decrypt".
' @param  in_fd%     reads input from this file descriptor.
' @param  out_fd%    writes output to this file descriptor.
' @param  k%()       encryption key (2 elements = 16 bytes = 128-bits).
' @param  iv%()      initialisation vector (2 elements = 16 bytes = 128-bits),
'                    this is ignored when cmd$ = "decrypt".
' @return            1 on success, 0 on failure, see sys.err$ for details of failure.
Function crypt.xxtea_file%(cmd$, in_fd%, out_fd%, k%(), iv%())

  If Bound(k%(), 1) - Bound(k%(), 0) <> 1 Then Error "k%() should have 2 elements"
  If Bound(iv%(), 1) - Bound(iv%(), 0) <> 1 Then Error "iv%() should have 2 elements"

  Local s$, num%
  Const s_addr% = Peek(VarAddr s$)
  Local block%(array.new%(2)) ' 16-bytes = 128-bits
  Const b_addr% = Peek(VarAddr block%())
  Local iv2%(array.new%(2)) ' 16-bytes = 128-bits
  Const iv2_addr% = Peek(VarAddr iv2%())

  If cmd$ = "encrypt" Then

    ' Write 128-bit initialisation vector.
    Poke Byte s_addr%, 16
    Memory Copy Peek(VarAddr iv%()), s_addr% + 1, 16
    Print #out_fd%, s$;

    ' iv2%() = iv%()
    Memory Copy Peek(VarAddr iv%()), iv2_addr%, 16

    Do

      ' Read 128-bit block.
      s$ = Input$(16, #in_fd%)
      num% = Len(s$)
      If num% > 0 Then Memory Copy s_addr% + 1, b_addr%, num%

      ' If we read a block of N bytes where N < 16 bytes long then we append PKCS#7
      ' padding of 16 - N bytes each of value 16 - N. In the event the input file
      ' size is an exact multiple of 16 then we append 16 bytes of value 16.
      If num% < 16 Then Memory Set b_addr% + num%, 16 - num%, 16 - num%

      ' Implement CBC by XORing iv2%() with block%().
      Poke Integer b_addr%, Peek(Integer b_addr%) Xor Peek(Integer iv2_addr%)
      Poke Integer b_addr% + 8, Peek(Integer b_addr% + 8) Xor Peek(Integer iv2_addr% + 8)

      ' Encrypt block.
      If Not crypt.xxtea_block%(cmd$, block%(), k%()) Then Exit Do

      ' Write 128-bit block.
      Poke Byte s_addr%, 16
      Memory Copy b_addr%, s_addr% + 1, 16
      Print #out_fd%, s$;

      ' iv2%() = block%()
      Memory Copy b_addr%, iv2_addr%, 16

    Loop Until num% < 16

  ElseIf cmd$ = "decrypt" Then

    Local iv3%(array.new%(2))
    Local iv3_addr% = Peek(VarAddr iv3%())

    ' Read 128-bit initialisation vector.
    s$ = Input$(16, #in_fd%)
    If Len(s$) < 16 Then
      sys.err$ = "encrypted file too short: " + Str$(Len(s$)) + " bytes"
      Exit Function
    EndIf
    Memory Copy s_addr% + 1, iv2_addr%, 16

    Do

      ' Read 128-bit block.
      s$ = Input$(16, #in_fd%)
      If Len(s$) < 16 Then
        sys.err$ = "encrypted file length not multiple of 16 bytes"
        Exit Function
      EndIf
      Memory Copy s_addr% + 1, b_addr%, 16

      ' iv3%() = block%(), which we will need for the next iteration.
      Memory Copy b_addr%, iv3_addr%, 16

      ' Decrypt block.
      If Not crypt.xxtea_block%(cmd$, block%(), k%()) Then Exit Function

      ' Implement CBC by XORing iv2%() with block%().
      Poke Integer b_addr%, Peek(Integer b_addr%) Xor Peek(Integer iv2_addr%)
      Poke Integer b_addr% + 8, Peek(Integer b_addr% + 8) Xor Peek(Integer iv2_addr% + 8)

      ' iv2%() = iv3%()
      Memory Copy iv3_addr%, iv2_addr%, 16

      ' If we've reached the EOF then we do not want to write the entire
      ' block, the last byte in the block will indicate the number of bytes
      ' we should ignore, which may be the entire block.
      num% = Choice(Eof(#in_fd%), 16 - Peek(Byte b_addr% + 15), 16)

      ' Write block.
      Poke Byte s_addr%, num%
      If num% > 0 Then Memory Copy b_addr%, s_addr% + 1, num%
      Print #out_fd%, s$;

    Loop Until num% < 16

  Else

    Error "Unknown encryption command: " + cmd$

  EndIf

  crypt.xxtea_file% = 1

End Function

' Encrypts/decrypts block using XXTEA algorithm.
'
' @param  cmd$      command to perform, either "encrypt" or "decrypt".
' @param  block%()  data to encrypted, interpreted as 32-bit words.
' @param  k%()      encryption key (2 elements = 16 bytes = 128-bits).
' @return           1 on success, 0 on failure, see sys.err$ for details of failure.
Function crypt.xxtea_block%(cmd$, block%(), k%())
  Const DELTA% = &h9e3779b9 ' 4-bytes
  Const b_addr% = Peek(VarAddr block%())
  Const k_addr% = Peek(VarAddr k%())
  Const N% = 2 * (Bound(block%(), 1) - Bound(block%(), 0) + 1)
  Local rounds% = 6 + 52 \ N%
  Local sum%, y%, z%, e%, p%

  If cmd$ = "encrypt" Then

    sum% = 0
    y% = 0
    z% = Peek(Word b_addr% + 4 * (N% - 1))

    Do
      sum% = (sum% + DELTA%) And &hFFFFFFFF
      e% = (sum% >> 2) And 3
      For p% = 0 To N% - 2
        y% = Peek(Word b_addr% + 4 * (p% + 1))
        z% = (Peek(Word b_addr% + 4 * p%) + crypt.mx%(y%, z%, sum%, k_addr%, p%, e%)) And &hFFFFFFFF
        Poke Word b_addr% + 4 * p%, z%
      Next
      y% = Peek(Word b_addr%)
      z% = (Peek(Word b_addr% + 4*(N%-1)) + crypt.mx%(y%, z%, sum%, k_addr%, p%, e%)) And &hFFFFFFFF
      Poke Word b_addr% + 4 * (N% - 1), z%
      Inc rounds%, -1
    Loop While rounds%

  ElseIf cmd$ = "decrypt" Then

    sum% = rounds% * DELTA%
    y% = Peek(Word b_addr%)
    z% = 0

    Do
      e% = (sum% >> 2) And 3
      For p% = N% - 1 To 1 Step -1
        z% = Peek(Word b_addr% + 4 * (p% - 1))
        y% = (Peek(Word b_addr% + 4 * p%) - crypt.mx%(y%, z%, sum%, k_addr%, p%, e%)) And &hFFFFFFFF
        Poke Word b_addr% + 4 * p%, y%
      Next
      z% = Peek(Word b_addr% + 4 * (N% - 1))
      y% = (Peek(Word b_addr%) - crypt.mx%(y%, z%, sum%, k_addr%, p%, e%)) And &hFFFFFFFF
      Poke Word b_addr%, y%
      sum% = (sum% - DELTA%) And &hFFFFFFFF
      Inc rounds%, -1
    Loop While rounds%

  Else

    Error "Unknown encryption command: " + cmd$

  EndIf

  crypt.xxtea_block% = 1
End Function

Function crypt.mx%(y%, z%, sum%, k_addr%, p%, e%)
  Local lhs% = (z% >> 5) Xor ((y% << 2) And &hFFFFFFFF)
  Inc lhs%, (y% >> 3) Xor ((z% << 4) And &hFFFFFFFF)
  Local rhs% = (sum% Xor y%) + (Peek(Word k_addr% + 4 * ((p% And 3) Xor e%)) Xor z%)
  crypt.mx% = (lhs% And &hFFFFFFFF) Xor (rhs% And &hFFFFFFFF)
End Function
