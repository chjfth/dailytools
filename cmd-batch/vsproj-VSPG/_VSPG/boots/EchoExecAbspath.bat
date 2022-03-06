@echo off
setlocal EnableDelayedExpansion

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.

:EchoExecAbspath
REM Echo the sub-command to execute then actually execute the sub-command
REM Sub-command is the whole %* content.
REM For %1, I will find out the abspath and substitute %1 to its abspath(fullpath),
REM so that user can clearly know which EXE is executed.
REM
REM USER NOTE:
REM (1) Explicit filename extension is required, e.g. write "foobar.exe", not "foobar".
REM (2) Don't add any dir-prefix to %1 .

set input_cmd_all=%*
set _exename_=%1
set exename=%~1
REM -- exe can be XXX.bat though.
set exepath=
set hasbkslash=

call "%batdir%\IsSubStr.bat" hasbkslash "%exename%" \
if "%hasbkslash%" == "1" (
	call :Echos [ERROR] Program name "%exename%" should not have dir-prefix.
	exit /b 4
)
if not "%hasbkslash%" == "0" (
	call :Echos [INTERNAL-ERROR] Sub-bat NOT found: "%batdir%\IsSubStr.bat"
	exit /b 4
)

call "%batdir%\IsSubStr.bat" hasdot "%exename%" .
if "%hasdot%" == "0" (
	call :Echos [ERROR] Program name "%exename%" must have extension suffix, for example, write Foobar.exe, not just Foobar.
	exit /b 4
)

for %%x in ("%exename%") do set exepath=%%~$PATH:x
if not defined exepath (
	call :Echos [ERROR] "%exename%" is NOT found in your PATH env-var.
	exit /b 4
)

call "%batdir%\strlen.bat" param1len _exename_
rem call :Echos _exename_slen=%param1len%

call set params_all=%%input_cmd_all:~%param1len%%%
set abspath_cmd_all="%exepath%" %params_all%

call :EchoExec %abspath_cmd_all%
call %abspath_cmd_all%

exit /b %ERRORLEVEL%



REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  echo %_vspgINDENTS%[%batfilenam%] %*
exit /b

:EchoExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
exit /b
