@setlocal
@echo off
set batdir=%~dp0
set batdir=%batdir:~0,-1%
pushd %batdir%

REM First parameter is the finish-delay seconds.
REM Remaining parameters are the "real" internal CMD command line to execute.
REM Limitation: Can only deal with 7 params to internal cmd, bcz `SHIFT` does not affect %* .

set DelaySeconds=%~1
set JOBCMD_PATH=%2

call :AssumeError

if "%DelaySeconds%" == "" (
	call %JOBCMD_PATH% %3 %4 %5 %6 %7 %8 %9 > %JOBCMD_PATH%.log 2>&1
) else (
	call %JOBCMD_PATH% %3 %4 %5 %6 %7 %8 %9
)

set ERRCODE=%ERRORLEVEL%

if ERRORLEVEL 1 if not "%DelaySeconds%" == "" (
	call :Delay %DelaySeconds%
)

exit /b %ERRCODE%

REM ========== Functions Below ==========

:Delay
echo Delay %1 seconds before quit...
ping 127.0.0.1 -n %1 -w 1000 > nul
ping 127.0.0.1 -n 2 -w 1000 > nul
exit /b

:AssumeError
exit /b 14
