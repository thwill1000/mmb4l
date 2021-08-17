  ' lunar_eclipse.bas         March 15, 2017

  ' local circumstances of lunar eclipses

  ' Micromite eXtreme version

  '''''''''''''''''''''''''''
Timer = 0

  Option default float

  Option base 1

  ' dimension global arrays and variables

  Dim xsl(50), xsr(50), xsa(50), xsb(50)

  Dim jdleap(28), leapsec(28)

  Dim month$(12) As string

  Dim xnut(11, 13), trr, elev_minima

  Dim jdsaved, jdprint, obslat, obslong, obsalt

  Dim jdtdbi, i As integer, j As integer

  Dim cmonth, cday, cyear, ndays, obliq

  Dim psi, pmoon, psun, sdsun, sdmoon

  ' global constants

  Const pi2 = 2.0 * Pi, pidiv2 = 0.5 * Pi, dtr = Pi / 180.0, rtd = 180.0 / Pi

  Const atr = Pi / 648000.0, seccon = 206264.8062470964

  ' astronomical unit (kilometers)

  Const aunit = 149597870.691

  ' equatorial radius of the earth (au)

  Const reqm = 6378.14 / aunit

  ' earth flattening factor (nd)

  Const flat = 1.0 / 298.257

  ' radius of the sun (au)

  Const radsun = 696000.0 / aunit

  ' radius of the moon (au)

  Const radmoon = 1738.0 / aunit

  ' read solar ephemeris data

  For i = 1 To 50

    Read xsl(i), xsr(i), xsa(i), xsb(i)

  Next i

  Data 403406,      0, 4.721964,      1.621043
  Data 195207, -97597, 5.937458,  62830.348067
  Data 119433, -59715, 1.115589,  62830.821524
  Data 112392, -56188, 5.781616,  62829.634302
  Data   3891,  -1556, 5.5474 , 125660.5691
  Data   2819,  -1126, 1.5120 , 125660.9845
  Data   1721,   -861, 4.1897 ,  62832.4766
  Data      0,    941, 1.163  ,      0.813
  Data    660,   -264, 5.415  , 125659.310
  Data    350,   -163, 4.315  ,  57533.850
  Data    334,      0, 4.553  ,    -33.931
  Data    314,    309, 5.198  , 777137.715
  Data    268,   -158, 5.989  ,  78604.191
  Data    242,      0, 2.911  ,      5.412
  Data    234,    -54, 1.423  ,  39302.098
  Data    158,      0, 0.061  ,    -34.861
  Data    132,    -93, 2.317  , 115067.698
  Data    129,    -20, 3.193  ,  15774.337
  Data    114,      0, 2.828  ,   5296.670
  Data     99,    -47, 0.52   ,  58849.27
  Data     93,      0, 4.65   ,   5296.11
  Data     86,      0, 4.35   ,  -3980.70
  Data     78,    -33, 2.75   ,  52237.69
  Data     72,    -32, 4.50   ,  55076.47
  Data     68,      0, 3.23   ,    261.08
  Data     64,    -10, 1.22   ,  15773.85
  Data     46,    -16, 0.14   , 188491.03
  Data     38,      0, 3.44   ,  -7756.55
  Data     37,      0, 4.37   ,    264.89
  Data     32,    -24, 1.14   , 117906.27
  Data     29,    -13, 2.84   ,  55075.75
  Data     28,      0, 5.96   ,  -7961.39
  Data     27,     -9, 5.09   , 188489.81
  Data     27,      0, 1.72   ,   2132.19
  Data     25,    -17, 2.56   , 109771.03
  Data     24,    -11, 1.92   ,  54868.56
  Data     21,      0, 0.09   ,  25443.93
  Data     21,     31, 5.98   , -55731.43
  Data     20,    -10, 4.03   ,  60697.74
  Data     18,      0, 4.27   ,   2132.79
  Data     17,    -12, 0.79   , 109771.63
  Data     14,      0, 4.24   ,  -7752.82
  Data     13,     -5, 2.01   , 188491.91
  Data     13,      0, 2.65   ,    207.81
  Data     13,      0, 4.98   ,  29424.63
  Data     12,      0, 0.93   ,     -7.99
  Data     10,      0, 2.21   ,  46941.14
  Data     10,      0, 3.59   ,    -68.29
  Data     10,      0, 1.50   ,  21463.25
  Data     10,     -9, 2.55   , 157208.40

  ' read subset of IAU2000 nutation data

  For j = 1 To 13

    For i = 1 To 11

      Read xnut(i, j)

    Next i

  Next j

  Data  0,  0, 0,  0, 1, -172064161, -174666, 92052331,  9086,  33386, 15377
  Data  0,  0, 2, -2, 2,  -13170906,   -1675,  5730336, -3015, -13696, -4587
  Data  0,  0, 2,  0, 2,   -2276413,    -234,   978459,  -485,   2796,  1374
  Data  0,  0, 0,  0, 2,    2074554,     207,  -897492,   470,   -698,  -291
  Data  0,  1, 0,  0, 0,    1475877,   -3633,    73871,  -184,  11817, -1924
  Data  0,  1, 2, -2, 2,    -516821,    1226,   224386,  -677,   -524,  -174
  Data  1,  0, 0,  0, 0,     711159,      73,    -6750,     0,   -872,   358
  Data  0,  0, 2,  0, 1,    -387298,    -367,   200728,    18,    380,   318
  Data  1,  0, 2,  0, 2,    -301461,     -36,   129025,   -63,    816,   367
  Data  0, -1, 2, -2, 2,     215829,    -494,   -95929,   299,    111,   132
  Data  0,  0, 2, -2, 1,     128227,     137,   -68982,    -9,    181,    39
  Data -1,  0, 2,  0, 2,     123457,      11,   -53311,    32,     19,    -4
  Data -1,  0, 0,  2, 0,     156994,      10,    -1235,     0,   -168,    82

  ' read leap second data

  For i = 1 To 28

    Read jdleap(i), leapsec(i)

  Next i

  Data 2441317.5,  10.0
  Data 2441499.5,  11.0
  Data 2441683.5,  12.0
  Data 2442048.5,  13.0
  Data 2442413.5,  14.0
  Data 2442778.5,  15.0
  Data 2443144.5,  16.0
  Data 2443509.5,  17.0
  Data 2443874.5,  18.0
  Data 2444239.5,  19.0
  Data 2444786.5,  20.0
  Data 2445151.5,  21.0
  Data 2445516.5,  22.0
  Data 2446247.5,  23.0
  Data 2447161.5,  24.0
  Data 2447892.5,  25.0
  Data 2448257.5,  26.0
  Data 2448804.5,  27.0
  Data 2449169.5,  28.0
  Data 2449534.5,  29.0
  Data 2450083.5,  30.0
  Data 2450630.5,  31.0
  Data 2451179.5,  32.0
  Data 2453736.5,  33.0
  Data 2454832.5,  34.0
  Data 2456109.5,  35.0
  Data 2457204.5,  36.0
  Data 2457754.5,  37.0

  ' calendar months

  month$(1) = "January"
  month$(2) = "February"
  month$(3) = "March"
  month$(4) = "April"
  month$(5) = "May"
  month$(6) = "June"
  month$(7) = "July"
  month$(8) = "August"
  month$(9) = "September"
  month$(10) = "October"
  month$(11) = "November"
  month$(12) = "December"

  Print " "
  Print "program lunar_eclipse"
  Print "====================="

  ' request initial calendar date (month, day, year)

  getdate(cmonth, cday, cyear)

  ' request observer coordinates

  Print " "

  observer(obslat, obslong, obsalt)

  ' initial utc julian day

  julian(cmonth, cday, cyear, jdutc)

  ' compute initial tdb julian date

  utc2tdb(jdutc, jdtdb)

  jdtdbi = jdtdb

  ' search duration (days)

  Print " "

  Print "please input the search duration in days"

'  Input ndays
ndays = 30

  Print " "
  Print "searching for lunar eclipse ..."
  Print " "

  ' define search parameters

  ti = 0.0

  tf = ndays

  dt = 0.25

  dtsml = 0.1

  ' find eclipse conditions

  ecl_event(ti, tf, dt, dtsml)

Print "Run time: " Timer/1000 " seconds"
End

  ''''''''''''''''
  ''''''''''''''''

Sub eclfunc(x, fx)

  ' lunar eclipse objective function

  ' input

  '  x = elapsed simulation time (days)

  ' output

  '  fx = objective function at x

  '''''''''''''''''''''''''''''''

  Local rsun(3), rmoon(3), usun(3), umoon(3)

  Local jdtdb, jdutc

  Local cpsi, pangle, uangle

  Local i As integer

  ' current tdb julian date

  jdtdb = jdtdbi + x

  ' calculate geocentric position vector of the moon (au)

  moon(jdtdb, rmoon())

  ' unit position vector of the moon

  uvector(rmoon(), umoon())

  ' compute semidiameter of the moon

  sdmoon = ASin(radmoon / vecmag(rmoon()))

  ' calculate geocentric position vector of the sun (au)

  sun(jdtdb, rsun())

  ' unit position vector of the sun

  uvector(rsun(), usun())

  ' compute semidiameter of the sun

  sdsun = ASin(radsun / vecmag(rsun()))

  ' calculate penumbra shadow angle

  pmoon = ASin(reqm / vecmag(rmoon()))

  psun = ASin(reqm / vecmag(rsun()))

  pangle = 1.02 * (0.99834 * pmoon + psun + sdsun)

  ' compute umbra shadow angle

  uangle = 1.02 * (0.99834 * pmoon + psun - sdsun)

  ' compute separation angle between anti-sun and moon vectors

  cpsi = vdot(umoon(), usun())

  psi = ACos(-cpsi)

  ' compute objective function

  fx = psi - pangle - sdmoon

End Sub

  '''''''''''''''''''''''''''''''
  '''''''''''''''''''''''''''''''

Sub ecl_event(ti, tf, dt, dtsml)

  ' predict lunar eclipse events

  ' input

  '  ti    = initial simulation time
  '  tf    = final simulation time
  '  dt    = step size used for bounding minima
  '  dtsml = small step size used to determine whether
  '          the function is increasing or decreasing

  '''''''''''''''''''''''''''''''''''''''''''''''''''

  Local tolm, iend As integer

  Local fmin1, tmin1

  Local ftemp, df, dflft

  Local el, er

  Local t, ft

  Local iter1 As integer, iter2 As integer

  Local iter3 As integer

  ' initialization

  tolm = 1.0e-8

  iend = 0

  ' check the initial time for a minimum

  eclfunc(ti, fmin1)

  tmin1 = ti

  eclfunc(ti + dtsml, ftemp)

  df = ftemp - fmin1

  dflft = df

  el = ti

  er = el

  t = ti

  ' if the slope is positive and the minimum is
  ' negative, calculate event conditions at the initial time

  If (df > 0.0 And fmin1 < 0.0) Then

    events1(ti, tf, tmin1)

  End If

  For iter1 = 1 To 1000

    ' find where function first starts decreasing

    For iter2 = 1 To 1000

      If (df <= 0.0) Then

        Exit For

      End If

      t = t + dt

      If (t >= tf) Then

        ' check final time for a minimum

        If (iend = 1) Then Exit Sub

        eclfunc(tf, fmin1)

        eclfunc(tf - dtsml, ftemp)

        df = fmin1 - ftemp

        ' set minimum time to final simulation time

        tmin1 = tf

        If (df < 0.0) Then

          ' if both the slope and minimum are negative,
          ' calculate the event conditions at the final
          ' simulation time

          If (fmin1 < 0.0) Then

            events1(ti, tf, tmin1)

          End If

          ' otherwise, we're finished

          Exit Sub

        End If

        If (dflft > 0.0) Then Exit Sub

        er = tf

        iend = 1

      End If

      eclfunc(t, ft)

      eclfunc(t - dtsml, ftemp)

      df = ft - ftemp

    Next iter2

    ' function decreasing - find where function
    ' first starts increasing

    For iter3 = 1 To 1000

      el = t

      dflft = df

      t = t + dt

      If (t >= tf) Then

        ' check final time for a minimum

        If (iend = 1) Then Exit Sub

        eclfunc(tf, fmin1)

        eclfunc(tf - dtsml, ftemp)

        df = fmin1 - ftemp

        ' set minimum time to final simulation time

        tmin1 = tf

        If (df < 0.0) Then

          ' if both the slope and minimum are negative,
          ' calculate the event conditions at the final
          ' simulation time

          If (fmin1 < 0.0) Then

            events1(ti, tf, tmin1)

          End If

          ' otherwise, we're finished

          Exit Sub

        End If

        If (dflft > 0.0) Then Exit Sub

        er = tf

        iend = 1

      End If

      eclfunc(t, ft)

      eclfunc(t - dtsml, ftemp)

      df = ft - ftemp

      If (df > 0.0) Then Exit For

    Next iter3

    er = t

    ' calculate minimum using Brent's method

    minima(el, er, tolm, tmin1, fmin1)

    el = er

    dflft = df

    ' if the minimum is negative,
    ' calculate event conditions for this minimum

    If (fmin1 < 0.0) Then

      events1(ti, tf, tmin1)

      t = trr

    End If

  Next iter1

End Sub

  ''''''''''''''''''''''''
  ''''''''''''''''''''''''

Sub events1(ti, tf, topt)

  ' compute and display eclipse events

  ' input

  '  ti   = initial simulation time
  '  tf   = final simulation time
  '  topt = extrema time

  ''''''''''''''''''''''

  ' define root-bracketing and root-finding control parameters

  Local factor = 0.25            ' geometric acceleration factor

  Local dxmax = 600.0 / 86400.0  ' rectification interval

  Local rtol = 1.0e-8            ' root-finding convergence tolerance

  Local t1in, t2in

  Local t1out, t2out

  Local troot, froot, jdate

  Local iter As integer

  ' display type of eclipse

  For iter = 1 To 10

    If (psi < 1.02 * (0.99834 * pmoon + psun - sdsun) - sdmoon) Then

      Print " "
      Print "total lunar eclipse"
      Print "==================="
      Print " "

      Exit For

    End If

    If (psi < 1.02 * (0.99834 * pmoon + psun - sdsun) + sdmoon) Then

      Print " "
      Print "partial lunar eclipse"
      Print "====================="
      Print " "

      Exit For

    End If

    If (psi < 1.02 * (0.99834 * pmoon + psun + sdsun) + sdmoon) Then

      Print " "
      Print "penumbral lunar eclipse"
      Print "======================="
      Print " "

      Exit For

    End If

  Next iter

  ' compute and display event start conditions

  t1in = topt

  t2in = t1in - 30.0 / 86400.0

  broot(t1in, t2in, factor, dxmax, t1out, t2out)

  brent(t1out, t2out, rtol, troot, froot)

  ' set to initial time if before ti

  If (troot < ti) Then

    troot = ti

    eclfunc(ti, froot)

  End If

  jdate = jdtdbi + troot

  eclprint(1, jdate)

  ' compute and display greatest eclipse conditions

  jdate = jdtdbi + topt

  eclprint(2, jdate)

  ' compute and display event end conditions

  t2in = t1in + 30.0 / 86400.0

  broot(t1in, t2in, factor, dxmax, t1out, t2out)

  brent(t1out, t2out, rtol, troot, froot)

  ' set to final time if after tf

  If (troot > tf) Then

    troot = tf

    eclfunc(tf, froot)

  End If

  jdate = jdtdbi + troot

  eclprint(3, jdate)

  ' save current value of root

  trr = troot

End Sub

  ''''''''''''''''''''''''
  ''''''''''''''''''''''''

Sub eclprint(iflag, jdtdb)

  ' print lunar eclipse conditions

  ''''''''''''''''''''''''''''''''

  Local rsun(3), rmoon(3)

  Local jdutc, dms$ As string

  Local deltat, gast, azimuth, elevation

  If (iflag = 1) Then

    Print " "
    Print "begin penumbral phase of lunar eclipse"
    Print "--------------------------------------"
    Print " "

    jdprint = jdtdb

  End If

  If (iflag = 2) Then

    Print " "
    Print "greatest eclipse conditions"
    Print "---------------------------"
    Print " "

  End If

  If (iflag = 3) Then

    Print " "
    Print "end penumbral phase of lunar eclipse"
    Print "------------------------------------"
    Print " "

  End If

  ' print UTC julian date

  tdb2utc(jdtdb, jdutc)

  jd2str(jdutc)

  Print " "

  Print "UTC julian day     ", Str$(jdutc, 0, 8)

  ' compute and display topocentric coordinates of the moon

  gast2(jdutc, gast)

  moon(jdtdb, rmoon())

  eci2topo(gast, rmoon(), azimuth, elevation)

  Print " "

  Print "topocentric coordinates"

  Print " "

  deg2str(rtd * azimuth, dms$)

  Print "lunar azimuth angle    ", dms$

  Print " "

  deg2str(rtd * elevation, dms$)

  Print "lunar elevation angle  ", dms$

  ' determine and display event duration

  If (iflag = 3) Then

    deltat = 24.0 * (jdtdb - jdprint)

    Print " "

    Print "event duration   ", Str$(deltat, 0, 8) + " hours"

    Print " "

  End If

End Sub

  '''''''''''''''''
  '''''''''''''''''

Sub sun(jd, rsun())

  ' precision ephemeris of the Sun

  ' input

  '  jd = julian ephemeris day

  ' output

  '  rlsun = ecliptic longitude of the sun
  '          (0 <= rlsun <= 2 pi)
  '  rasc  = right ascension of the Sun (radians)
  '          (0 <= rasc <= 2 pi)
  '  decl  = declination of the Sun (radians)
  '          (-pi/2 <= decl <= pi/2)

  ''''''''''''''''''''''''''''''''''

  Local u, a1, a2, psi, deps, meps, eps, seps, ceps

  Local dl, dr, w, srl, crl, srb, crb, sra, cra

  u = (jd - 2451545.0) / 3652500.0

  ' compute nutation in longitude

  a1 = 2.18 + u * (-3375.7 + u * 0.36)

  a2 = 3.51 + u * (125666.39 + u * 0.1)

  psi = 0.0000001 * (-834.0 * Sin(a1) - 64.0 * Sin(a2))

  ' compute nutation in obliquity

  deps = 0.0000001 * u * (-226938 + u * (-75 + u * (96926 + u * (-2491 - u * 12104))))

  meps = 0.0000001 * (4090928.0 + 446.0 * Cos(a1) + 28.0 * Cos(a2))

  eps = meps + deps

  obliq = eps

  seps = Sin(eps)

  ceps = Cos(eps)

  dl = 0.0

  dr = 0.0

  For i% = 1 To 50

    w = xsa(i%) + xsb(i%) * u

    dl = dl + xsl(i%) * Sin(w)

    If (xsr(i%) <> 0.0) Then

      dr = dr + xsr(i%) * Cos(w)

    End If

  Next i%

  dl = modulo(dl * 0.0000001 + 4.9353929 + 62833.196168 * u)

  dr = dr * 0.0000001 + 1.0001026

  rlsun = modulo(dl + 0.0000001 * (-993.0 + 17.0 * Cos(3.1 + 62830.14 * u)) + psi)

  rb = 0.0

  ' compute geocentric declination and right ascension

  crl = Cos(rlsun)
  srl = Sin(rlsun)
  crb = Cos(rb)
  srb = Sin(rb)

  decl = ASin(ceps * srb + seps * crb * srl)

  sra = -seps * srb + ceps * crb * srl

  cra = crb * crl

  rasc = atan3(sra, cra)

  ' geocentric equatorial position vector of the Sun (au)

  rsun(1) = dr * Cos(rasc) * Cos(decl)

  rsun(2) = dr * Sin(rasc) * Cos(decl)

  rsun(3) = dr * Sin(decl)

End Sub

  ''''''''''''''''''''''''''''''''''''
  ''''''''''''''''''''''''''''''''''''

Sub eci2topo(gast, robj(), azim, elev)

  ' convert eci position vector to topocentric coordinates

  ' input

  '  gast = Greenwich apparent sidereal time (radians)
  '  robj = eci position vector of object (kilometers)

  ' output

  '  azim = topocentric azimuth (radians)
  '  elev = topocentric elevation (radians)

  ''''''''''''''''''''''''''''''''''''''''''''''

  Local rsite(3), rhoijk(3), rhohatijk(3), rhohatsez(3)

  Local i As integer, tmatrix(3, 3)

  Local obslst, srange, sobslat

  Local cobslat, sobslst, cobslst

  ' observer local sidereal time

  obslst = modulo(gast + obslong)

  gsite(obslst, rsite())

  ' eci vector from observer to moon

  For i = 1 To 3

    rhoijk(i) = aunit * robj(i) - rsite(i)

  Next i

  ' observer-to-object slant range (kilometers)

  srange = vecmag(rhoijk())

  ' compute topocentric unit pointing vector

  uvector(rhoijk(), rhohatijk())

  ' compute eci-to-sez transformation matrix

  sobslat = Sin(obslat)
  cobslat = Cos(obslat)

  sobslst = Sin(obslst)
  cobslst = Cos(obslst)

  tmatrix(1, 1) = sobslat * cobslst
  tmatrix(1, 2) = sobslat * sobslst
  tmatrix(1, 3) = -cobslat

  tmatrix(2, 1) = -sobslst
  tmatrix(2, 2) = cobslst
  tmatrix(2, 3) = 0.0

  tmatrix(3, 1) = cobslat * cobslst
  tmatrix(3, 2) = cobslat * sobslst
  tmatrix(3, 3) = sobslat

  ' compute sez coordinates

  matxvec(tmatrix(), rhohatijk(), rhohatsez())

  ' topocentric elevation (radians)

  elev = ASin(rhohatsez(3))

  ' topocentric azimuth (radians)

  azim = atan3(rhohatsez(2), -rhohatsez(1))

End Sub

  ''''''''''''''''''''''
  ''''''''''''''''''''''

Sub moon(jdate, rmoon())

  ' geocentric position of the moon subroutine

  ' input

  '  jdate = tdb julian day

  ' output

  '  rmoon = eci position vector of the moon (kilometers)

  '''''''''''''''''''''''''''''''''''''''''''''''''''''''

  Local t1, t2, t3

  Local t4, dpsi, deps

  Local ll, d, lp

  Local l, f, t

  Local ve, ma, ju

  Local rm, dv, pl

  Local plat, plon

  Local a, b

  Local rasc, decl, obliq

  ' get nutations and obliquity

  obliq_lp(jdate, dpsi, deps, obliq)

  ' evaluate lunar ephemeris

  t1 = (jdate - 2451545.0) / 36525.0

  t2 = t1 * t1
  t3 = t1 * t1 * t1
  t4 = t1 * t1 * t1 * t1

  ' calculate fundamental arguments (radians)

  ll = dtr * (218 + (18 * 60 + 59.95571) / 3600)

  ll = modulo(ll + atr * (1732564372.83264 * t1 - 4.7763 * t2 + .006681 * t3 - 0.00005522 * t4))

  d = dtr * (297 + (51 * 60 + .73512) / 3600)

  d = modulo(d + atr * (1602961601.4603 * t1 - 5.8681 * t2 + .006595 * t3 - 0.00003184 * t4))

  lp = dtr * (357 + (31 * 60 + 44.79306) / 3600)

  lp = modulo(lp + atr * (129596581.0474 * t1 - .5529 * t2 + 0.000147 * t3))

  l = dtr * (134 + (57 * 60 + 48.28096) / 3600)

  l = modulo(l + atr * (1717915923.4728 * t1 + 32.3893 * t2 + .051651 * t3 - 0.0002447 * t4))

  f = dtr * (93 + (16 * 60 + 19.55755) / 3600)

  f = modulo(f + atr * (1739527263.0983 * t1 - 12.2505 * t2 - .001021 * t3 + 0.00000417 * t4))

  t = dtr * (100 + (27 * 60 + 59.22059) / 3600)

  t = modulo(t + atr * (129597742.2758 * t1 - .0202 * t2 + .000009 * t3 + 0.00000015 * t4))

  ve = dtr * (181 + (58 * 60 + 47.28305) / 3600)

  ve = modulo(ve + atr * 210664136.43355 * t1)

  ma = dtr * (355 + (25 * 60 + 59.78866) / 3600)

  ma = modulo(ma + atr * 68905077.59284 * t1)

  ju = dtr * (34 + (21 * 60 + 5.34212) / 3600)

  ju = modulo(ju + atr * 10925660.42861 * t1)

  ' compute geocentric distance (kilometers)

  ' a(c,0,r) series

  rm = 385000.52899
  rm = rm - 20905.35504 * Sin(l + pidiv2)
  rm = rm - 3699.11092 * Sin(2 * d - l + pidiv2)
  rm = rm - 2955.96756 * Sin(2 * d + pidiv2)
  rm = rm - 569.92512 * Sin(2 * l + pidiv2)
  rm = rm + 246.15848 * Sin(2 * d - 2 * l + pidiv2)
  rm = rm - 204.58598 * Sin(2 * d - lp + pidiv2)
  rm = rm - 170.73308 * Sin(2 * d + l + pidiv2)
  rm = rm - 152.13771 * Sin(2 * d - lp - l + pidiv2)
  rm = rm - 129.62014 * Sin(lp - l + pidiv2)
  rm = rm + 108.7427 * Sin(d + pidiv2)
  rm = rm + 104.75523 * Sin(lp + l + pidiv2)
  rm = rm + 79.66056 * Sin(l - 2 * f + pidiv2)
  rm = rm + 48.8883 * Sin(lp + pidiv2)
  rm = rm - 34.78252 * Sin(4 * d - l + pidiv2)
  rm = rm + 30.82384 * Sin(2 * d + lp + pidiv2)
  rm = rm + 24.20848 * Sin(2 * d + lp - l + pidiv2)
  rm = rm - 23.21043 * Sin(3 * l + pidiv2)
  rm = rm - 21.63634 * Sin(4 * d - 2 * l + pidiv2)
  rm = rm - 16.67471 * Sin(d + lp + pidiv2)
  rm = rm + 14.40269 * Sin(2 * d - 3 * l + pidiv2)
  rm = rm - 12.8314 * Sin(2 * d - lp + l + pidiv2)
  rm = rm - 11.64995 * Sin(4 * d + pidiv2)
  rm = rm - 10.44476 * Sin(2 * d + 2 * l + pidiv2)
  rm = rm + 10.32111 * Sin(2 * d - 2 * f + pidiv2)
  rm = rm + 10.0562 * Sin(2 * d - lp - 2 * l + pidiv2)
  rm = rm - 9.88445 * Sin(2 * d - 2 * lp + pidiv2)
  rm = rm + 8.75156 * Sin(2 * d - l - 2 * f + pidiv2)
  rm = rm - 8.37911 * Sin(d - l + pidiv2)
  rm = rm - 7.00269 * Sin(lp - 2 * l + pidiv2)
  rm = rm + 6.322 * Sin(d + l + pidiv2)
  rm = rm + 5.75085 * Sin(lp + 2 * l + pidiv2)
  rm = rm - 4.95013 * Sin(2 * d - 2 * lp - l + pidiv2)
  rm = rm - 4.42118 * Sin(2 * l - 2 * f + pidiv2)
  rm = rm + 4.13111 * Sin(2 * d + l - 2 * f + pidiv2)
  rm = rm - 3.95798 * Sin(4 * d - lp - l + pidiv2)
  rm = rm + 3.25824 * Sin(3 * d - l + pidiv2)
  rm = rm - 3.1483 * Sin(2 * f + pidiv2)
  rm = rm + 2.61641 * Sin(2 * d + lp + l + pidiv2)
  rm = rm + 2.35363 * Sin(2 * d + 2 * lp - l + pidiv2)
  rm = rm - 2.11713 * Sin(2 * lp - l + pidiv2)
  rm = rm - 1.89704 * Sin(4 * d - lp - 2 * l + pidiv2)
  rm = rm - 1.73853 * Sin(d - 2 * l + pidiv2)
  rm = rm - 1.57139 * Sin(4 * d - lp + pidiv2)
  rm = rm - 1.42255 * Sin(4 * d + l + pidiv2)
  rm = rm - 1.41893 * Sin(3 * d + pidiv2)
  rm = rm + 1.16553 * Sin(2 * lp + l + pidiv2)
  rm = rm - 1.11694 * Sin(4 * l + pidiv2)
  rm = rm + 1.06567 * Sin(2 * lp + pidiv2)
  rm = rm - .93332 * Sin(d + lp + l + pidiv2)
  rm = rm + .86243 * Sin(3 * d - 2 * l + pidiv2)
  rm = rm + .85124 * Sin(d + lp - l + pidiv2)
  rm = rm - .8488 * Sin(2 * d - lp + 2 * l + pidiv2)
  rm = rm - .79563 * Sin(d - 2 * f + pidiv2)
  rm = rm + .77854 * Sin(2 * d - 4 * l + pidiv2)
  rm = rm + .77404 * Sin(2 * d - 2 * l + 2 * f + pidiv2)
  rm = rm - .66968 * Sin(2 * d + 3 * l + pidiv2)
  rm = rm - .65753 * Sin(2 * d - 2 * lp + l + pidiv2)
  rm = rm + .65706 * Sin(2 * d - lp - 2 * f + pidiv2)
  rm = rm + .59632 * Sin(2 * d - l + 2 * f + pidiv2)
  rm = rm + .57879 * Sin(4 * d + lp - l + pidiv2)
  rm = rm - .51423 * Sin(4 * d - 3 * l + pidiv2)
  rm = rm - .50792 * Sin(4 * d - 2 * f + pidiv2)
  rm = rm + .49755 * Sin(d - lp + pidiv2)
  rm = rm + .49504 * Sin(2 * d - lp - 3 * l + pidiv2)
  rm = rm + .47262 * Sin(2 * d - 2 * l - 2 * f + pidiv2)
  rm = rm - .4225 * Sin(6 * d - 2 * l + pidiv2)
  rm = rm - .42241 * Sin(lp - 3 * l + pidiv2)
  rm = rm - .41071 * Sin(2 * d - 3 * lp + pidiv2)
  rm = rm + .37852 * Sin(d + 2 * l + pidiv2)
  rm = rm + .35508 * Sin(lp + 3 * l + pidiv2)
  rm = rm + .34302 * Sin(2 * d - 2 * lp - 2 * l + pidiv2)
  rm = rm + .33463 * Sin(lp - l + 2 * f + pidiv2)
  rm = rm + .33225 * Sin(d + lp - 2 * l + pidiv2)
  rm = rm + .32334 * Sin(2 * d - lp - l - 2 * f + pidiv2)
  rm = rm - .32176 * Sin(4 * d - l - 2 * f + pidiv2)
  rm = rm - .28663 * Sin(6 * d - l + pidiv2)
  rm = rm + .28399 * Sin(2 * d + 2 * l - 2 * f + pidiv2)
  rm = rm - .27904 * Sin(4 * d - 2 * lp - l + pidiv2)
  rm = rm + .2556 * Sin(3 * d - lp - l + pidiv2)
  rm = rm - .2481 * Sin(lp + l - 2 * f + pidiv2)
  rm = rm + .24452 * Sin(4 * d + lp + pidiv2)
  rm = rm + .23695 * Sin(4 * d + lp - 2 * l + pidiv2)
  rm = rm - .21258 * Sin(3 * d + lp - l + pidiv2)
  rm = rm + .21251 * Sin(2 * d + lp + 2 * l + pidiv2)
  rm = rm + .20941 * Sin(2 * d - lp + l - 2 * f + pidiv2)
  rm = rm - .20285 * Sin(4 * d - lp + l + pidiv2)
  rm = rm + .20099 * Sin(3 * d - 2 * f + pidiv2)
  rm = rm - .18567 * Sin(lp - 2 * f + pidiv2)
  rm = rm - .18316 * Sin(6 * d - 3 * l + pidiv2)
  rm = rm + .16857 * Sin(2 * d + lp - 3 * l + pidiv2)
  rm = rm - .15802 * Sin(lp + 2 * f + pidiv2)
  rm = rm - .15707 * Sin(3 * d - lp + pidiv2)
  rm = rm - .14806 * Sin(2 * d - 3 * lp - l + pidiv2)
  rm = rm + .14763 * Sin(2 * d + 2 * lp + pidiv2)
  rm = rm + .14368 * Sin(2 * d + lp - 2 * l + pidiv2)
  rm = rm - .13922 * Sin(4 * d + 2 * l + pidiv2)
  rm = rm - .13617 * Sin(2 * lp - 2 * l + pidiv2)
  rm = rm - .13571 * Sin(2 * d + lp - 2 * f + pidiv2)
  rm = rm - .12805 * Sin(4 * d - 2 * lp + pidiv2)
  rm = rm + .11411 * Sin(d - lp - l + pidiv2)
  rm = rm + .10998 * Sin(d - lp + l + pidiv2)
  rm = rm - .10887 * Sin(2 * d + 2 * lp - 2 * l + pidiv2)
  rm = rm - .10833 * Sin(4 * d - 2 * lp - 2 * l + pidiv2)
  rm = rm - .10766 * Sin(3 * d + lp + pidiv2)
  rm = rm - .10326 * Sin(l + 2 * f + pidiv2)
  rm = rm - .09938 * Sin(d - 3 * l + pidiv2)
  rm = rm - .08587 * Sin(6 * d + pidiv2)
  rm = rm - .07982 * Sin(4 * d - 2 * l - 2 * f + pidiv2)
  rm = rm - 6.678e-02 * Sin(6 * d - lp - 2 * l + pidiv2)
  rm = rm - 6.545e-02 * Sin(3 * d + l + pidiv2)
  rm = rm + .06055 * Sin(d + l - 2 * f + pidiv2)
  rm = rm - .05904 * Sin(d + lp + 2 * l + pidiv2)
  rm = rm - .05888 * Sin(5 * l + pidiv2)
  rm = rm - .0585 * Sin(2 * d - lp + 3 * l + pidiv2)
  rm = rm - .05789 * Sin(4 * d - lp - 2 * f + pidiv2)
  rm = rm - .05527 * Sin(2 * d + lp + l - 2 * f + pidiv2)
  rm = rm + .05293 * Sin(3 * d - lp - 2 * l + pidiv2)
  rm = rm - .05191 * Sin(6 * d - lp - l + pidiv2)
  rm = rm + .05072 * Sin(2 * lp + 2 * l + pidiv2)
  rm = rm - .0502 * Sin(lp - 2 * l + 2 * f + pidiv2)
  rm = rm - .04843 * Sin(3 * d - 3 * l + pidiv2)
  rm = rm + .0474 * Sin(2 * d - 5 * l + pidiv2)
  rm = rm - .04736 * Sin(2 * d + lp - l - 2 * f + pidiv2)
  rm = rm - .04608 * Sin(2 * d - 2 * lp + 2 * l + pidiv2)
  rm = rm + .04591 * Sin(5 * d - 2 * l + pidiv2)
  rm = rm - .04422 * Sin(2 * d + 4 * l + pidiv2)
  rm = rm - .04316 * Sin(4 * d - lp - 3 * l + pidiv2)
  rm = rm - .04232 * Sin(d - l - 2 * f + pidiv2)
  rm = rm - .03894 * Sin(3 * lp - l + pidiv2)
  rm = rm + .0381 * Sin(3 * d + lp - 2 * l + pidiv2)
  rm = rm + .03734 * Sin(2 * d - lp - l + 2 * f + pidiv2)
  rm = rm + .03729 * Sin(d + 2 * lp + pidiv2)
  rm = rm + .03682 * Sin(4 * d + lp + l + pidiv2)
  rm = rm + .03379 * Sin(d + lp - 2 * f + pidiv2)
  rm = rm + .03265 * Sin(lp + 2 * l - 2 * f + pidiv2)
  rm = rm + .03143 * Sin(2 * d + 2 * f + pidiv2)
  rm = rm + .03024 * Sin(2 * d - lp - 2 * l + 2 * f + pidiv2)
  rm = rm - .02948 * Sin(d - 2 * lp + pidiv2)
  rm = rm - .02939 * Sin(4 * d - 4 * l + pidiv2)
  rm = rm + .0291 * Sin(2 * d - 3 * l - 2 * f + pidiv2)
  rm = rm - .02855 * Sin(2 * d - 3 * lp + l + pidiv2)
  rm = rm + .02839 * Sin(2 * d - 2 * lp - 2 * f + pidiv2)
  rm = rm - .02698 * Sin(4 * d - lp - l - 2 * f + pidiv2)
  rm = rm - .02674 * Sin(lp - 4 * l + pidiv2)
  rm = rm + .02658 * Sin(4 * d + 2 * lp - 2 * l + pidiv2)
  rm = rm - .02471 * Sin(d - l + 2 * f + pidiv2)
  rm = rm - .02436 * Sin(6 * d - lp - 3 * l + pidiv2)
  rm = rm - .02399 * Sin(4 * d + lp - 3 * l + pidiv2)
  rm = rm + .02368 * Sin(d + 3 * l + pidiv2)
  rm = rm + .02334 * Sin(2 * d - lp - 4 * l + pidiv2)
  rm = rm + .02304 * Sin(lp + 4 * l + pidiv2)
  rm = rm + .02127 * Sin(3 * lp + pidiv2)
  rm = rm - .02079 * Sin(4 * d - lp + 2 * l + pidiv2)
  rm = rm - .02008 * Sin(2 * d - 3 * l + 2 * f + pidiv2)

  ' a(p,0,r) series

  rm = rm + 1.0587 * Sin(2 * t - 2 * ju + 2 * d - l + 90.11969000000001 * dtr)
  rm = rm + .72783 * Sin(18 * ve - 16 * t - 2 * l + 116.54311 * dtr)
  rm = rm + .68256 * Sin(18 * ve - 16 * t + 296.54574 * dtr)
  rm = rm + .59827 * Sin(3 * ve - 3 * t + 2 * d - l + 89.98187 * dtr)
  rm = rm + .45648 * Sin(ll + l - f + 270.00126 * dtr)
  rm = rm + .45276 * Sin(ll - l - f + 90.00128 * dtr)
  rm = rm + .41011 * Sin(2 * t - 3 * ju + 2 * d - l + 280.06924 * dtr)
  rm = rm + .20497 * Sin(t - ju - 2 * d + 91.79862 * dtr)
  rm = rm + .20473 * Sin(18 * ve - 16 * t - 2 * d - l + 116.54222 * dtr)
  rm = rm + .20367 * Sin(18 * ve - 16 * t + 2 * d - l + 296.54299 * dtr)
  rm = rm + .16644 * Sin(2 * ve - 2 * t - 2 * d + 90.36386 * dtr)
  rm = rm + .1578 * Sin(4 * t - 8 * ma + 3 * ju + l + 194.98833 * dtr)
  rm = rm + .1578 * Sin(4 * t - 8 * ma + 3 * ju - l + 14.98841 * dtr)
  rm = rm + .15751 * Sin(t - ju - 2 * d + l + 91.74578 * dtr)
  rm = rm + .1445 * Sin(2 * t - 2 * ju - l + 89.97863 * dtr)
  rm = rm + .13811 * Sin(ve - t - l + 270.00993 * dtr)
  rm = rm + .13477 * Sin(18 * ve - 16 * t - 2 * d + 116.53978 * dtr)
  rm = rm + .12671 * Sin(18 * ve - 16 * t + 2 * d - 2 * l + 296.54238 * dtr)
  rm = rm + .12666 * Sin(t - ju - l + 91.22751 * dtr)
  rm = rm + .12362 * Sin(ve - t - 2 * d + 269.98523 * dtr)
  rm = rm + .12047 * Sin(2 * ve - 2 * t + 2 * d - l + 269.99692 * dtr)
  rm = rm + .11998 * Sin(ve - t + l + 90.01606 * dtr)
  rm = rm + .11617 * Sin(2 * ve - 2 * t - 2 * d + l + 90.31081 * dtr)
  rm = rm + .11256 * Sin(4 * t - 8 * ma + 3 * ju + 2 * d - l + 197.11421 * dtr)
  rm = rm + .11251 * Sin(4 * t - 8 * ma + 3 * ju - 2 * d + l + 17.11263 * dtr)
  rm = rm + .11226 * Sin(4 * t - 8 * ma + 3 * ju + 2 * d + 196.69224 * dtr)
  rm = rm + .11216 * Sin(4 * t - 8 * ma + 3 * ju - 2 * d + 16.68897 * dtr)
  rm = rm + .10689 * Sin(ll + 2 * d - f + 270.00092 * dtr)
  rm = rm + .10504 * Sin(t - ju + l + 271.06726 * dtr)
  rm = rm + .1006 * Sin(ve - t - 2 * d + l + 269.98452 * dtr)
  rm = rm + 9.932e-02 * Sin(3 * ve - 3 * t - l + 90.1054 * dtr)
  rm = rm + .09554 * Sin(ll - 2 * d - f + 90.00096 * dtr)
  rm = rm + .08508 * Sin(ll - f + 270.00061 * dtr)
  rm = rm + 7.9450e-02 * Sin(4 * ve - 4 * t + 2 * d - l + 89.99224 * dtr)
  rm = rm + .07725 * Sin(2 * t - 3 * ju - l + 280.16516 * dtr)
  rm = rm + 7.054e-02 * Sin(6 * ve - 8 * t + 2 * d - l + 77.22087 * dtr)
  rm = rm + 6.313e-02 * Sin(2 * ju - 5 * l + l + 256.56163 * dtr)

  ' a(p,1,r) series

  rm = rm + .51395 * t1 * Sin(2 * d - lp + pidiv2)
  rm = rm + .38245 * t1 * Sin(2 * d - lp - l + pidiv2)
  rm = rm + .32654 * t1 * Sin(lp - l + pidiv2)
  rm = rm + .26396 * t1 * Sin(lp + l + 270 * dtr)
  rm = rm + .12302 * t1 * Sin(lp + 270 * dtr)
  rm = rm + .07754 * t1 * Sin(2 * d + lp + 270 * dtr)
  rm = rm + .06068 * t1 * Sin(2 * d + lp - l + 270 * dtr)
  rm = rm + .0497 * t1 * Sin(2 * d - 2 * lp + pidiv2)
  rm = rm + .04194 * t1 * Sin(d + lp + pidiv2)
  rm = rm + .03222 * t1 * Sin(2 * d - lp + l + pidiv2)
  rm = rm + .02529 * t1 * Sin(2 * d - lp - 2 * l + 270 * dtr)
  rm = rm + .0249 * t1 * Sin(2 * d - 2 * lp - l + pidiv2)
  rm = rm + .00149 * t2 * Sin(2 * d - lp + pidiv2)
  rm = rm + .00111 * t2 * Sin(2 * d - lp - l + pidiv2)

  rmm = rm / aunit

  ' compute geocentric ecliptic longitude (arc seconds)

  ' a(c,0,v) series

  dv = 22639.58578 * Sin(l)
  dv = dv + 4586.4383 * Sin(2 * d - l)
  dv = dv + 2369.91394 * Sin(2 * d)
  dv = dv + 769.02571 * Sin(2 * l)
  dv = dv - 666.4171 * Sin(lp)
  dv = dv - 411.59567 * Sin(2 * f)
  dv = dv + 211.65555 * Sin(2 * d - 2 * l)
  dv = dv + 205.43582 * Sin(2 * d - lp - l)
  dv = dv + 191.9562 * Sin(2 * d + l)
  dv = dv + 164.72851 * Sin(2 * d - lp)
  dv = dv - 147.32129 * Sin(lp - l)
  dv = dv - 124.98812 * Sin(d)
  dv = dv - 109.38029 * Sin(lp + l)
  dv = dv + 55.17705 * Sin(2 * d - 2 * f)
  dv = dv - 45.0996 * Sin(l + 2 * f)
  dv = dv + 39.53329 * Sin(l - 2 * f)
  dv = dv + 38.42983 * Sin(4 * d - l)
  dv = dv + 36.12381 * Sin(3 * l)
  dv = dv + 30.77257 * Sin(4 * d - 2 * l)
  dv = dv - 28.39708 * Sin(2 * d + lp - l)
  dv = dv - 24.35821 * Sin(2 * d + lp)
  dv = dv - 18.58471 * Sin(d - l)
  dv = dv + 17.95446 * Sin(d + lp)
  dv = dv + 14.53027 * Sin(2 * d - lp + l)
  dv = dv + 14.3797 * Sin(2 * d + 2 * l)
  dv = dv + 13.89906 * Sin(4 * d)
  dv = dv + 13.19406 * Sin(2 * d - 3 * l)
  dv = dv - 9.67905 * Sin(lp - 2 * l)
  dv = dv - 9.36586 * Sin(2 * d - l + 2 * f)
  dv = dv + 8.60553 * Sin(2 * d - lp - 2 * l)
  dv = dv - 8.45310 * Sin(d + l)
  dv = dv + 8.05016 * Sin(2 * d - 2 * lp)
  dv = dv - 7.63015 * Sin(lp + 2 * l)
  dv = dv - 7.44749 * Sin(2 * lp)
  dv = dv + 7.37119 * Sin(2 * d - 2 * lp - l)
  dv = dv - 6.38315 * Sin(2 * d + l - 2 * f)
  dv = dv - 5.74161 * Sin(2 * d + 2 * f)
  dv = dv + 4.37401 * Sin(4 * d - lp - l)
  dv = dv - 3.99761 * Sin(2 * l + 2 * f)
  dv = dv - 3.20969 * Sin(3 * d - l)
  dv = dv - 2.91454 * Sin(2 * d + lp + l)
  dv = dv + 2.73189 * Sin(4 * d - lp - 2 * l)
  dv = dv - 2.56794 * Sin(2 * lp - l)
  dv = dv - 2.5212 * Sin(2 * d + 2 * lp - l)
  dv = dv + 2.48889 * Sin(2 * d + lp - 2 * l)
  dv = dv + 2.14607 * Sin(2 * d - lp - 2 * f)
  dv = dv + 1.97773 * Sin(4 * d + l)
  dv = dv + 1.93368 * Sin(4 * l)
  dv = dv + 1.87076 * Sin(4 * d - lp)
  dv = dv - 1.75297 * Sin(d - 2 * l)
  dv = dv - 1.43716 * Sin(2 * d + lp - 2 * f)
  dv = dv - 1.37257 * Sin(2 * l - 2 * f)
  dv = dv + 1.26182 * Sin(d + lp + l)
  dv = dv - 1.22412 * Sin(3 * d - 2 * l)
  dv = dv + 1.18683 * Sin(4 * d - 3 * l)
  dv = dv + 1.177 * Sin(2 * d - lp + 2 * l)
  dv = dv - 1.16169 * Sin(2 * lp + l)
  dv = dv + 1.07769 * Sin(d + lp - l)
  dv = dv + 1.0595 * Sin(2 * d + 3 * l)
  dv = dv - .99022 * Sin(2 * d + l + 2 * f)
  dv = dv + .94828 * Sin(2 * d - 4 * l)
  dv = dv + .75168 * Sin(2 * d - 2 * lp + l)
  dv = dv - .66938 * Sin(lp - 3 * l)
  dv = dv - .63521 * Sin(4 * d + lp - l)
  dv = dv - .58399 * Sin(d + 2 * l)
  dv = dv - .58331 * Sin(d - 2 * f)
  dv = dv + .57156 * Sin(6 * d - 2 * l)
  dv = dv - .56064 * Sin(2 * d - 2 * l - 2 * f)
  dv = dv - .55692 * Sin(d - lp)
  dv = dv - .54592 * Sin(lp + 3 * l)
  dv = dv - .53571 * Sin(2 * d - 2 * l + 2 * f)
  dv = dv + .4784 * Sin(2 * d - lp - 3 * f)
  dv = dv - .45379 * Sin(2 * d + 2 * l - 2 * f)
  dv = dv - .42622 * Sin(2 * d - lp - l + 2 * f)
  dv = dv + .42033 * Sin(4 * f)
  dv = dv + .4134 * Sin(lp + 2 * f)
  dv = dv + .40423 * Sin(3 * d)
  dv = dv + .39451 * Sin(6 * d - l)
  dv = dv - .38213 * Sin(2 * d - lp + 2 * f)
  dv = dv - .37451 * Sin(2 * d - lp + l - 2 * f)
  dv = dv - .35758 * Sin(4 * d + lp - 2 * l)
  dv = dv + .34965 * Sin(d + lp - 2 * l)
  dv = dv + .33979 * Sin(2 * d - 3 * lp)
  dv = dv - .32866 * Sin(3 * l + 2 * f)
  dv = dv + .30872 * Sin(4 * d - 2 * lp - l)
  dv = dv + .30155 * Sin(lp - l - 2 * f)
  dv = dv + .30086 * Sin(4 * d - l - 2 * f)
  dv = dv + .2942 * Sin(2 * d - 2 * lp - 2 * l)
  dv = dv + .29255 * Sin(6 * d - 3 * l)
  dv = dv - .29022 * Sin(2 * d + lp + 2 * l)

  ' a(p,2,v) series

  dv = dv + .00487 * t2 * Sin(lp)
  dv = dv - .0015 * t2 * Sin(2 * d - lp - l + Pi)
  dv = dv - .0012 * t2 * Sin(2 * d - lp + Pi)
  dv = dv + .00108 * t2 * Sin(lp - l)
  dv = dv + .0008 * t2 * Sin(lp + l)

  ' a(p,0,v) series

  dv = dv + 14.24883 * Sin(18 * ve - 16 * t - l + dtr * 26.54261)
  dv = dv + 7.06304 * Sin(ll - f + dtr * .00094)
  dv = dv + 1.14307 * Sin(2 * t - 2 * ju + 2 * d - l + dtr * 180.11977)
  dv = dv + .901140 * Sin(4 * t - 8 * ma + 3 * ju + dtr * 285.98707)
  dv = dv + .82155 * Sin(ve - t + dtr * 180.00988)
  dv = dv + .78811 * Sin(18 * ve - 16 * t - 2 * l + dtr * 26.54324)
  dv = dv + .7393 * Sin(18 * ve - 16 * t + dtr * 26.5456)
  dv = dv + .64371 * Sin(3 * ve - 3 * t + 2 * d - l + dtr * 179.98144)
  dv = dv + .6388 * Sin(t - ju + dtr * 1.2289)
  dv = dv + .56341 * Sin(10 * ve - 3 * t - l + dtr * 333.30551)
  dv = dv + .49331 * Sin(ll + l - f + .00127 * dtr)
  dv = dv + .49141 * Sin(ll - l - f + .00127 * dtr)
  dv = dv + .44532 * Sin(2 * t - 3 * ju + 2 * d - l + 10.07001 * dtr)
  dv = dv + .36061 * Sin(ll + f + .00071 * dtr)
  dv = dv + .34355 * Sin(2 * ve - 3 * t + 269.95393 * dtr)
  dv = dv + .32455 * Sin(t - 2 * ma + 318.13776 * dtr)
  dv = dv + .30155 * Sin(2 * ve - 2 * t + .20448 * dtr)
  dv = dv + .28938 * Sin(t + d - f + 95.13523 * dtr)
  dv = dv + .28281 * Sin(2 * t - 3 * ju + 2 * d - 2 * l + 10.03835 * dtr)
  dv = dv + .24515 * Sin(2 * t - 2 * ju + 2 * d - 2 * l + .08642 * dtr)

  ' a(p,1,v) series

  dv = dv + 1.6768 * t1 * Sin(lp)
  dv = dv + .51642 * t1 * Sin(2 * d - lp - l + Pi)
  dv = dv + .41383 * t1 * Sin(2 * d - lp + Pi)
  dv = dv + .37115 * t1 * Sin(lp - l)
  dv = dv + .2756 * t1 * Sin(lp + l)
  dv = dv + .25425 * t1 * Sin(18 * ve - 16 * t - l + 114.5655 * dtr)
  dv = dv + 7.1178e-02 * t1 * Sin(2 * d + lp - l)
  dv = dv + .06128 * t1 * Sin(2 * d + lp)
  dv = dv + .04516 * t1 * Sin(d + lp + Pi)
  dv = dv + .04048 * t1 * Sin(2 * d - 2 * lp + Pi)
  dv = dv + .03747 * t1 * Sin(2 * lp)
  dv = dv + .03707 * t1 * Sin(2 * d - 2 * lp - l + Pi)
  dv = dv + .03649 * t1 * Sin(2 * d - lp + l + Pi)
  dv = dv + .02438 * t1 * Sin(lp - 2 * l)
  dv = dv + .02165 * t1 * Sin(2 * d - lp - 2 * l + Pi)
  dv = dv + .01923 * t1 * Sin(lp + 2 * l)

  plon = modulo(ll + atr * dv + dpsi)

  ' compute geocentric ecliptic latitude (arc seconds)

  ' a(c,0,u) series

  pl = 18461.23868 * Sin(f)
  pl = pl + 1010.16707 * Sin(l + f)
  pl = pl + 999.69358 * Sin(l - f)
  pl = pl + 623.65243 * Sin(2 * d - f)
  pl = pl + 199.48374 * Sin(2 * d - l + f)
  pl = pl + 166.5741 * Sin(2 * d - l - f)
  pl = pl + 117.26069 * Sin(2 * d + f)
  pl = pl + 61.91195 * Sin(2 * l + f)
  pl = pl + 33.3572 * Sin(2 * d + l - f)
  pl = pl + 31.75967 * Sin(2 * l - f)
  pl = pl + 29.57658 * Sin(2 * d - lp - f)
  pl = pl + 15.56626 * Sin(2 * d - 2 * l - f)
  pl = pl + 15.12155 * Sin(2 * d + l + f)
  pl = pl - 12.09414 * Sin(2 * d + lp - f)
  pl = pl + 8.86814 * Sin(2 * d - lp - l + f)
  pl = pl + 7.95855 * Sin(2 * d - lp + f)
  pl = pl + 7.43455 * Sin(2 * d - lp - l - f)
  pl = pl - 6.73143 * Sin(lp - l - f)
  pl = pl + 6.57957 * Sin(4 * d - l - f)
  pl = pl - 6.46007 * Sin(lp + f)
  pl = pl - 6.29648 * Sin(3 * f)
  pl = pl - 5.63235 * Sin(lp - l + f)
  pl = pl - 5.3684 * Sin(d + f)
  pl = pl - 5.31127 * Sin(lp + l + f)
  pl = pl - 5.07591 * Sin(lp + l - f)
  pl = pl - 4.83961 * Sin(lp - f)
  pl = pl - 4.80574 * Sin(d - f)
  pl = pl + 3.98405 * Sin(3 * l + f)
  pl = pl + 3.67446 + Sin(4 * d - f)
  pl = pl + 2.99848 * Sin(4 * d - l + f)
  pl = pl + 2.79864 * Sin(l - 3 * f)
  pl = pl + 2.41388 * Sin(4 * d - 2 * l + f)
  pl = pl + 2.18631 * Sin(2 * d - 3 * f)
  pl = pl + 2.14617 * Sin(2 * d + 2 * l - f)
  pl = pl + 1.76598 * Sin(2 * d - lp + l - f)
  pl = pl - 1.62442 * Sin(2 * d - 2 * l + f)
  pl = pl + 1.5813 * Sin(3 * l - f)
  pl = pl + 1.51975 * Sin(2 * d + 2 * l + f)
  pl = pl - 1.51563 * Sin(2 * d - 3 * l - f)
  pl = pl - 1.31782 * Sin(2 * d + lp - l + f)
  pl = pl - 1.26427 * Sin(2 * d + lp + f)
  pl = pl + 1.19187 * Sin(4 * d + f)
  pl = pl + 1.13461 * Sin(2 * d - lp + l + f)
  pl = pl + 1.08578 * Sin(2 * d - 2 * lp - f)
  pl = pl - 1.01938 * Sin(l + 3 * f)
  pl = pl - .822710 * Sin(2 * d + lp + l - f)
  pl = pl + .80422 * Sin(d + lp - f)
  pl = pl + .80259 * Sin(d + lp + f)
  pl = pl - .79319 * Sin(lp - 2 * l - f)
  pl = pl - .79101 * Sin(2 * d + lp - l - f)
  pl = pl - .66741 * Sin(d + l + f)
  pl = pl + .65022 * Sin(2 * d - lp - 2 * l - f)
  pl = pl - .63881 * Sin(lp + 2 * l + f)
  pl = pl + .63371 * Sin(4 * d - 2 * l - f)
  pl = pl + .59577 * Sin(4 * d - lp - l - f)
  pl = pl - .58893 * Sin(d + l - f)
  pl = pl + .47338 * Sin(4 * d + l - f)
  pl = pl - .42989 * Sin(d - l - f)
  pl = pl + .41494 * Sin(4 * d - lp - f)
  pl = pl + .3835 * Sin(2 * d - 2 * lp + f)
  pl = pl - .35183 * Sin(3 * d - f)
  pl = pl + .33881 * Sin(4 * d - lp - l + f)
  pl = pl + .32906 * Sin(2 * d - l - 3 * f)

  ' a(p,0,u) series

  pl = pl + 8.04508 * Sin(ll + 180.00071 * dtr)
  pl = pl + 1.51021 * Sin(t + d + 276.68007 * dtr)
  pl = pl + .63037 * Sin(18 * ve - 16 * t - l + f + 26.54287 * dtr)
  pl = pl + .63014 * Sin(18 * ve - 16 * t - l - f + 26.54272 * dtr)
  pl = pl + .45586 * Sin(ll - l + .00075 * dtr)
  pl = pl + .41571 * Sin(ll + l + 180.00069 * dtr)
  pl = pl + .32622 * Sin(ll - 2 * f + .00086 * dtr)
  pl = pl + .29854 * Sin(ll - 2 * d + .00072 * dtr)

  ' a(p,1,u) series

  pl = pl + .0743 * t1 * Sin(2 * d - lp - f + Pi)
  pl = pl + .03043 * t1 * Sin(2 * d + lp - f)
  pl = pl + .02229 * t1 * Sin(2 * d - lp - l + f + Pi)
  pl = pl + .01999 * t1 * Sin(2 * d - lp + f + Pi)
  pl = pl + .01869 * t1 * Sin(2 * d - lp - l - f + Pi)
  pl = pl + .01696 * t1 * Sin(lp - l - f)
  pl = pl + .01623 * t1 * Sin(lp + f)

  plat = atr * pl

  ' compute geocentric right ascension and declination

  a = Sin(plon) * Cos(obliq) - Tan(plat) * Sin(obliq)

  b = Cos(plon)

  rasc = atan3(a, b)

  decl = ASin(Sin(plat) * Cos(obliq) + Cos(plat) * Sin(obliq) * Sin(plon))

  ' compute the geocentric position vector of the moon (au)

  rmoon(1) = rmm * Cos(rasc) * Cos(decl)

  rmoon(2) = rmm * Sin(rasc) * Cos(decl)

  rmoon(3) = rmm * Sin(decl)

End Sub

  ''''''''''''''''''''''''''''''''''''
  ''''''''''''''''''''''''''''''''''''

Sub obliq_lp(jdate, dpsi, deps, obliq)

  ' nutations and true obliquity

  ' input

  '  jdate = julian date

  ' output

  '  dpsi = nutation in longitude in radians

  '  deps = nutation in obliquity in radians

  '  obliq = true obliquity of the ecliptic in radians

  ''''''''''''''''''''''''''''''''''''''''''''''''''''

  Local t, t2, t3, eqeq

  Local th, tl, obm, obt, st

  Local tjdh, tjdl

  ' split the julian date

  tjdh = Int(jdate)

  tjdl = jdate - tjdh

  ' fundamental time units

  th = (tjdh - 2451545.0) / 36525.0

  tl = tjdl / 36525.0

  t = th + tl

  t2 = t * t

  t3 = t2 * t

  ' obtain equation of the equinoxes

  eqeq = 0.0

  ' obtain nutations

  nut2000_lp(jdate, dpsi, deps)

  ' compute mean obliquity of the ecliptic in seconds of arc

  obm = 84381.4480 - 46.8150 * t - 0.00059 * t2 + 0.001813 * t3

  ' compute true obliquity of the ecliptic in seconds of arc

  obt = obm + deps

  ' return elements in radians

  deps = atr * deps

  dpsi = atr * dpsi

  obliq = atr * obt

End Sub

  '''''''''''''''''''''''''''''''
  '''''''''''''''''''''''''''''''

Sub nut2000_lp(jdate, dpsi, deps)

  ' low precison nutation based on iau 2000a

  ' this function evaluates a short nutation series and returns approximate
  ' values for nutation in longitude and nutation in obliquity for a given
  ' tdb julian date. in this mode, only the largest 13 terms of the iau 2000a
  ' nutation series are evaluated.

  ' input

  '  jdate = tdb julian date

  ' output

  '  dpsi = nutation in longitude in arcseconds

  '  deps = nutation in obliquity in arcseconds

  '''''''''''''''''''''''''''''''''''''''''''''

  Local rev = 360.0 * 3600.0

  Local el, elp, f, d, omega

  Local i%, arg

  Local t = (jdate - 2451545.0) / 36525.0

  ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
  ' computation of fundamental (delaunay) arguments from simon et al. (1994)
  ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

  ' mean anomaly of the moon

  el = (485868.249036 + t * (1717915923.2178 + t * (31.8792 + t * (0.051635 + t * (-0.00024470)))) Mod rev) / seccon

  ' mean anomaly of the sun

  elp = (1287104.79305 + t * (129596581.0481 + t * (-0.5532 + t * (0.000136 + t * (- 0.00001149)))) Mod rev) / seccon

  ' mean argument of the latitude of the moon

  f = (335779.526232 + t * (1739527262.8478 + t * (-12.7512 + t * (-0.001037 + t * (0.00000417)))) Mod rev) / seccon

  ' mean elongation of the moon from the sun

  d = (1072260.70369 + t * (1602961601.2090 + t * (- 6.3706 + t * (0.006593 + t * (- 0.00003169)))) Mod rev) / seccon

  ' mean longitude of the ascending node of the moon (from simon section 3.4(b.3), precession = 5028.8200 arcsec/cy)

  omega = (450160.398036 + t * (- 6962890.5431 + t * (7.4722 + t * (0.007702 + t * (- 0.00005939)))) Mod rev) / seccon

  dpsi = 0.0

  deps = 0.0

  ' sum nutation series terms

  For i% = 13 To 1 Step -1

    arg = xnut(1, i%) * el + xnut(2, i%) * elp + xnut(3, i%) * f + xnut(4, i%) * d + xnut(5, i%) * omega

    dpsi = (xnut(6, i%) + xnut(7, i%) * t) * Sin(arg) + xnut(10, i%) * Cos(arg) + dpsi

    deps = (xnut(8, i%) + xnut(9, i%) * t) * Cos(arg) + xnut(11, i%) * Sin(arg) + deps

  Next i%

  dpsi = 1.0e-7 * dpsi

  deps = 1.0e-7 * deps

  ' add in out-of-phase component of principal (18.6-year) term
  ' (to avoid small but long-term bias in results)

  dpsi = dpsi + 0.0033 * Cos(omega)

  deps = deps + 0.0015 * Sin(omega)

End Sub

  ''''''''''''''''''''''''
  ''''''''''''''''''''''''

Sub tdb2utc(jdtdb, jdutc)

  ' convert TDB julian day to UTC julian day subroutine

  ' input

  '  jdtdb = TDB julian day

  ' output

  '  jdutc = UTC julian day

  '''''''''''''''''''''''''

  Local x1, x2

  Local xroot, froot

  jdsaved = jdtdb

  ' set lower and upper bounds

  x1 = jdsaved - 0.1

  x2 = jdsaved + 0.1

  ' solve for UTC julian day using Brent's method

  jbrent(x1, x2, 1.0e-8, xroot, froot)

  jdutc = xroot

End Sub

  ''''''''''''''''''''''''
  ''''''''''''''''''''''''

Sub gsite(angle, rsite())

  ' ground site position vector

  ' input

  '  angle  = local sidereal time or east longitude
  '           (radians; 0 <= angle <= 2*pi)

  ' input via global

  '  obslat = geodetic latitude (radians)
  '           (+north, -south; -pi/2 <= lat <= -pi/2)
  '  obsalt = geodetic altitude (meters)
  '           (+ above sea level, - below sea level)

  ' output

  '  rsite = ground site position vector (kilometers)

  ' special notes

  '  (1) eci coordinates if angle = local sidereal time

  '  (2) ecf coordinates if angle = east longitude

  '  (3) geocentric, equatorial coordinates

  '''''''''''''''''''''''''''''''''''''''''

  Local slat, clat

  Local sangle, cangle

  Local b, c, d, rtmp

  slat = Sin(obslat)

  clat = Cos(obslat)

  sangle = Sin(angle)

  cangle = Cos(angle)

  ' compute geodetic constants

  b = Sqr(1.0 - (2.0 * flat - flat * flat) * slat * slat)

  rtmp = aunit * reqm

  c = rtmp / b + 0.001 * obsalt

  d = rtmp * (1.0 - flat) * (1.0 - flat) / b + 0.001 * obsalt

  ' compute x, y and z components of position vector (kilometers)

  rsite(1) = c * clat * cangle

  rsite(2) = c * clat * sangle

  rsite(3) = d * slat

End Sub

  ''''''''''''''''''''
  ''''''''''''''''''''

Sub gast2(jdate, gast)

  ' greenwich apparent sidereal time

  ' input

  '  jdate = julian date

  ' output

  '  gast = greenwich apparent sidereal time (radians)

  '''''''''''''''''''''''''''''''''''''''''''''''''''

  Local t, t2, t3, eqeq, dpsi, deps

  Local th, tl, obm, obt, st, x

  Local tjdh As integer, tjdl

  tjdh = Int(jdate)

  tjdl = jdate - tjdh

  th = (tjdh - 2451545.0) / 36525

  tl = tjdl / 36525.0

  t = th + tl

  t2 = t * t

  t3 = t2 * t

  ' obtain equation of the equinoxes

  eqeq = 0.0

  ' obtain nutation parameters in seconds of arc

  nut2000_lp(jdate, dpsi, deps)

  ' compute mean obliquity of the ecliptic in seconds of arc

  obm = 84381.4480 - 46.8150 * t - 0.00059 * t2 + 0.001813 * t3

  ' compute true obliquity of the ecliptic in seconds of arc

  obliq = obm + deps

  ' compute equation of the equinoxes in seconds of time

  eqeq = (dpsi / 15.0) * Cos(obliq / seccon)

  st = eqeq - 6.2e-6 * t3 + 0.093104 * t2 + 67310.54841 + 8640184.812866 * tl + 3155760000.0 * tl + 8640184.812866 * th + 3155760000.0 * th

  ' modulo 24 hours

  x = st / 3600.0

  gast = x - 24.0 * Fix(x / 24.0)

  If (gast < 0.0) Then

    gast = gast + 24.0

  End If

  ' convert to radians

  gast = pi2 * (gast / 24.0)

End Sub

  ''''''''''''''''''''''''''''
  ''''''''''''''''''''''''''''

Sub getdate(month, day, year)
  ' interactive request calendar date subroutine

  Do
    Print " "
    Print "please input the initial calendar date"
    Print " "
    Print "(month [1 - 12], day [1 - 31], year [yyyy])"
    Print "< for example, october 21, 1986 is input as 10,21,1986 >"
    Print "< b.c. dates are negative, a.d. dates are positive >"
    Print "< the day of the month may also include a decimal part >"
    Print " "
'    Input month, day, year
month = 1
day = 1
year = 2000

  Loop Until (month >= 1 And month <= 12) And (day >= 1 And day <= 31)

End Sub

  '''''''''''''''''''''''''''''''''''
  '''''''''''''''''''''''''''''''''''

Sub observer(obslat, obslong, obsalt)

  ' interactive request of latitude, longitude and altitude subroutine

  ' output

  '  obslat  = latitude (radians)
  '  obslong = longitude (radians)
  '  obsalt  = altitude (meters)

  ''''''''''''''''''''''''''''''

  Do

    Print "please input the geographic latitude of the observer"
    Print "(degrees [-90 to +90], minutes [0 - 60], seconds [0 - 60])"
    Print "(north latitudes are positive, south latitudes are negative)"

'    Input obslat.deg$, obslat.min, obslat.sec
obslat.deg$ ="39"
obslat.min = 40
obslat.sec = 36

  Loop Until (Abs(Val(obslat.deg$)) <= 90.0 And (obslat.min >= 0.0 And obslat.min <= 60.0) And (obslat.sec >= 0.0 And obslat.sec <= 60.0))

  If (Left$(obslat.deg$, 2) = "-0") Then

    obslat = -dtr * (obslat.min / 60.0 + obslat.sec / 3600.0)

  ElseIf (Val(obslat.deg$) = 0.0) Then

    obslat = dtr * (obslat.min / 60.0 + obslat.sec / 3600.0)

  Else

    obslat = dtr * (Sgn(Val(obslat.deg$)) * (Abs(Val(obslat.deg$)) + obslat.min / 60.0 + obslat.sec / 3600.0))

  EndIf

  Do

    Print
    Print "please input the geographic longitude of the observer"
    Print "(degrees [0 - 360], minutes [0 - 60], seconds [0 - 60])"
    Print "(east longitude is positive, west longitude is negative)"

'    Input obslong.deg$, obslong.min, obslong.sec
obslong.deg$ = "-104"
obslong.min = 57
obslong.sec =12

  Loop Until (Abs(Val(obslong.deg$)) >= 0.0 And Abs(Val(obslong.deg$)) <= 360.0) And (obslong.min >= 0.0 And obslong.min <= 60.0) And (obslong.sec >= 0.0 And obslong.sec <= 60.0)

  If (Left$(obslong.deg$, 2) = "-0") Then

    obslong = -dtr * (obslong.min / 60 + obslong.sec / 3600)

  ElseIf (Val(obslong.deg$) = 0.0) Then

    obslong = dtr * (obslong.min / 60.0 + obslong.sec / 3600.0)

  Else

    obslong = dtr * (Sgn(Val(obslong.deg$)) * (Abs(Val(obslong.deg$)) + obslong.min / 60.0 + obslong.sec / 3600.0))

  EndIf

  Print " "

  Print "please input the altitude of the observer (meters)"
  Print "(positive above sea level, negative below sea level)"

'  Input obsalt
obslat = 1644

End Sub

  ''''''''''''''''''''''''''''''''
  ''''''''''''''''''''''''''''''''

Sub minima(a, b, tolm, xmin, fmin)

  ' one-dimensional minimization

  ' Brent's method

  ' input

  '  a    = initial x search value
  '  b    = final x search value
  '  tolm = convergence criterion

  ' output

  '  xmin = minimum x value

  ' note

  '  user-defined objective subroutine
  '  coded as usr_func(x, fx)

  ' remember: a maximum is simply a minimum
  '           with a negative attitude!

  '''''''''''''''''''''''''''''''''''''

  ' machine epsilon

  Local epsm = 2.23e-16

  ' golden number

  Local c = 0.38196601125

  Local iter As integer, d, e

  Local t2, p, q

  Local r, u, fu

  Local x, xm, w

  Local v, fx, fw

  Local tol1, fv

  x = a + c * (b - a)

  w = x

  v = w

  e = 0.0
  p = 0.0
  q = 0.0
  r = 0.0

  eclfunc(x, fx)

  fw = fx

  fv = fw

  For iter = 1 To 100

    If (iter > 50) Then

      Print ("error in function minima!")
      Print ("(more than 50 iterations)")

End

    End If

    xm = 0.5 * (a + b)

    tol1 = tolm * Abs(x) + epsm

    t2 = 2.0 * tol1

    If (Abs(x - xm) <= (t2 - 0.5 * (b - a))) Then

      xmin = x

      fmin = fx

      Exit Sub

    End If

    If (Abs(e) > tol1) Then

      r = (x - w) * (fx - fv)

      q = (x - v) * (fx - fw)

      p = (x - v) * q - (x - w) * r

      q = 2.0 * (q - r)

      If (q > 0.0) Then

        p = -p

      End If

      q = Abs(q)

      r = e

      e = d

    End If

    If ((Abs(p) >= Abs(0.5 * q * r)) Or (p <= q * (a - x)) Or (p >= q * (b - x))) Then

      If (x >= xm) Then

        e = a - x

      Else

        e = b - x

      End If

      d = c * e

    Else

      d = p / q

      u = x + d

      If ((u - a) < t2) Or ((b - u) < t2) Then

        d = Sgn(xm - x) * tol1

      End If

    End If

    If (Abs(d) >= tol1) Then

      u = x + d

    Else

      u = x + Sgn(d) * tol1

    End If

    eclfunc(u, fu)

    If (fu <= fx) Then

      If (u >= x) Then

        a = x

      Else

        b = x

      End If

      v = w

      fv = fw

      w = x

      fw = fx

      x = u

      fx = fu

    Else

      If (u < x) Then

        a = u

      Else

        b = u

      End If

      If ((fu <= fw) Or (w = x)) Then

        v = w

        fv = fw

        w = u

        fw = fu

      ElseIf ((fu <= fv) Or (v = x) Or (v = w)) Then

        v = u

        fv = fu

      End If

    End If

  Next iter

End Sub

  ''''''''''''''''''''''''''''''''''''''''''''''''
  ''''''''''''''''''''''''''''''''''''''''''''''''

Sub broot(x1in, x2in, factor, dxmax, x1out, x2out)

  ' bracket a single root of a nonlinear equation

  ' input

  '  x1in   = initial guess for first bracketing x value
  '  x2in   = initial guess for second bracketing x value
  '  factor = acceleration factor (non-dimensional)
  '  dxmax  = rectification interval

  ' output

  '  xout1 = final value for first bracketing x value
  '  xout2 = final value for second bracketing x value

  ''''''''''''''''''''''''''''''''''''''''''''''''''''

  Local f1, f2

  Local x3, dx

  ' evaluate objective function at initial value

  eclfunc(x1in, f1)

  ' save initial value

  x3 = x1in

  ' save initial delta-x

  dx = x2in - x1in

  ' perform bracketing until the product of the
  ' two function values is negative

  Do

    ' geometrically accelerate the second point

    x2in = x2in + factor * (x2in - x3)

    ' evaluate objective function at x2

    eclfunc(x2in, f2)

    ' check to see if rectification is required

    If (Abs(x2in - x3) > dxmax) Then

      x3 = x2in - dx

    End If

    ' is the root bracketed?

    If ((f1 * f2) < 0.0) Then Exit Do

  Loop

  x1out = x1in

  x2out = x2in

End Sub

  ''''''''''''''''''''''''''''''''''''''
  ''''''''''''''''''''''''''''''''''''''

Sub brent(x1, x2, tol, xroot, froot)

  ' real root of a single non-linear function subroutine

  ' input

  '  x1  = lower bound of search interval
  '  x2  = upper bound of search interval
  '  tol = convergence criter%ia

  ' output

  '  xroot = real root of f(x) = 0
  '  froot = function value

  ' note: requires sub eclfunc

  '''''''''''''''''''''''''''

  Local eps, a, b

  Local c, d, e

  Local fa, fb, fcc

  Local tol1, xm, p

  Local q, r, s

  Local xmin, tmp

  eps = 2.23e-16

  e = 0.0

  a = x1

  b = x2

  eclfunc(a, fa)

  eclfunc(b, fb)

  fcc = fb

  For iter% = 1 To 50

    If (fb * fcc > 0.0) Then

      c = a

      fcc = fa

      d = b - a

      e = d

    End If

    If (Abs(fcc) < Abs(fb)) Then

      a = b

      b = c

      c = a

      fa = fb

      fb = fcc

      fcc = fa

    End If

    tol1 = 2.0 * eps * Abs(b) + 0.5 * tol

    xm = 0.5 * (c - b)

    If (Abs(xm) <= tol1 Or fb = 0.0) Then Exit For

    If (Abs(e) >= tol1 And Abs(fa) > Abs(fb)) Then

      s = fb / fa

      If (a = c) Then

        p = 2.0 * xm * s

        q = 1.0 - s

      Else

        q = fa / fcc

        r = fb / fcc

        p = s * (2.0 * xm * q * (q - r) - (b - a) * (r - 1.0))

        q = (q - 1.0) * (r - 1.0) * (s - 1.0)

      End If

      If (p > 0.0) Then q = -q

      p = Abs(p)

      min = Abs(e * q)

      tmp = 3.0 * xm * q - Abs(tol1 * q)

      If (min < tmp) Then min = tmp

      If (2.0 * p < min) Then

        e = d

        d = p / q

      Else

        d = xm

        e = d

      End If

    Else

      d = xm

      e = d

    End If

    a = b

    fa = fb

    If (Abs(d) > tol1) Then

      b = b + d

    Else

      b = b + Sgn(xm) * tol1

    End If

    eclfunc(b, fb)

  Next iter%

  froot = fb

  xroot = b

End Sub

  ''''''''''''''''''''''''''''''''''''''
  ''''''''''''''''''''''''''''''''''''''

Sub jbrent(x1, x2, tol, xroot, froot)

  ' real root of a single non-linear function subroutine

  ' input

  '  x1  = lower bound of search interval
  '  x2  = upper bound of search interval
  '  tol = convergence criter%ia

  ' output

  '  xroot = real root of f(x) = 0
  '  froot = function value

  ' note: requires sub jdfunc

  '''''''''''''''''''''''''''

  Local eps, a, b

  Local c, d, e

  Local fa, fb, fcc

  Local tol1, xm, p

  Local q, r, s

  Local xmin, tmp, iter As integer

  eps = 2.23e-16

  e = 0.0

  a = x1

  b = x2

  jdfunc(a, fa)

  jdfunc(b, fb)

  fcc = fb

  For iter = 1 To 50

    If (fb * fcc > 0.0) Then

      c = a

      fcc = fa

      d = b - a

      e = d

    End If

    If (Abs(fcc) < Abs(fb)) Then

      a = b

      b = c

      c = a

      fa = fb

      fb = fcc

      fcc = fa

    End If

    tol1 = 2.0 * eps * Abs(b) + 0.5 * tol

    xm = 0.5 * (c - b)

    If (Abs(xm) <= tol1 Or fb = 0) Then Exit For

    If (Abs(e) >= tol1 And Abs(fa) > Abs(fb)) Then

      s = fb / fa

      If (a = c) Then

        p = 2.0 * xm * s

        q = 1.0 - s

      Else

        q = fa / fcc

        r = fb / fcc

        p = s * (2.0 * xm * q * (q - r) - (b - a) * (r - 1.0))

        q = (q - 1.0) * (r - 1.0) * (s - 1.0)

      End If

      If (p > 0) Then q = -q

      p = Abs(p)

      xmin = Abs(e * q)

      tmp = 3.0 * xm * q - Abs(tol1 * q)

      If (xmin < tmp) Then xmin = tmp

      If (2.0 * p < xmin) Then

        e = d

        d = p / q

      Else

        d = xm

        e = d

      End If

    Else

      d = xm

      e = d

    End If

    a = b

    fa = fb

    If (Abs(d) > tol1) Then

      b = b + d

    Else

      b = b + Sgn(xm) * tol1

    End If

    jdfunc(b, fb)

  Next iter

  froot = fb

  xroot = b

End Sub

  ''''''''''''''''''''''''''''''''
  ''''''''''''''''''''''''''''''''

Sub julian(month, day, year, jday)

  ' Gregorian date to julian day subroutine

  ' input

  '  month = calendar month
  '  day   = calendar day
  '  year  = calendar year (all four digits)

  ' output

  '  jday = julian day

  ' special notes

  '  (1) calendar year must include all digits

  '  (2) will report October 5, 1582 to October 14, 1582
  '      as invalid calendar dates and exit

  '''''''''''''''''''''''''''''''''''''''''

  Local a, b, c, m, y

  y = year

  m = month

  b = 0.0

  c = 0.0

  If (m <= 2.0) Then

    y = y - 1.0

    m = m + 12.0

  End If

  If (y < 0.0) Then c = -0.75

  If (year < 1582.0) Then

    ' null

  ElseIf (year > 1582.0) Then

    a = Fix(y / 100.0)

    b = 2.0 - a + Fix(a / 4.0)

  ElseIf (month < 10.0) Then

    ' null

  ElseIf (month > 10.0) Then

    a = Fix(y / 100.0)

    b = 2.0 - a + Fix(a / 4.0)

  ElseIf (day <= 4.0) Then

    ' null

  ElseIf (day > 14.0) Then

    a = Fix(y / 100.0)

    b = 2.0 - a + Fix(a / 4.0)

  Else

    Print "this date does not exist!!"

    Exit

  End If

  jday = Fix(365.25 * y + c) + Fix(30.6001 * (m + 1.0)) + day + b + 1720994.5

End Sub

  '''''''''''''''''''''''''''''''
  '''''''''''''''''''''''''''''''

Sub gdate(jday, month, day, year)

  ' Julian day to Gregorian date subroutine

  ' input

  '  jday = julian day

  ' output

  '  month = calendar month
  '  day   = calendar day
  '  year  = calendar year

  ''''''''''''''''''''''''

  Local a, b, c, d, e, f, z, alpha

  z = Fix(jday + 0.5)

  f = jday + 0.5 - z

  If (z < 2299161) Then

    a = z

  Else

    alpha = Fix((z - 1867216.25) / 36524.25)

    a = z + 1.0 + alpha - Fix(alpha / 4.0)

  End If

  b = a + 1524.0

  c = Fix((b - 122.1) / 365.25)

  d = Fix(365.25 * c)

  e = Fix((b - d) / 30.6001)

  day = b - d - Fix(30.6001 * e) + f

  If (e < 13.5) Then

    month = e - 1.0

  Else

    month = e - 13.0

  End If

  If (month > 2.5) Then

    year = c - 4716.0

  Else

    year = c - 4715.0

  End If

End Sub

  ''''''''''''''''''''''''
  ''''''''''''''''''''''''

Sub utc2tdb(jdutc, jdtdb)

  ' convert UTC julian date to TDB julian date

  ' input

  '  jdutc   = UTC julian day
  '  tai_utc = TAI-UTC (seconds)

  ' output

  '  jdtdb = TDB julian day

  ' Reference Frames in Astronomy and Geophysics
  ' J. Kovalevsky et al., 1989, pp. 439-442

  '''''''''''''''''''''''''''''''''''''''''

  Local corr, jdtt, t, leapsecond

  ' find current number of leap seconds

  findleap(jdutc, leapsecond)

  ' compute TDT julian date

  corr = (leapsecond + 32.184) / 86400.0

  jdtt = jdutc + corr

  ' time argument for correction

  t = (jdtt - 2451545.0) / 36525.0

  ' compute correction in microseconds

  corr = 1656.675 * Sin(dtr * (35999.3729 * t + 357.5287))
  corr = corr + 22.418     * Sin(dtr * (32964.467  * t + 246.199))
  corr = corr + 13.84      * Sin(dtr * (71998.746  * t + 355.057))
  corr = corr +  4.77      * Sin(dtr * ( 3034.906  * t +  25.463))
  corr = corr +  4.677     * Sin(dtr * (34777.259  * t + 230.394))
  corr = corr + 10.216 * t * Sin(dtr * (35999.373  * t + 243.451))
  corr = corr +  0.171 * t * Sin(dtr * (71998.746  * t + 240.98 ))
  corr = corr +  0.027 * t * Sin(dtr * ( 1222.114  * t + 194.661))
  corr = corr +  0.027 * t * Sin(dtr * ( 3034.906  * t + 336.061))
  corr = corr +  0.026 * t * Sin(dtr * (  -20.186  * t +   9.382))
  corr = corr +  0.007 * t * Sin(dtr * (29929.562  * t + 264.911))
  corr = corr +  0.006 * t * Sin(dtr * (  150.678  * t +  59.775))
  corr = corr +  0.005 * t * Sin(dtr * ( 9037.513  * t + 256.025))
  corr = corr +  0.043 * t * Sin(dtr * (35999.373  * t + 151.121))

  ' convert corrections to days

  corr = 0.000001 * corr / 86400.0

  ' TDB julian date

  jdtdb = jdtt + corr

End Sub

  ''''''''''''''''''''''''''''
  ''''''''''''''''''''''''''''

Sub findleap(jday, leapsecond)

  ' find number of leap seconds for utc julian day

  ' input

  '  jday = utc julian day

  ' input via global

  '  jdleap  = array of utc julian dates
  '  leapsec = array of leap seconds

  ' output

  '  leapsecond = number of leap seconds

  ''''''''''''''''''''''''''''''''''''''

  Local i As integer

  If (jday <= jdleap(1)) Then

    ' date is <= 1972; set to first data element

    leapsecond = leapsec(1)

    Exit Sub

  End If

  If (jday >= jdleap(28)) Then

    ' date is >= end of current data
    ' set to last data element

    leapsecond = leapsec(28)

    Exit Sub

  End If

  ' find data within table

  For i = 1 To 27

    If (jday >= jdleap(i) And jday < jdleap(i + 1)) Then

      leapsecond = leapsec(i)

      Exit Sub

    End If

  Next i

End Sub

  ''''''''''''''''''''
  ''''''''''''''''''''

Sub jdfunc(jdutc, fx)

  ' objective function for tdb2utc

  ' input

  '  jdin = current value for UTC julian day

  ' output

  '  fx = delta julian day

  ''''''''''''''''''''''''

  Local jdwrk

  utc2tdb(jdutc, jdwrk)

  fx = jdwrk - jdsaved

End Sub

  ''''''''''''''''
  ''''''''''''''''

Function modulo(x)

  ' modulo 2 pi function

  ''''''''''''''''''''''

  Local a

  a = x - pi2 * Fix(x / pi2)

  If (a < 0.0) Then

    a = a + pi2

  End If

  modulo = a

End Function

  ''''''''''''''''''
  ''''''''''''''''''

Function atan3(a, b)

  ' four quadrant inverse tangent function

  ' input

  '  a = sine of angle
  '  b = cosine of angle

  ' output

  '  atan3 = angle (0 =< atan3 <= 2 * pi; radians)

  ''''''''''''''''''''''''''''''''''''''''''''''''

  Local c

  If (Abs(a) < 1.0e-10) Then

    atan3 = (1.0 - Sgn(b)) * pidiv2

    Exit Function

  Else

    c = (2.0 - Sgn(a)) * pidiv2

  EndIf

  If (Abs(b) < 1.0e-10) Then

    atan3 = c

    Exit Function

  Else

    atan3 = c + Sgn(a) * Sgn(b) * (Abs(Atn(a / b)) - pidiv2)

  EndIf

End Function

  ''''''''''''''''''
  ''''''''''''''''''

Function vecmag(a())

  ' vector magnitude function

  ' input

  '  { a } = column vector ( 3 rows by 1 column )

  ' output

  '  vecmag = scalar magnitude of vector { a }

  vecmag = Sqr(a(1) * a(1) + a(2) * a(2) + a(3) * a(3))

End Function

  ''''''''''''''''''''
  ''''''''''''''''''''

Sub uvector(a(), b())

  ' unit vector subroutine

  ' input

  '  a = column vector (3 rows by 1 column)

  ' output

  '  b = unit vector (3 rows by 1 column)

  '''''''''''''''''''''''''''''''''''''''

  Local i As integer, amag

  amag = vecmag(a())

  For i = 1 To 3

    If (amag <> 0.0) Then

      b(i) = a(i) / amag

    Else

      b(i) = 0.0

    End If

  Next i

End Sub

  '''''''''''''''''''''
  '''''''''''''''''''''

Function vdot(a(), b())

  ' vector dot product function

  ' c = { a } dot { b }

  ' input

  '  n%    = number of rows
  '  { a } = column vector with n rows
  '  { b } = column vector with n rows

  ' output

  '  vdot = dot product of { a } and { b }

  ''''''''''''''''''''''''''''''''''''''''

  Local c = 0.0

  For i% = 1 To 3

    c = c + a(i%) * b(i%)

  Next i%

  vdot = c

End Function

  '''''''''''''''''''''''
  '''''''''''''''''''''''

Sub vcross(a(), b(), c())

  ' vector cross product subroutine

  ' { c } = { a } x { b }

  ' input

  '  { a } = vector a ( 3 rows by 1 column )
  '  { b } = vector b ( 3 rows by 1 column )

  ' output

  '  { c } = { a } x { b } ( 3 rows by 1 column )

  c(1) = a(2) * b(3) - a(3) * b(2)
  c(2) = a(3) * b(1) - a(1) * b(3)
  c(3) = a(1) * b(2) - a(2) * b(2)

End Sub

  ''''''''''''''''''''''''
  ''''''''''''''''''''''''

Sub matxvec(a(), b(), c())

  ' matrix/vector multiplication subroutine

  ' { c } = [ a ] * { b }

  ' input

  '  a  = matrix a ( 3 rows by 3 columns )
  '  b  = vector b ( 3 rows )

  ' output

  '  c = vector c ( 3 rows )

  ''''''''''''''''''''''''''

  Local s, i%, j%

  For i% = 1 To 3

    s = 0.0

    For j% = 1 To 3

      s = s + a(i%, j%) * b(j%)

    Next j%

    c(i%) = s

  Next i%

End Sub

  ''''''''''''''''''''''
  ''''''''''''''''''''''

Sub transpose(a(), b())

  ' matrix traspose subroutine

  ' input

  '  m = number of rows in matrix [ a ]
  '  n = number of columns in matrix [ a ]
  '  a = matrix a ( 3 rows by 3 columns )

  ' output

  '  b = matrix transpose ( 3 rows by 3 columns )

  '''''''''''''''''''''''''''''''''''''''''''''''

  Local i%, j%

  For i% = 1 To 3

    For j% = 1 To 3

      b(i%, j%) = a(j%, i%)

    Next j%

  Next i%

End Sub

  '''''''''''''''
  '''''''''''''''

Sub jd2str(jdutc)

  ' convert julian day to calendar date and UTC time

  ''''''''''''''''''''''''''''''''''''''''''''''''''

  gdate(jdutc, cmonth, day, year)

  Print "calendar date  ", month$(cmonth), " ", Str$(Int(day)), " ", Str$(year)

  Print " "

  thr0 = 24.0 * (day - Int(day))

  thr = Int(thr0)

  tmin0 = 60.0 * (thr0 - thr)

  tmin = Int(tmin0)

  tsec = 60.0 * (tmin0 - tmin)

  Print "UTC time       ", Str$(thr) + " hours " + Str$(tmin) + " minutes " + Str$(tsec, 0, 2) + " seconds"

End Sub

  '''''''''''''''''''
  '''''''''''''''''''

Sub deg2str(dd, dms$)

  ' convert decimal degrees to degrees,
  ' minutes, seconds string

  ' input

  '  dd = angle in decimal degrees

  ' output

  '  dms$ = string equivalent

  '''''''''''''''''''''''''''

  Local d1, d, m, s

  d1 = Abs(dd)

  d = Fix(d1)

  d1 = (d1 - d) * 60.0

  m = Fix(d1)

  s = (d1 - m) * 60.0

  If (dd < 0.0) Then

    If (d <> 0.0) Then

      d = -d

    ElseIf (m <> 0.0) Then

      m = -m

    Else

      s = -s

    End If

  End If

  dms$ = Str$(d) + " deg " + Str$(m) + " min " + Str$(s, 0, 2) + " sec"

End Sub