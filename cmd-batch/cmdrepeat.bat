@echo off
setlocal EnableDelayedExpansion
set batdir=%~dp0
set batdir=%batdir:~0,-1%

set inputcmd=%*
set param1=%~1

if "%param1%" == "" (
	echo Missing input command.
	exit /b 4
)

if not defined SLEEP_SECONDS_BETWEEN_REPEAT (
	set SLEEP_SECONDS_BETWEEN_REPEAT=3
)

set /A ii=0

:REPEAT

set /A ii=%ii%+1
echo [REPEAT %ii%] Now %DATE% %TIME%

call %*

set exitcode=%ERRORLEVEL%

timeout /t %SLEEP_SECONDS_BETWEEN_REPEAT%

goto :REPEAT

exit /b %ERRORLEVEL%
