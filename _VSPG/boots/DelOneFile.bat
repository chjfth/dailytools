@echo off
setlocal EnableDelayedExpansion

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.
REM call :Echos START from %batdir%

: This is a function.
:DelOneFile

: Param1: the filepath to delete

if not exist "%~1" (
	call :Echos Already deleted: "%~1"
	exit /b 0
)

call :Echos del "%~1"

: Now delete a file and correctly reports ERRORLEVEL.
: Thanks to: https://stackoverflow.com/a/33403497/151453

> nul ver & for /F "tokens=*" %%# in ('del /Q "%~1" 2^>^&1 1^> nul') do (2> nul set =)

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
  %*
exit /b %ERRORLEVEL%

:EchoVar
  setlocal & set Varname=%~1
  call echo %_vspgINDENTS%[%batfilenam%] %Varname% = %%%Varname%%%
exit /b 0
