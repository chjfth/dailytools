@echo off
setlocal EnableDelayedExpansion
set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.
call :Echos START from %batdir%

REM == debugging purpose ==
REM call :EchoVar _BuildConf_
REM call :EchoVar BuildConf
REM call :EchoVar PlatformName
REM call :EchoVar _ExeDllDir_
REM call :EchoVar ExeDllDir
REM call :EchoVar TargetName

REM =========================================================================
REM This bat copies VSIDE project output(EXE/DLL etc) to other target directories,
REM or clean(=delete) them from those target directories(called barns here). 
REM Env-var vspg_COPYORCLEAN_DO_CLEAN from outside determines copy-or-clean(1=clean).
REM 
REM Typical usage cases:
REM * Copy EXE/DLL to a remote machine shared folder, so to debug your program
REM   running on that remote machine (remote debugging).
REM * By copying EXE/DLL from different projects to a single barn, you collect those
REM   EXE/DLL to help create a complete set of software package.
REM 
REM Three variables control the copy operation implemented in this bat.
REM 
REM (1) AGILE_COPY_PATTERNS
REM
if "%PlatformShortName%" == "amd64" set PlatformShortName=x64

set AGILE_COPY_PATTERNS="%ExeDllDir%\%TargetFilenam%#%TargetName%-%PlatformShortName%.exe"
REM -- Notice the '#' above, I will copy KeyView2.exe to KeyView2-x86.exe and/or KeyView2-x64.exe .
REM    So that I can easily copy those output EXEs to D:\software_vmwin\wintools\KeyView .
REM 
REM		This sets what files (concrete filenames, separated by spaces) or 
REM     file-pattern (*.exe etc) to copy or clean. You can mix both.
REM	
REM		This can be absolute path or relative path. If relative path, it is relative
REM		to $(TargetDir) from VSIDE, also referred to as %ExeDllDir% in this bat.
REM	
REM		This can be a list of multiple file/file-patterns separated by spaces.
REM		It means you can have multiple barns, each barn gets one copy.
REM	
REM		NOTE: If a pattern/filepath has space char, you must wrap it in double-quotes.
REM	
REM	(2) AGILE_BARN_DIRS
REM 
set AGILE_BARN_DIRS="%ProjectDir%\%BuildConf%"
REM
REM		You can list multiple barn dirs, separated by spaces.
REM 
REM		Also, if a barn dir contains space char, you must wrap that dir-path in double-quotes.
REM	
REM Hint: You can set/modify above AGILE_xxx vars directly in this bat file, 
REM       or set them from parent environment.
REM 
REM (3) BARN_SUBDIR
REM     This creates extra subdir(s) inside a barn dir.
REM     Usually, we want to separate Debug/Release outputs, and separate Win32/x64 output,
REM     so this subdir specification comes to help.
REM     Hint: You don't need to surround BARN_SUBDIR's value with double-quotes, bcz
REM           when we pass it to CopyFilePatternsToDirs.bat, double-quotes is added there.
REM 
rem set BARN_SUBDIR=%PlatformName%\__%BuildConf%
REM 
REM =========================================================================

if not defined AGILE_BARN_DIRS (
	REM This is not considered error, it just means user do not want to do copy/clean now.
	call :Echos AGILE_BARN_DIRS is empty, nothing to copy.
	exit /b 0
)

if not defined AGILE_COPY_PATTERNS (
	call :Echos [ERROR] AGILE_COPY_PATTERNS is empty.
	exit /b 4
)

REM We do not check whether vspg_COPYORCLEAN_DO_CLEAN=1 here, bcz CopyFilePatternsToDirs.bat will take care of it.
REM 
call "%_vspg_bootsdir%\CopyFilePatternsToDirs.bat" "%ExeDllDir%" AGILE_COPY_PATTERNS AGILE_BARN_DIRS "%BARN_SUBDIR%"

exit /b %ERRORLEVEL%



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

:EchoVar
  setlocal & set Varname=%~1
  call echo %_vspgINDENTS%[%batfilenam%]%~2 %Varname% = %%%Varname%%%
exit /b 0

:SetErrorlevel
  REM Usage example:
  REM call :SetErrorlevel 4
exit /b %1
