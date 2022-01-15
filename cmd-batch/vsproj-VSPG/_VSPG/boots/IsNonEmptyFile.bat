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

REM Check that it is not a folder.
REM Thanks: https://stackoverflow.com/a/138995/151453

FOR %%i IN ("%infile%") DO IF EXIST %%~si\NUL (
	call :Echos [ERROR] Input path "%infile%" is a directory. I need a file.
	exit /b 4
)

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
) else (
	exit /b 0
)


REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  echo %_vspgINDENTS%[%batfilenam%] %*
exit /b 0

:EchoExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
exit /b 0
