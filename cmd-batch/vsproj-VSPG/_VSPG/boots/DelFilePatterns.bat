@echo off
setlocal EnableDelayedExpansion

: This is a function :

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.
call :Echos START from %batdir%

REM Param1: One source folder. This folder is reference when a pattern contains wildcard(* or ?).
REM Param2: One destination folder. Files in there will be deleted.
REM Remaining params: Each is a filename or a pattern.

set VSPG_COPYFILE_DO_DELETE=1
call "%bootsdir%\CopyFilePatterns" %*

exit /b %ERRORLEVEL%


REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  REM This function preserves %ERRORLEVEL% for the caller,
  REM and, LastError does NOT pollute the caller.
  setlocal & set LastError=%ERRORLEVEL%
  echo %_vspgINDENTS%[%batfilenam%] %*
exit /b %LastError%

:EchoAndExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
  call %*
exit /b %ERRORLEVEL%

:EchoVar
  setlocal & set Varname=%~1
  call echo %_vspgINDENTS%[%batfilenam%]%~2 %Varname% = %%%Varname%%%
exit /b 0
