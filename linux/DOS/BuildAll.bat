@echo off
SET WATCOM=C:\WATCOM
SET PATH=%WATCOM%\BINNT;%WATCOM%\BINW;%PATH%
SET EDPATH=%WATCOM%\EDDAT
SET INCLUDE=%WATCOM%\H;%WATCOM%\H\NT;..\MMBASIC;Source

DEL /q MMBasic.exe

echo Compiling Source\Main.c
WCC386 /mf /zq /3s /dDOS "Source\Main.c"
IF %ERRORLEVEL% NEQ 0 goto abortexit

echo Compiling Source\DOS_Misc.c
WCC386 /mf /zq /3s /dDOS "Source\DOS_Misc.c"
IF %ERRORLEVEL% NEQ 0 goto abortexit

echo Compiling Source\File_IO.c
WCC386 /mf /zq /3s /dDOS "Source\File_IO.c"
IF %ERRORLEVEL% NEQ 0 goto abortexit

echo Compiling Source\Memory.c
WCC386 /mf /zq /zev /3s /dDOS "Source\Memory.c"
IF %ERRORLEVEL% NEQ 0 goto abortexit

echo Compiling Source\Editor.c
WCC386 /mf /zq /zev /3s /dDOS "Source\Editor.c"
IF %ERRORLEVEL% NEQ 0 goto abortexit

echo Compiling Source\Serial.c
WCC386 /mf /zq /zev /3s /dDOS "Source\Serial.c"
IF %ERRORLEVEL% NEQ 0 goto abortexit

echo Compiling ..\MMBASIC\Commands.c
WCC386 /mf /zq /zev /3s /dDOS "..\MMBASIC\Commands.c"
IF %ERRORLEVEL% NEQ 0 goto abortexit

echo Compiling ..\MMBASIC\Functions.c
WCC386 /mf /zq /3s /dDOS "..\MMBASIC\Functions.c"
IF %ERRORLEVEL% NEQ 0 goto abortexit

echo Compiling ..\MMBASIC\Operators.c
WCC386 /mf /zq /3s /dDOS "..\MMBASIC\Operators.c"
IF %ERRORLEVEL% NEQ 0 goto abortexit

echo Compiling ..\MMBASIC\MMBasic.c
WCC386 /mf /zq /zev /3s /dDOS "..\MMBASIC\MMBasic.c"
IF %ERRORLEVEL% NEQ 0 goto abortexit


WLINK system nt name MMBasic.exe file Commands.obj file File_IO.obj file Functions.obj file Main.obj file MMBasic.obj file Operators.obj file Memory.obj file Editor.obj file Serial.obj file DOS_Misc.obj library CLIB3S.LIB library MATH387S.LIB
IF %ERRORLEVEL% NEQ 0 goto abortexit

echo xxx >xxx.err
DEL /q *.err
DEL /q *.obj

echo.
echo Build successful - no errors
exit

:abortexit
echo.
echo Aborting build