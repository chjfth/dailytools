@echo off
setlocal EnableDelayedExpansion
set batfilenam=%~nx0
set batdir=%~dp0
set batdir=%batdir:~0,-1%

REM First parameter is the finish-delay seconds.
REM Remaining parameters are the "real" internal CMD command line to execute.
REM Limitation: Can only deal with 7 params to internal cmd, bcz `SHIFT` does not affect %* .

set DelaySeconds=%~1
if "%DelaySeconds%" == "" (
	set DelaySeconds=0
)

set JOBCMD_PATH=%~2
set JOBCMD_LOGFILE=%JOBCMD_PATH%.log
set JOBCMD_LOGFILE0=%JOBCMD_LOGFILE%

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
	echo.
	call :Echos [ERROR] Cannot create logfile: "%JOBCMD_LOGFILE0%"
	call :Echos ....... The user-bat will NOT be launched: "%JOBCMD_PATH%"
	echo.
	if "!DelaySeconds!" == "0" (
		call :Echos ##########################################################
		call :Echos Delay 5 seconds then quit.
		call :Echos ##########################################################
		call :Delay 5
	) else (
		call :Echos ==== Press any key to dismiss. ====
		pause
	)
	exit /b 4
)

:LogfileOK

if "%DelaySeconds%" == "0" (
	REM In thise case, we will write user-bat's output to log-file, not to console.
	
	REM But, If this case is run temporarily in log-foreground-mode, user will 
	REM still see a CMD window on screen. To make this CMD window message-friendly,
	REM I will print "prolog" and "epilog" to user in front of the screen.
	
	call :Echos [NOTE] Remaining program output will not be display in this console window.
	call :Echos ...... They will be saved to log file: "%JOBCMD_LOGFILE%"

	call :PrintCurEnv >> "%JOBCMD_LOGFILE%" 2>&1
	
	@echo on
	call "%JOBCMD_PATH%" %3 %4 %5 %6 %7 %8 %9 >> "%JOBCMD_LOGFILE%" 2>&1
	
	REM Now the user-bat has finished.
	REM From bat author's perspective, the best behavior is:
	REM * If it had run in log-background-mode, quit immediately, no matter success/failure.
	REM   human user can later check the log-file to know the result.
	REM * If it had run in log-foreground-mode(user can see CMD window on screen), 
	REM * make a pause so that human-user can see the result.
	REM
	REM However, from bat-file code's perspective, we do not know whether our bat 
	REM is run in log-background-mode or log-foreground mode, which is controlled from 
	REM taskschd.msc option:
	REM    log-foreground-mode = "Run only when user is logged on"
	REM    log-background-mode = "Run whether(no matter) user is logged on or not"
	REM So, my decision here is:
	REM * If user-bat runs with success, quit immediately. That is, human user sees 
	REM   CMD window appearing and vanishes very quickly.
	REM * If user-bat runs with error, I will have the CMD window delay 5 seconds then quit.

	@echo off
	set ERRCODE=!ERRORLEVEL!
	
	if !ERRCODE!==0 (
		call :Echos User-bat execution success.
		exit /b 0
	) else (
		echo.
		call :Echos ##########################################################
		call :Echos User-bat execution error, please check logfile for reason.
		call :Echos Delay 5 seconds then quit.
		call :Echos ##########################################################
		call :Delay 5
		exit /b %ERRCODE%
	)
	
) 

call :PrintCurEnv
call "%JOBCMD_PATH%" %3 %4 %5 %6 %7 %8 %9

set ERRCODE=%ERRORLEVEL%

if ERRORLEVEL 1 (
	echo.
	call :Echos ==== ERROR OCCURRED! Please review. Press any key to dismiss. ====
	echo.
	pause
) else (
	echo.
	call :Echos Delay %1 seconds before quit...
	call :Delay %DelaySeconds%
)


exit /b %ERRCODE%

REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  REM This function preserves %ERRORLEVEL% for the caller,
  REM and, LastError does NOT pollute the caller.
  setlocal & set LastError=%ERRORLEVEL%
  echo [%batfilenam%] %*
exit /b %LastError%

:EchoAndExec
  echo [%batfilenam%] EXEC: %*
  call %*
exit /b %ERRORLEVEL%

:Delay
ping 127.0.0.1 -n %1 -w 1000 > nul
ping 127.0.0.1 -n 2 -w 1000 > nul
exit /b

:AssumeError
exit /b 144

:PrintCurEnv 
  call :Echos Now datetime: %DATE% %TIME%
  call :Echos Working dir : %CD%
  echo.
exit /b 0
