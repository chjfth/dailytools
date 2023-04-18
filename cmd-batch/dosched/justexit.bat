@echo off
setlocal EnableDelayedExpansion
set batfilenam=%~nx0
set batdir=%~dp0
set batdir=%batdir:~0,-1%

set exitcode=%~1

if not defined exitcode (
	set exitcode=0
)

REM Append a testfile log.
echo "%DATE% %TIME%" %batfilenam%  >> "%batdir%\justexit.timestamps.txt"


echo [%batfilenam%] Will exit with code %exitcode% .

exit /b %exitcode%
