@echo off
setlocal EnableDelayedExpansion
REM This bat file boots actual VSPG bat files. It should be kept minimalist. 
REM This file exists to ease VSPG developer to temporarily switch local [boots] dir to [boots-dev].
REM so that a new VSPG version can be developed in side-by-side with the actual project using VSPG.
REM
REM Operation hint: Create a soft link boots-dev pointing to VSPG original source.
REM
REM 	cd D:\some\big work\_VSPG
REM 	if exists ".\boots\VSPG-Boots.bat" (echo Condition OK.)
REM 	mklink /j boots-dev "D:\gitw\dailytools\cmd-batch\vsproj-VSPG\_VSPG\boots"
REM 	
REM Now, "D:\some\big work\_VSPG\boots\VSPG-Boots.bat" will next-step call content in
REM      "D:\some\big work\_VSPG\boots-dev\VSPG-StartBat9_.bat" 
REM instead of
REM	     "D:\some\big work\_VSPG\boots\VSPG-StartBat9_.bat"
REM .

REM set batfilenam to .bat filename(no directory prefix)
set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%

set _vspg_bootsdir=%~dp0
set _vspg_bootsdir=%_vspg_bootsdir:~0,-1%
REM

call "%_vspg_bootsdir%\PathSplit.bat" "%_vspg_bootsdir%" ParentDir Subdir

set "_vspg_userbatdir=%ParentDir%"


if exist "%ParentDir%\boots-dev\VSPG-StartBat9_.bat" (
	REM Override _vspg_bootsdir to be the -dev one.
	set "_vspg_bootsdir=%ParentDir%\boots-dev"
	set "_vspg_use_dev=1"
	call :Echos Detected [boots-dev]. Will use VSPG from "!_vspg_bootsdir!" .
)

call "%_vspg_bootsdir%\VSPG-StartBat9_.bat" %*

if errorlevel 1 exit /b 4

REM ======== copy [boots-dev] to [boots] if necessary ========

if "%_vspg_use_dev%" == "" goto :DONE_COPY_DEV_TO_USER
if "%vspg_COPY_DEV_TO_USER%" == "" goto :DONE_COPY_DEV_TO_USER

call :Echos Copying [boots-dev] content to user [boots] ...

copy "%_vspg_bootsdir%\*.bat"   "%batdir%" 
if errorlevel 1 exit /b 4
copy "%_vspg_bootsdir%\*.props" "%batdir%" 
if errorlevel 1 exit /b 4

:DONE_COPY_DEV_TO_USER

exit /b 0

REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  REM This function preserves %ERRORLEVEL% for the caller,
  REM and, LastError does NOT pollute the caller.
  setlocal & set LastError=%ERRORLEVEL%
  echo %_vspgINDENTS%[%batfilenam%] %*
exit /b %LastError%

:EchoAndExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
  call %*
exit /b %ERRORLEVEL%

