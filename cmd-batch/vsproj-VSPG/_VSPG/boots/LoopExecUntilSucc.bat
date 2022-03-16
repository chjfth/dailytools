@echo off
setlocal EnableDelayedExpansion
set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.

: This is a function.

:LoopExecUntilSucc
REM Usage: 
REM call LoopExec.bat #10# commands params ...
REM
REM The first param determines how many times to loop.
REM On first success(exitcode=0), this bat quits.
REM 

set param1=%~1
set charhead=%param1:~0,1%
set chartail=%param1:~-1%

if "%param1%" == "" (
	call :Echos [ERROR] Missing first parameter as loop-count.
	exit /b 4
)

if "%~2" == "" (
	call :Echos [ERROR] Missing parameter: empty command to execute.
	exit /b 4
)

if not "%charhead%" == "#" (
	call :Echos [ERROR] First char of first parameter is not #
	exit /b 4
)
if not "%chartail%" == "#" (
	call :Echos [ERROR] Final char of first parameter is not #
	exit /b 4
)

set LoopCount=%param1:~1,-1%

set _LoopCount_=#%LoopCount%#
set cmdexec=%*

for %%g in (%cmdexec%) do set cmdexec=!cmdexec:%_LoopCount_%=!

REM -- Now we have stripped away %1 from the passed-in command line,
REM %cmdexec% is the very command to execute.

set /a NowCount=1

:Loop

call :EchoAndExec1 %cmdexec%
if not errorlevel 1 (
	exit /b 0
)

if "%NowCount%" == "%LoopCount%" exit /b 4

set /a NowCount=%NowCount%+1

call :Echos Will retry in 2 seconds...
call "%batdir%\DelaySeconds.bat" 2

goto :Loop


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

:EchoAndExec1
  echo %_vspgINDENTS%[%batfilenam%] %NowCount%/%LoopCount% EXEC: %*
  call %*
exit /b %ERRORLEVEL%
