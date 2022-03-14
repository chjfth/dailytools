@echo off
setlocal EnableDelayedExpansion

:Function: GetParentDir

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.

REM Usage: 
REM call GetParentDir.bat varParent "c:\program files\d1\d2" 
REM Output varParent=c:\program files\d2
REM `varParent` can be any batch variable name user choose.

if "%~1" == "" (
	call :Echos [ERROR] Missing 1st parameter: varParent.
	exit /b 4
)

if "%~2" == "" (
	call :Echos [ERROR] Missing 2nd parameter: an input path.
	exit /b 4
)
set inpath=%~2

for %%g in ("%inpath%") do set parentdir=%%~dpg

set parentdir=!parentdir:~0,-1!

endlocal & (
	set "%~1=%parentdir%"
)

exit /b 0


REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  echo %_vspgINDENTS%[%batfilenam%] %*
exit /b 0

:EchoAndExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
  call %*
exit /b %ERRORLEVEL%

:SetErrorlevel
  REM Usage example:
  REM call :SetErrorlevel 4
exit /b %1
