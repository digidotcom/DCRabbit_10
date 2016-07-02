@echo off

rem Automate build of BIOS/somefile.bin
rem Tom Collins - September 2009
rem lightly tested (for coldloadserflash)

rem Default is to triplet what's built
set TRIPLET=1

:menu
echo Recompile Pilot and Coldloader files
echo ------------------------------------
echo    1) PreColdLoad
echo    2) CheckRamCS0
echo    3) CheckCS04mem

echo    4) ColdLoad
echo    5) ColdLoad16bit
echo    6) ColdLoadSerFlash

echo    7) Pilot
echo    8) Pilot16bit
echo    9) PilotSerFlash

echo    X) Exit

set /p choice=Choice:
rem Take first character of input
set choice=%choice:~0,1%

if "%choice%"=="X" goto exit
if "%choice%"=="x" goto exit
if "%choice%"=="1" goto precoldload
if "%choice%"=="2" goto checkramcs0
if "%choice%"=="3" goto checkcs04mem
if "%choice%"=="4" goto coldload
if "%choice%"=="5" goto coldload16bit
if "%choice%"=="6" goto coldloadserflash
if "%choice%"=="7" goto pilot
if "%choice%"=="8" goto pilot16bit
if "%choice%"=="9" goto pilotserflash

echo Invalid Choice
goto menu

:precoldload
set FILE=precoldload
goto defproj

:checkramcs0
set FILE=checkramcs0
goto defproj

:checkcs04mem
set FILE=checkcs04mem
set PROJ=compile_coldload
goto build

:coldload
set FILE=coldload
goto defproj

:coldload16bit
set FILE=coldload16bit
goto defproj

:coldloadserflash
set FILE=coldloadserflash
set PROJ=compile_coldload_serflash
goto build

:pilot
set FILE=pilot
set TRIPLET=0
goto defproj

:pilot16bit
set FILE=pilot16bit
set PROJ=compile_pilot16
set TRIPLET=0
goto build

:pilotserflash
set FILE=pilotserflash
set PROJ=pilotserflash
set TRIPLET=0
goto build

:defproj
set PROJ=compile_%FILE%

:build
echo Recompiling %FILE%.c
..\distribcommon\dccl_cmp.exe %FILE%.c -pf %PROJ%.dcp
if %TRIPLET%==1 (
	echo Using triplet.exe to create BIOS\%FILE%.bin
	triplets\triplets.exe %FILE% ..\distribcommon\bios\%FILE%
) else (
	echo Copying %FILE%.bin to BIOS\%FILE%.bin
	copy %FILE%.bin ..\distribcommon\bios\%FILE%.bin
)
:exit
