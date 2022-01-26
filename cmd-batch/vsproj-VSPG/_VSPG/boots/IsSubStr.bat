@echo off
setlocal EnableDelayedExpansion
set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.

: This is a function.
: Find a substring in a given string, return result in %1.


:IsSubStr
REM Usage: 
REM call IsSubStr.bat OutputVar %Haystack% %Needle%
REM OutputVar=1, yes; =0, no
REM
REM Implementation: Replace Needle with empty string; if resulting string is NOT 
REM the same as Haystack, then Needle is found.
  
set Haystack=%~2
set Needle=%~3

if "%Haystack%" == "" (
	endlocal & ( set "%~1=0" )
	exit /b 0
)

if "%Needle%" == "" (
	call :Echos ERROR: Parameter Needle is empty.
	exit /b 3
)

REM The Env-var double expansion trick 
for /F %%i in ('echo any') do set Replaced=!Haystack:%Needle%=!

REM echo Replaced=%Replaced%

if "%Replaced%" == "%Haystack%" ( set "Found=0" ) else ( set "Found=1" )

endlocal & ( set "%~1=%Found%" )

exit /b 0


REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  echo %_vspgINDENTS%[%batfilenam%] %*
exit /b 0
