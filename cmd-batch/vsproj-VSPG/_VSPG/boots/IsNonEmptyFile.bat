@echo off
setlocal EnableDelayedExpansion

:Function: IsNonEmptyFile

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.

set infile=%~1

if "%infile%" == "" (
	call :Echos [ERROR] Missing input file as parameter.
	exit /b 4
)

call "%batdir%\IsFolder.bat" "%infile%"
if not errorlevel 1 (
	call :Echos [ERROR] Input path "%infile%" is a directory. I need a file.
	exit /b 4
)

REM Must reset ERRORLEVEL here(could have been set by IsFolder.bat)
call :SetErrorlevel 0

set size=0
for /f delims^=^ eol^= %%i in ("%infile%") do (
	set size=%%~zi
)
if errorlevel 1 exit /b 4

if "%size%" == "" (
	REM The given file does not exist, we give FALSE answer.
	exit /b 4
)

if "%size%" == "0" (
	REM Zero byte output, something must gone wrong.
	exit /b 4
) 

exit /b 0


REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  echo %_vspgINDENTS%[%batfilenam%] %*
exit /b 0

:EchoExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
exit /b 0

:SetErrorlevel
  REM Usage example:
  REM call :SetErrorlevel 4
exit /b %1
