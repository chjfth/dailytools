@setlocal
@echo off
set batdir=%~dp0
set batdir=%batdir:~0,-1%
pushd %batdir%

REM First parameter is the finish-delay seconds.
REM Remaining parameters are the "real" internal CMD command line to execute.
REM Limitation: Can only deal with 7 params to internal cmd, bcz `SHIFT` does not affect %* .

set DelaySeconds=%~1
if "%DelaySeconds%" == "" (
	set DelaySeconds=0
)

set JOBCMD_PATH=%~2
set JOBCMD_LOGFILE=%JOBCMD_PATH%.log

call :AssumeError

REM Ensuring the logfile can be created, trying 3 filenames.
REM Hint: If %JOBCMD_LOGFILE% can NOT be created, :AssumeError will remain effective.
ver > %JOBCMD_LOGFILE%
if not ERRORLEVEL 1 goto :LogfileOK
set JOBCMD_LOGFILE=%JOBCMD_PATH%.1.log
ver > %JOBCMD_LOGFILE%
if not ERRORLEVEL 1 goto :LogfileOK
set JOBCMD_LOGFILE=%JOBCMD_PATH%.2.log
ver > %JOBCMD_LOGFILE%
if ERRORLEVEL 1 (
  echo So bad, Cannot create logfile after trying as many times as %JOBCMD_LOGFILE%
  exit /b 145
)

:LogfileOK
echo Now datetime: %DATE% %TIME% >> %JOBCMD_LOGFILE%
echo. >> %JOBCMD_LOGFILE%

if "%DelaySeconds%" == "0" (
	call "%JOBCMD_PATH%" %3 %4 %5 %6 %7 %8 %9 >> "%JOBCMD_LOGFILE%" 2>&1
	
	REM Since we have redirected program output to file, there is 
	REM definitely no sense to delay here, no matter the program runs 
	REM with success or failure. So we exit right now.
	exit /b %ERRORLEVEL%
) 

call "%JOBCMD_PATH%" %3 %4 %5 %6 %7 %8 %9

set ERRCODE=%ERRORLEVEL%

if ERRORLEVEL 1 (
	echo.
	echo ==== ERROR OCCURRED! Please review. Press any key to dismiss. ====
	echo.
	pause
) else (
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
exit /b 144
