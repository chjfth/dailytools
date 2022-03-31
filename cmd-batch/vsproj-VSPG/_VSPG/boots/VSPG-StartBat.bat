@echo off
setlocal EnableDelayedExpansion
REM Usage: This .bat is to be called from Visual Studio VSPG_PreBuild/VSPG_PostBuild target,
REM so that we can write complex batch programs in this .bat or its subbat-s. 
REM This replaces the pre-year-2000 old way of jamming all .bat statements into VC's stock 
REM Prebuild/Postbuild events.

REM set batfilenam to .bat filename(no directory prefix)
set batfilenam=%~n0%~x0
set bootsdir=%~dp0
set bootsdir=%bootsdir:~0,-1%
REM Use PathSplit to get parent directory of bootsdir.
call "%bootsdir%\GetParentDir.bat" userbatdir "%bootsdir%"
set _vspgINDENTS=%_vspgINDENTS%.
REM
set SubworkBatfile=%~1
set SubworkBatpath=%bootsdir%\%SubworkBatfile%


call "%bootsdir%\VSPG-version.bat" vspgver

call :EchosV1 [VSPG version %vspgver%] started as: "%bootsdir%\%batfilenam%"


if defined vspg_DO_SHOW_VERBOSE (
  call :EchoVar SubworkBatpath
)


if not exist "%SubworkBatpath%" (
  call :Echos [INTERNAL-ERROR] SubworkBatpath NOT found: "%SubworkBatpath%"
  call :SetErrorlevel 4
  exit /b 4
)

REM ==== Prepare directory search list for VSPU-StartEnv.bat.

call :GetParentDir ProjectDir_up "%ProjectDir%"
call :GetParentDir ProjectDir_upup "%ProjectDir_up%"

set SubbatSearchDirsWideToNarrow=^
  "%userbatdir%"^
  "%SolutionDir%"^
  "%ProjectDir_upup%"^
  "%ProjectDir_up%"^
  "%ProjectDir%"

REM ======== Loading User Env-vars ======== 

REM This is a greedy search, bcz user may want to accumulate env-vars from outer env.
REM But if user does not like some env-var from outer env, he can override it(or clear it) 
REM from inner env explicitly.
REM In one word, the search order is from wide to narrow.

call "%bootsdir%\SearchAndExecSubbat.bat" Greedy1 VSPU-StartEnv.bat "" %SubbatSearchDirsWideToNarrow% 
if errorlevel 1 exit /b 4

REM ==== Prepare directory search list for other .bat-s.

REM From VSPU-StartEnv.bat, user can append new search dirs in vspg_USER_BAT_SEARCH_DIRS, so that they will be searched.

set SubbatSearchDirsNarrowToWide=%vspg_USER_BAT_SEARCH_DIRS% "%ProjectDir%" "%SolutionDir%" "%userbatdir%"



REM ======== call VSPG-Prebuild8.bat or VSPG-Postbuild8.bat ======== 
REM ====== which one to call is determined by SubworkBatfile =======

call "%bootsdir%\SearchAndExecSubbat.bat" Greedy0 "%SubworkBatfile%" "" "%bootsdir%"
if errorlevel 1 exit /b 4


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

:EchosV1
  REM echo %* only when vspg_DO_SHOW_VERBOSE=1 .
  setlocal & set LastError=%ERRORLEVEL%
  if not defined vspg_DO_SHOW_VERBOSE goto :_EchosV1_done
  echo %_vspgINDENTS%[%batfilenam%]# %*
:_EchosV1_done
exit /b %LastError%

:EchoAndExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
  call %*
exit /b %ERRORLEVEL%

:EchoVar
  setlocal & set Varname=%~1
  call echo %_vspgINDENTS%[%batfilenam%] %Varname% = %%%Varname%%%
exit /b 0

:SetErrorlevel
  REM Usage example:
  REM call :SetErrorlevel 4
exit /b %1

:Touch
	REM Touch updates a file's modification time to current.
	REM NOTE: No way to check for success/fail here. So, call it only when 
	REM you have decided to fail the whole bat.
	
	copy /b "%~1"+,, "%~1" >NUL 2>&1
exit /b %ERRORLEVEL%

:GetParentDir
  REM Example
  REM
  REM   call :GetParentDir outputvar "c:\program files\d1\d2"
  REM 
  REM Return:
  REM 
  REM   outputvar=c:\program files\d2
  setlocal
  if "%~1"=="" exit /b 4
  for %%g in ("%~2") do set parentdir=%%~dpg
  endlocal & ( set "%~1=%parentdir:~0,-1%" )
exit /b 0
