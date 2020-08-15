@echo off
@setlocal
set batdir=%~dp0
set batdir=%batdir:~0,-1%
pushd %batdir%

REM ~~~~~~~~~~~~~~~~~~~~~~~~~~
REM Use these .bat files to backup and lab-restore my Confluence site.
REM Run it in my VM T:\_vms\CF673-export\Ws2012-CF-6.7.3.vmx
REM And it can be launched by dosched0.bat via Windows Task Scheduler.
set CFDIR=d:\CF-6.7
set ATT_NODS=data\attachments
set CFATTDIR=%CFDIR%\%ATT_NODS%
REM ~~~~~~~~~~~~~~~~~~~~~~~~~~

set OLD_PATH=%PATH%
PATH=D:\cygwin64\bin;%PATH%

bash.exe -c './do-irsync.sh'

if ERRORLEVEL 1 goto :END

PATH=%OLD_PATH%

REM =============================
REM ==== when backup success ====
REM =============================

echo OK. Confluence backup pulled from remote machine. Will restore in 5 seconds.
call :Delay 5

call :IsExeRunning _ret java.exe
if %_ret% == YES (
	set ConfluenceWasRunning=1
	echo Will restart confluence after backup restore.
) else (
	echo Will NOT restart confluence after backup restore, bcz Confluence is NOT running now.
)

REM == Try to stop Confluence instance. ==

set countdown=10
:Loop
  echo Killing and waiting java.exe to quit (%countdown%...)
  taskkill /F /IM "java.exe"

  call :Delay 1

  call :IsExeRunning _ret java.exe
  if %_ret% == NO goto JAVA_STOPPED

  set /a countdown-=1
  if %countdown% == 0 (
  	echo Cannot stop java.exe, so I can not continue.
  	exit /b 4
  )
  
goto :Loop

:JAVA_STOPPED
echo Java.exe processes all quitted. Now start restoring confluence data.

REM Read finishdir.txt content into cmd var, and replace / with \
set /p FINISHDIR=<finishdir.txt
set FINISHDIR=%FINISHDIR:/=\%

if exist %CFATTDIR% rd %CFATTDIR%
set CMD_MKLINK=mklink /j  %CFATTDIR%  %FINISHDIR%\%ATT_NODS%
%CMD_MKLINK%
if ERRORLEVEL 1 (
	echo mklink command fail:
	echo     %CMD_MKLINK%
	goto :END
)

REM == Restore database ==

set PGPASSWORD=pgchj
dropdb -U postgres --if-exists cf2018
if ERRORLEVEL 1 (
	echo PostgresQL dropdb fail!!! Cannot continue.
	goto :END
)
echo Well, dropdb done, wait 5 seconds then pg_restore...
call :Delay 5
pg_restore -v -U postgres -C -d postgres %FINISHDIR%\cf2018.pgdump
if ERRORLEVEL 1 (
	echo PostgresQL pg_restore execution fail!!!
	goto :END
)

echo Great. Confluence attachments and database restore success.

:RESTART_CONFLUENCE
set CFSTART_BATDIR=%CFDIR%\atlassian-confluence-6.7.3\bin
if "%ConfluenceWasRunning%" == "1" (
	echo Now restart Confluence at %CFSTART_BATDIR%
	pushd %CFSTART_BATDIR%
	call startup.bat
	if ERRORLEVEL 1 (
		echo Bad! Cannot restart Confluence startup.bat !!!
		exit /b 4
	)
)

:END
exit /b %ERRORLEVEL%

REM ========== Functions Below ==========

:Delay
ping 127.0.0.1 -n %1 -w 1000 > nul
ping 127.0.0.1 -n 2 -w 1000 > nul
exit /b

:AssumeError
exit /b 14

:IsExeRunning
tasklist /FI "IMAGENAME eq %~2" | findstr /I "%~2" > NUL
if ERRORLEVEL 1 (
    set %~1=NO
) else (
    set %~1=YES
)
