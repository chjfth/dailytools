@echo off
setlocal EnableDelayedExpansion

: Function: IsFolder
: Checks that input parameter is an existing folder.
: Returns 0 means true.
: true : C:\Users\win7evn\AppData\Local\Temp
: true : C:\Users\win7evn\AppData\Local\Temp\
: true : C:\Documents and Settings\win7evn\AppData\Local\Temp
: true : C:\Documents and Settings\win7evn\AppData\Local\Temp\
: false: C:\Users\win7evn\AppData\Local\IconCache.db
: false: C:\Documents and Settings\win7evn\AppData\Local\IconCache.db

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.

set inpath=%~1

if "%inpath%" == "" (
	call :Echos [ERROR] Missing input path as parameter.
	exit /b 4
)

set finalchar=%inpath:~-1%
if "%finalchar%" == "\" (
	REM Strip final backslash
	set finalchar=%finalchar:~0,-1%
)

REM Check whether it is a folder.
REM Thanks to: https://stackoverflow.com/a/1466528/151453
if exist "%inpath%\*" (
	exit /b 0
) else (
	exit /b 4
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
