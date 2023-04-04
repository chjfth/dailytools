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

set DelaySecondsChar0=%DelaySeconds:~0,1%

call :NowTimeAsFilename DatetimeNow

set JOBCMD_PATH=%~2
set JOBCMD_LOGFILE=%JOBCMD_PATH%.%DatetimeNow%.log

if not "%DelaySecondsChar0%" == "0" goto TEST_LOGFILE_DONE

REM Ensuring the logfile can be created.
REM Hint: If %JOBCMD_LOGFILE% can NOT be created, :AssumeError will remain effective.
call :AssumeError
ver > "%JOBCMD_LOGFILE%"

if not !ERRORLEVEL! == 0 (
	echo.
	call :Echos [ERROR] Cannot create logfile: "%JOBCMD_LOGFILE%"
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

:TEST_LOGFILE_DONE

if "%DelaySecondsChar0%" == "0" (
	
	call :PrintCurEnv
	call :PrintCurEnv >> "%JOBCMD_LOGFILE%" 2>&1
	
	REM In this case, we will write user-bat's output to log-file, not to console.
	
	REM But, If this case is run by developer interact-mode, user will 
	REM still see a CMD window popup on screen. To make this CMD window message-friendly,
	REM I will print "prolog" and "epilog" to user in front of the screen.
	
	call :Echos [NOTE] Remaining program output will not be displayed in this console window.
	call :Echos ...... They will be saved to log file:
	call :Echos ...... "%JOBCMD_LOGFILE%"

	echo.
	echo "%JOBCMD_PATH%" %3 %4 %5 %6 %7 %8 %9
	echo.
	call "%JOBCMD_PATH%" %3 %4 %5 %6 %7 %8 %9 >> "%JOBCMD_LOGFILE%" 2>&1
	
	REM Now the user-bat has finished.
	REM From user-bat author's perspective, the best behavior is:
	REM * If it had run in (background)service-mode, quit immediately, no matter success/failure.
	REM   human user can later check the log-file to know the result.
	REM * If it had run in (foreground)interact-mode (user can see CMD window on screen), 
	REM * make a pause so that human-user can see the result.
	REM
	REM However, from bat-file code's perspective, it does NOT know whether the current bat 
	REM is run in background or foreground(which is controlled from taskschd.msc option).
	REM 
	REM    run foreground = "Run only when user is logged on"
	REM    run background = "Run whether(no matter) user is logged on or not"
	REM 
	REM So, my decision here is:
	REM * If user-bat runs with success, quit immediately. That is, human user sees 
	REM   CMD window appearing and vanishes very quickly.
	REM * If user-bat runs with error, I will have the CMD window delay 5 seconds then quit.
	REM   During this 5 seconds, if run foreground, human user will see on screen:
	REM   - Datetime this bat is launched..
	REM   - The log-filename (%JOBCMD_LOGFILE%) that will hold user command's console output.
	REM   - User command(bat/exe) that was executed.
	REM   - User command(bat/exe) execution result, success or failure.
	REM   During this 5 seconds, if run foreground, human user will NOT see on screen:
	REM   - User commands's console output, because that have been redirected to %JOBCMD_LOGFILE% .

	set ERRCODE=!ERRORLEVEL!
	
	if !ERRCODE! == 0 (
		call :Echos User-bat execution success.
		
		if not "%DelaySeconds%" == "00" (
			call :Echos Deleting success logfile: "%JOBCMD_LOGFILE%"
			del "%JOBCMD_LOGFILE%"
		)
		
		REM Add 1 second delay so that human user, if run interact-mode, can notice it.
		call :Delay 1
	) else (
		echo.
		call :Echos ##########################################################
		call :Echos User-bat execution error, please check logfile for reason.
		call :Echos Delay 5 seconds then quit.
		call :Echos ##########################################################
		call :Delay 5
	)

	exit /b %ERRCODE%
	
) else (

	call :PrintCurEnv
	
	echo "%JOBCMD_PATH%" %3 %4 %5 %6 %7 %8 %9
	echo.
	call "%JOBCMD_PATH%" %3 %4 %5 %6 %7 %8 %9
	
	set ERRCODE=!ERRORLEVEL!

	if !ERRCODE! == 0 (
		echo.
		call :Echos Delay %1 seconds before quit...
		call :Delay %DelaySeconds%
	) else (
		echo.
		call :Echos ==== ERROR OCCURRED! Please review. Press any key to dismiss. ====
		echo.
		pause
	)
	
	exit /b %ERRCODE%
)

REM SHOULD NOT GET HERE.
exit /b 44



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

:NowTimeAsFilename
  REM Example:
  REM 
  REM   call :TimeStrAsFilename varTimestr
  REM 
  REM On return, caller's varTimestr var will contain a string representing
  REM current time, like: "04-04-2023 Tue 20.53.32"
  REM Note: The output string deliberately has space in it, so caller 
  REM must be able to deal with space-char in filename. This is a situation
  REM that the user MUST cope with, bcz, the %DATE% expanded string 
  REM may have already contained space-chars, under some user-locale selection
  REM (in intl.cpl).

  setlocal
  REM We create the string from env-var DATE and TIME, and at the sametime,
  REM replacing invalid file-system chars to valid ones.
  set timestr=%DATE:/=-% %TIME::=.%
  REM 
  REM Remove "percent-second" tail from %TIME%
  set timestr=%timestr:~0,-3%
  endlocal & ( set "%~1=%timestr%" )
exit /b 0
