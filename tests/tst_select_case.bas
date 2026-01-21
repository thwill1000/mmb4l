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

add_test("test_select_case")
add_test("test_select_given_string_key")
add_test("test_select_given_int_key")
add_test("test_errors")

If InStr(Mm.CmdLine$, "--base") Then run_tests() Else run_tests("--base=1")

End

Sub test_select_case()
  assert_string_equals("DD", select_case$(0))
  assert_string_equals("AA", select_case$(1))
  assert_string_equals("BB", select_case$(2))
  assert_string_equals("BB", select_case$(3))
  assert_string_equals("BB", select_case$(4))
  assert_string_equals("CC", select_case$(5))
  assert_string_equals("CC", select_case$(6))
  assert_string_equals("CC", select_case$(7))
  assert_string_equals("DD", select_case$(8))
  assert_string_equals("DD", select_case$(9))
  assert_string_equals("EE", select_case$(10))
  assert_string_equals("FF", select_case$(11))
  assert_string_equals("GG", select_case$(12))
  assert_string_equals("HH", select_case$(13))
  assert_string_equals("HH", select_case$(14))
  assert_string_equals("II", select_case$(15))
End Sub

Function select_case$(i%)
  Select Case i%
    Case 1
      select_case$ = "AA"
    Case 2 To 4
      select_case$ = "BB"
    Case 5, 6, 7
      select_case$ = "CC"
    Case Is < 10
      select_case$ = "DD"
    Case 10 To 14
      Select Case i%
        Case 10: select_case$ = "EE"
        Case 11: select_case$ = "FF"
        Case < 13 : select_case$ = "GG"
        Case Else : select_case$ = "HH"
      End Select
    Case Else
      select_case$ = "II"
  End Select
End Function

Sub test_select_given_string_key()
  assert_string_equals("005", select_string$("case005"))
  assert_string_equals("050", select_string$("case050"))
  assert_string_equals("100", select_string$("case100"))
  assert_string_equals("150", select_string$("case150"))
  assert_string_equals("199", select_string$("case199"))
  assert_string_equals("256", select_string$("case256"))
  assert_string_equals("ELSE", select_string$("nomatch"))

  ' Stress test
  Local i%
  For i% = 1 To 1000
    assert_string_equals("100", select_string$("case100"))
  Next
End Sub

Function select_string$(key$)
  Select Case key$
    Case "case001": select_string$ = "001"
    Case "case002": select_string$ = "002"
    Case "case003": select_string$ = "003"
    Case "case004": select_string$ = "004"
    Case "case005": select_string$ = "005"
    Case "case006": select_string$ = "006"
    Case "case007": select_string$ = "007"
    Case "case008": select_string$ = "008"
    Case "case009": select_string$ = "009"
    Case "case010": select_string$ = "010"
    Case "case011": select_string$ = "011"
    Case "case012": select_string$ = "012"
    Case "case013": select_string$ = "013"
    Case "case014": select_string$ = "014"
    Case "case015": select_string$ = "015"
    Case "case016": select_string$ = "016"
    Case "case017": select_string$ = "017"
    Case "case018": select_string$ = "018"
    Case "case019": select_string$ = "019"
    Case "case020": select_string$ = "020"
    Case "case021": select_string$ = "021"
    Case "case022": select_string$ = "022"
    Case "case023": select_string$ = "023"
    Case "case024": select_string$ = "024"
    Case "case025": select_string$ = "025"
    Case "case026": select_string$ = "026"
    Case "case027": select_string$ = "027"
    Case "case028": select_string$ = "028"
    Case "case029": select_string$ = "029"
    Case "case030": select_string$ = "030"
    Case "case031": select_string$ = "031"
    Case "case032": select_string$ = "032"
    Case "case033": select_string$ = "033"
    Case "case034": select_string$ = "034"
    Case "case035": select_string$ = "035"
    Case "case036": select_string$ = "036"
    Case "case037": select_string$ = "037"
    Case "case038": select_string$ = "038"
    Case "case039": select_string$ = "039"
    Case "case040": select_string$ = "040"
    Case "case041": select_string$ = "041"
    Case "case042": select_string$ = "042"
    Case "case043": select_string$ = "043"
    Case "case044": select_string$ = "044"
    Case "case045": select_string$ = "045"
    Case "case046": select_string$ = "046"
    Case "case047": select_string$ = "047"
    Case "case048": select_string$ = "048"
    Case "case049": select_string$ = "049"
    Case "case050": select_string$ = "050"
    Case "case051": select_string$ = "051"
    Case "case052": select_string$ = "052"
    Case "case053": select_string$ = "053"
    Case "case054": select_string$ = "054"
    Case "case055": select_string$ = "055"
    Case "case056": select_string$ = "056"
    Case "case057": select_string$ = "057"
    Case "case058": select_string$ = "058"
    Case "case059": select_string$ = "059"
    Case "case060": select_string$ = "060"
    Case "case061": select_string$ = "061"
    Case "case062": select_string$ = "062"
    Case "case063": select_string$ = "063"
    Case "case064": select_string$ = "064"
    Case "case065": select_string$ = "065"
    Case "case066": select_string$ = "066"
    Case "case067": select_string$ = "067"
    Case "case068": select_string$ = "068"
    Case "case069": select_string$ = "069"
    Case "case070": select_string$ = "070"
    Case "case071": select_string$ = "071"
    Case "case072": select_string$ = "072"
    Case "case073": select_string$ = "073"
    Case "case074": select_string$ = "074"
    Case "case075": select_string$ = "075"
    Case "case076": select_string$ = "076"
    Case "case077": select_string$ = "077"
    Case "case078": select_string$ = "078"
    Case "case079": select_string$ = "079"
    Case "case080": select_string$ = "080"
    Case "case081": select_string$ = "081"
    Case "case082": select_string$ = "082"
    Case "case083": select_string$ = "083"
    Case "case084": select_string$ = "084"
    Case "case085": select_string$ = "085"
    Case "case086": select_string$ = "086"
    Case "case087": select_string$ = "087"
    Case "case088": select_string$ = "088"
    Case "case089": select_string$ = "089"
    Case "case090": select_string$ = "090"
    Case "case091": select_string$ = "091"
    Case "case092": select_string$ = "092"
    Case "case093": select_string$ = "093"
    Case "case094": select_string$ = "094"
    Case "case095": select_string$ = "095"
    Case "case096": select_string$ = "096"
    Case "case097": select_string$ = "097"
    Case "case098": select_string$ = "098"
    Case "case099": select_string$ = "099"
    Case "case100": select_string$ = "100"
    Case "case101": select_string$ = "101"
    Case "case102": select_string$ = "102"
    Case "case103": select_string$ = "103"
    Case "case104": select_string$ = "104"
    Case "case105": select_string$ = "105"
    Case "case106": select_string$ = "106"
    Case "case107": select_string$ = "107"
    Case "case108": select_string$ = "108"
    Case "case109": select_string$ = "109"
    Case "case110": select_string$ = "110"
    Case "case111": select_string$ = "111"
    Case "case112": select_string$ = "112"
    Case "case113": select_string$ = "113"
    Case "case114": select_string$ = "114"
    Case "case115": select_string$ = "115"
    Case "case116": select_string$ = "116"
    Case "case117": select_string$ = "117"
    Case "case118": select_string$ = "118"
    Case "case119": select_string$ = "119"
    Case "case120": select_string$ = "120"
    Case "case121": select_string$ = "121"
    Case "case122": select_string$ = "122"
    Case "case123": select_string$ = "123"
    Case "case124": select_string$ = "124"
    Case "case125": select_string$ = "125"
    Case "case126": select_string$ = "126"
    Case "case127": select_string$ = "127"
    Case "case128": select_string$ = "128"
    Case "case129": select_string$ = "129"
    Case "case130": select_string$ = "130"
    Case "case131": select_string$ = "131"
    Case "case132": select_string$ = "132"
    Case "case133": select_string$ = "133"
    Case "case134": select_string$ = "134"
    Case "case135": select_string$ = "135"
    Case "case136": select_string$ = "136"
    Case "case137": select_string$ = "137"
    Case "case138": select_string$ = "138"
    Case "case139": select_string$ = "139"
    Case "case140": select_string$ = "140"
    Case "case141": select_string$ = "141"
    Case "case142": select_string$ = "142"
    Case "case143": select_string$ = "143"
    Case "case144": select_string$ = "144"
    Case "case145": select_string$ = "145"
    Case "case146": select_string$ = "146"
    Case "case147": select_string$ = "147"
    Case "case148": select_string$ = "148"
    Case "case149": select_string$ = "149"
    Case "case150": select_string$ = "150"
    Case "case151": select_string$ = "151"
    Case "case152": select_string$ = "152"
    Case "case153": select_string$ = "153"
    Case "case154": select_string$ = "154"
    Case "case155": select_string$ = "155"
    Case "case156": select_string$ = "156"
    Case "case157": select_string$ = "157"
    Case "case158": select_string$ = "158"
    Case "case159": select_string$ = "159"
    Case "case160": select_string$ = "160"
    Case "case161": select_string$ = "161"
    Case "case162": select_string$ = "162"
    Case "case163": select_string$ = "163"
    Case "case164": select_string$ = "164"
    Case "case165": select_string$ = "165"
    Case "case166": select_string$ = "166"
    Case "case167": select_string$ = "167"
    Case "case168": select_string$ = "168"
    Case "case169": select_string$ = "169"
    Case "case170": select_string$ = "170"
    Case "case171": select_string$ = "171"
    Case "case172": select_string$ = "172"
    Case "case173": select_string$ = "173"
    Case "case174": select_string$ = "174"
    Case "case175": select_string$ = "175"
    Case "case176": select_string$ = "176"
    Case "case177": select_string$ = "177"
    Case "case178": select_string$ = "178"
    Case "case179": select_string$ = "179"
    Case "case180": select_string$ = "180"
    Case "case181": select_string$ = "181"
    Case "case182": select_string$ = "182"
    Case "case183": select_string$ = "183"
    Case "case184": select_string$ = "184"
    Case "case185": select_string$ = "185"
    Case "case186": select_string$ = "186"
    Case "case187": select_string$ = "187"
    Case "case188": select_string$ = "188"
    Case "case189": select_string$ = "189"
    Case "case190": select_string$ = "190"
    Case "case191": select_string$ = "191"
    Case "case192": select_string$ = "192"
    Case "case193": select_string$ = "193"
    Case "case194": select_string$ = "194"
    Case "case195": select_string$ = "195"
    Case "case196": select_string$ = "196"
    Case "case197": select_string$ = "197"
    Case "case198": select_string$ = "198"
    Case "case199": select_string$ = "199"
    Case "case200": select_string$ = "200"
    Case "case201": select_string$ = "201"
    Case "case202": select_string$ = "202"
    Case "case203": select_string$ = "203"
    Case "case204": select_string$ = "204"
    Case "case205": select_string$ = "205"
    Case "case206": select_string$ = "206"
    Case "case207": select_string$ = "207"
    Case "case208": select_string$ = "208"
    Case "case209": select_string$ = "209"
    Case "case210": select_string$ = "210"
    Case "case211": select_string$ = "211"
    Case "case212": select_string$ = "212"
    Case "case213": select_string$ = "213"
    Case "case214": select_string$ = "214"
    Case "case215": select_string$ = "215"
    Case "case216": select_string$ = "216"
    Case "case217": select_string$ = "217"
    Case "case218": select_string$ = "218"
    Case "case219": select_string$ = "219"
    Case "case220": select_string$ = "220"
    Case "case221": select_string$ = "221"
    Case "case222": select_string$ = "222"
    Case "case223": select_string$ = "223"
    Case "case224": select_string$ = "224"
    Case "case225": select_string$ = "225"
    Case "case226": select_string$ = "226"
    Case "case227": select_string$ = "227"
    Case "case228": select_string$ = "228"
    Case "case229": select_string$ = "229"
    Case "case230": select_string$ = "230"
    Case "case231": select_string$ = "231"
    Case "case232": select_string$ = "232"
    Case "case233": select_string$ = "233"
    Case "case234": select_string$ = "234"
    Case "case235": select_string$ = "235"
    Case "case236": select_string$ = "236"
    Case "case237": select_string$ = "237"
    Case "case238": select_string$ = "238"
    Case "case239": select_string$ = "239"
    Case "case240": select_string$ = "240"
    Case "case241": select_string$ = "241"
    Case "case242": select_string$ = "242"
    Case "case243": select_string$ = "243"
    Case "case244": select_string$ = "244"
    Case "case245": select_string$ = "245"
    Case "case246": select_string$ = "246"
    Case "case247": select_string$ = "247"
    Case "case248": select_string$ = "248"
    Case "case249": select_string$ = "249"
    Case "case250": select_string$ = "250"
    Case "case251": select_string$ = "251"
    Case "case252": select_string$ = "252"
    Case "case253": select_string$ = "253"
    Case "case254": select_string$ = "254"
    Case "case255": select_string$ = "255"
    Case "case256": select_string$ = "256"
    Case Else: select_string$ = "ELSE"
  End Select
End Function

Sub test_select_given_int_key()
  assert_int_equals(5, select_int%(5))
  assert_int_equals(50, select_int%(50))
  assert_int_equals(100, select_int%(100))
  assert_int_equals(150, select_int%(150))
  assert_int_equals(199, select_int%(199))
  assert_int_equals(-1, select_int%(1000))

  ' Stress test
  Local i%
  For i% = 1 To 1000
    assert_int_equals(100, select_int%(100)))
  Next
End Sub

Function select_int%(key%)
  Select Case key%
    Case 1: select_int% = 1
    Case 2: select_int% = 2
    Case 3: select_int% = 3
    Case 4: select_int% = 4
    Case 5: select_int% = 5
    Case 6: select_int% = 6
    Case 7: select_int% = 7
    Case 8: select_int% = 8
    Case 9: select_int% = 9
    Case 10: select_int% = 10
    Case 11: select_int% = 11
    Case 12: select_int% = 12
    Case 13: select_int% = 13
    Case 14: select_int% = 14
    Case 15: select_int% = 15
    Case 16: select_int% = 16
    Case 17: select_int% = 17
    Case 18: select_int% = 18
    Case 19: select_int% = 19
    Case 20: select_int% = 20
    Case 21: select_int% = 21
    Case 22: select_int% = 22
    Case 23: select_int% = 23
    Case 24: select_int% = 24
    Case 25: select_int% = 25
    Case 26: select_int% = 26
    Case 27: select_int% = 27
    Case 28: select_int% = 28
    Case 29: select_int% = 29
    Case 30: select_int% = 30
    Case 31: select_int% = 31
    Case 32: select_int% = 32
    Case 33: select_int% = 33
    Case 34: select_int% = 34
    Case 35: select_int% = 35
    Case 36: select_int% = 36
    Case 37: select_int% = 37
    Case 38: select_int% = 38
    Case 39: select_int% = 39
    Case 40: select_int% = 40
    Case 41: select_int% = 41
    Case 42: select_int% = 42
    Case 43: select_int% = 43
    Case 44: select_int% = 44
    Case 45: select_int% = 45
    Case 46: select_int% = 46
    Case 47: select_int% = 47
    Case 48: select_int% = 48
    Case 49: select_int% = 49
    Case 50: select_int% = 50
    Case 51: select_int% = 51
    Case 52: select_int% = 52
    Case 53: select_int% = 53
    Case 54: select_int% = 54
    Case 55: select_int% = 55
    Case 56: select_int% = 56
    Case 57: select_int% = 57
    Case 58: select_int% = 58
    Case 59: select_int% = 59
    Case 60: select_int% = 60
    Case 61: select_int% = 61
    Case 62: select_int% = 62
    Case 63: select_int% = 63
    Case 64: select_int% = 64
    Case 65: select_int% = 65
    Case 66: select_int% = 66
    Case 67: select_int% = 67
    Case 68: select_int% = 68
    Case 69: select_int% = 69
    Case 70: select_int% = 70
    Case 71: select_int% = 71
    Case 72: select_int% = 72
    Case 73: select_int% = 73
    Case 74: select_int% = 74
    Case 75: select_int% = 75
    Case 76: select_int% = 76
    Case 77: select_int% = 77
    Case 78: select_int% = 78
    Case 79: select_int% = 79
    Case 80: select_int% = 80
    Case 81: select_int% = 81
    Case 82: select_int% = 82
    Case 83: select_int% = 83
    Case 84: select_int% = 84
    Case 85: select_int% = 85
    Case 86: select_int% = 86
    Case 87: select_int% = 87
    Case 88: select_int% = 88
    Case 89: select_int% = 89
    Case 90: select_int% = 90
    Case 91: select_int% = 91
    Case 92: select_int% = 92
    Case 93: select_int% = 93
    Case 94: select_int% = 94
    Case 95: select_int% = 95
    Case 96: select_int% = 96
    Case 97: select_int% = 97
    Case 98: select_int% = 98
    Case 99: select_int% = 99
    Case 100: select_int% = 100
    Case 101: select_int% = 101
    Case 102: select_int% = 102
    Case 103: select_int% = 103
    Case 104: select_int% = 104
    Case 105: select_int% = 105
    Case 106: select_int% = 106
    Case 107: select_int% = 107
    Case 108: select_int% = 108
    Case 109: select_int% = 109
    Case 110: select_int% = 110
    Case 111: select_int% = 111
    Case 112: select_int% = 112
    Case 113: select_int% = 113
    Case 114: select_int% = 114
    Case 115: select_int% = 115
    Case 116: select_int% = 116
    Case 117: select_int% = 117
    Case 118: select_int% = 118
    Case 119: select_int% = 119
    Case 120: select_int% = 120
    Case 121: select_int% = 121
    Case 122: select_int% = 122
    Case 123: select_int% = 123
    Case 124: select_int% = 124
    Case 125: select_int% = 125
    Case 126: select_int% = 126
    Case 127: select_int% = 127
    Case 128: select_int% = 128
    Case 129: select_int% = 129
    Case 130: select_int% = 130
    Case 131: select_int% = 131
    Case 132: select_int% = 132
    Case 133: select_int% = 133
    Case 134: select_int% = 134
    Case 135: select_int% = 135
    Case 136: select_int% = 136
    Case 137: select_int% = 137
    Case 138: select_int% = 138
    Case 139: select_int% = 139
    Case 140: select_int% = 140
    Case 141: select_int% = 141
    Case 142: select_int% = 142
    Case 143: select_int% = 143
    Case 144: select_int% = 144
    Case 145: select_int% = 145
    Case 146: select_int% = 146
    Case 147: select_int% = 147
    Case 148: select_int% = 148
    Case 149: select_int% = 149
    Case 150: select_int% = 150
    Case 151: select_int% = 151
    Case 152: select_int% = 152
    Case 153: select_int% = 153
    Case 154: select_int% = 154
    Case 155: select_int% = 155
    Case 156: select_int% = 156
    Case 157: select_int% = 157
    Case 158: select_int% = 158
    Case 159: select_int% = 159
    Case 160: select_int% = 160
    Case 161: select_int% = 161
    Case 162: select_int% = 162
    Case 163: select_int% = 163
    Case 164: select_int% = 164
    Case 165: select_int% = 165
    Case 166: select_int% = 166
    Case 167: select_int% = 167
    Case 168: select_int% = 168
    Case 169: select_int% = 169
    Case 170: select_int% = 170
    Case 171: select_int% = 171
    Case 172: select_int% = 172
    Case 173: select_int% = 173
    Case 174: select_int% = 174
    Case 175: select_int% = 175
    Case 176: select_int% = 176
    Case 177: select_int% = 177
    Case 178: select_int% = 178
    Case 179: select_int% = 179
    Case 180: select_int% = 180
    Case 181: select_int% = 181
    Case 182: select_int% = 182
    Case 183: select_int% = 183
    Case 184: select_int% = 184
    Case 185: select_int% = 185
    Case 186: select_int% = 186
    Case 187: select_int% = 187
    Case 188: select_int% = 188
    Case 189: select_int% = 189
    Case 190: select_int% = 190
    Case 191: select_int% = 191
    Case 192: select_int% = 192
    Case 193: select_int% = 193
    Case 194: select_int% = 194
    Case 195: select_int% = 195
    Case 196: select_int% = 196
    Case 197: select_int% = 197
    Case 198: select_int% = 198
    Case 199: select_int% = 199
    Case 200: select_int% = 200
    Case 201: select_int% = 201
    Case 202: select_int% = 202
    Case 203: select_int% = 203
    Case 204: select_int% = 204
    Case 205: select_int% = 205
    Case 206: select_int% = 206
    Case 207: select_int% = 207
    Case 208: select_int% = 208
    Case 209: select_int% = 209
    Case 210: select_int% = 210
    Case 211: select_int% = 211
    Case 212: select_int% = 212
    Case 213: select_int% = 213
    Case 214: select_int% = 214
    Case 215: select_int% = 215
    Case 216: select_int% = 216
    Case 217: select_int% = 217
    Case 218: select_int% = 218
    Case 219: select_int% = 219
    Case 220: select_int% = 220
    Case 221: select_int% = 221
    Case 222: select_int% = 222
    Case 223: select_int% = 223
    Case 224: select_int% = 224
    Case 225: select_int% = 225
    Case 226: select_int% = 226
    Case 227: select_int% = 227
    Case 228: select_int% = 228
    Case 229: select_int% = 229
    Case 230: select_int% = 230
    Case 231: select_int% = 231
    Case 232: select_int% = 232
    Case 233: select_int% = 233
    Case 234: select_int% = 234
    Case 235: select_int% = 235
    Case 236: select_int% = 236
    Case 237: select_int% = 237
    Case 238: select_int% = 238
    Case 239: select_int% = 239
    Case 240: select_int% = 240
    Case 241: select_int% = 241
    Case 242: select_int% = 242
    Case 243: select_int% = 243
    Case 244: select_int% = 244
    Case 245: select_int% = 245
    Case 246: select_int% = 246
    Case 247: select_int% = 247
    Case 248: select_int% = 248
    Case 249: select_int% = 249
    Case 250: select_int% = 250
    Case 251: select_int% = 251
    Case 252: select_int% = 252
    Case 253: select_int% = 253
    Case 254: select_int% = 254
    Case 255: select_int% = 255
    Case 256: select_int% = 256
    Case Else: select_int% = -1
  End Select
End Function

Sub test_errors()
  On Error Ignore
  Select Case 5
  assert_raw_error("No matching END SELECT")
  On Error Abort
End Sub
