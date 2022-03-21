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
REM OutputVar=1, yes
REM OutputVar=0, no
REM
REM Implementation: Replace Needle with empty string; if resulting string is NOT 
REM the same as Haystack, then Needle is found.

if "%~1" == "" (
	call :Echos [ERROR] Missing parameters.
	exit /b 4
)

set Haystack=%~2
set Needle=%~3

if "%Haystack%" == "" (
	call :Echos [ERROR] Missing Haystack parameter.
	endlocal & ( set "%~1=0" )
	exit /b 0
)

if "%Needle%" == "" (
	call :Echos [ERROR] Missing Needle parameter.
	exit /b 3
)

if not "%Needle%" == "*" goto :NEXT1

REM Reason not to use `if` block: 
REM https://stackoverflow.com/questions/71493157/delaydexpansion-and-endlocal-conflict-any-solution

	call :IsAsteriskInStr Found "%Haystack%"
	endlocal & ( set %~1=%Found% )
	exit /b 0

:NEXT1

call :IsAsteriskInStr isAster "%Needle%"
if "%isAster%" == "1" (
	call :Echos [ERROR] Sorry, I cannot cope with the case that Needle parameter contains a "*", your input Neele is: "%Needle%"
	exit /b 4
)

REM The Env-var double expansion trick 
CALL set Replaced=%%Haystack:%Needle%=_%%


REM echo Replaced=%Replaced%

if "%Replaced%" == "%Haystack%" ( set "Found=0" ) else ( set "Found=1" )

endlocal & ( set "%~1=%Found%" )

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

:IsAsteriskInStr
REM Check whether * is in input-string.
REM Thanks: https://stackoverflow.com/a/51390405/151453
REM Usage:
REM    call :IsAsteriskInStr OutputVar %Haystack%
REM OutputVar=1, yes
REM OutputVar=0, no
  setlocal
  for /f "tokens=1,* delims=*" %%a in ("%~2") do (set replaced=%%a%%b)
  if "%replaced%" == "%~2" (
    REM not found
    endlocal & (set "%~1=0")
  ) else (
    REM Found
    endlocal & (set "%~1=1")
  )
exit /b 0
