@echo off
setlocal EnableDelayedExpansion
set batfilenam=%~nx0
set batdir=%~dp0
set batdir=%batdir:~0,-1%

call :Echos This file creates a bunch of testing scheduled-tasks that can be used to verify DoSched bat wrapper's functionality.

set userpasswd=%~1
if not defined userpasswd (
	call :Echos Current user password must be provided as parameter, so that service-task can be created.
	exit /b 4
)

echo.
call :Echos Creating task DoSched3 and exit with 0.
call :EchoAndExec schtasks /Create /TN "DoSched-test\DoSched3-exit-0" /SC ONCE /ST 23:59 /TR "'%batdir%\dosched3.bat' '%batdir%\justexit.bat' 0"
if not !ERRORLEVEL! == 0 exit /b 4


echo.
call :Echos Create task DoSched3 and exit with 4.
call :EchoAndExec schtasks /Create /TN "DoSched-test\DoSched3-exit-4" /SC ONCE /ST 23:59 /TR "'%batdir%\dosched3.bat' '%batdir%\justexit.bat' 4"
if not !ERRORLEVEL! == 0 exit /b 4


echo.
call :Echos Creating task DoSched0 and exit with 0.
call :EchoAndExec schtasks /Create /TN "DoSched-test\DoSched0-exit-0" /SC ONCE /ST 23:59 /TR "'%batdir%\dosched0.bat' '%batdir%\justexit.bat' 0"
if not !ERRORLEVEL! == 0 exit /b 4

echo.
call :Echos Create task DoSched0 and exit with 4.
call :EchoAndExec schtasks /Create /TN "DoSched-test\DoSched0-exit-4" /SC ONCE /ST 23:59 /TR "'%batdir%\dosched0.bat' '%batdir%\justexit.bat' 4"
if not !ERRORLEVEL! == 0 exit /b 4

REM Create service-style task.

set OPT_TASKUSER=/RU "%USERDOMAIN%\%USERNAME%" /RP "%userpasswd%" /NP

echo.
call :Echos Creating task DoSched0 service and exit with 0.
call :EchoAndExec schtasks /Create /TN "DoSched-test\DoSched0-service-exit-0" %OPT_TASKUSER%  /SC ONCE /ST 23:59 /TR "'%batdir%\dosched0.bat' '%batdir%\justexit.bat' 0"
if not !ERRORLEVEL! == 0 exit /b 4

echo.
call :Echos Creating task DoSched0 service and exit with 4.
call :EchoAndExec schtasks /Create /TN "DoSched-test\DoSched0-service-exit-4" %OPT_TASKUSER%  /SC ONCE /ST 23:59 /TR "'%batdir%\dosched0.bat' '%batdir%\justexit.bat' 4"
if not !ERRORLEVEL! == 0 exit /b 4


REM DoSched00 : Always preserve logfile, no matter user-bat success or fail.

echo.
call :Echos Creating task DoSched00 and exit with 0.
call :EchoAndExec schtasks /Create /TN "DoSched-test\DoSched00-exit-0" /SC ONCE /ST 23:59 /TR "'%batdir%\dosched00.bat' '%batdir%\justexit.bat' 0"
if not !ERRORLEVEL! == 0 exit /b 4

REM Now a failure case, user-bat does not exist.

echo.
call :Echos Creating task User-bat-missing.
call :EchoAndExec schtasks /Create /TN "DoSched-test\User-bat-missing" /SC ONCE /ST 23:59 /TR "'%batdir%\dosched0.bat' '%batdir%\no-such.bat' xxyyzz"
if not !ERRORLEVEL! == 0 exit /b 4

exit /b 0



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

