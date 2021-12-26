@echo off
setlocal
REM This bat dumps each parameters(%1 %2 %3 ...) seen within this bat itself.
REM -- Jimm Chen 2021.12.16

echo [0] %0
echo [~0] %~0
set n=1

if [%1]==[] exit /b
echo  [%n%] %1
echo [~%n%] %~1

:REPEAT
	shift
	set /a n=n+1
	if [%1]==[] exit /b
	echo  [%n%] %1
	echo [~%n%] %~1
	goto :REPEAT
