@echo off
setlocal EnableDelayedExpansion

set batfilenam=%~n0%~x0
set bootsdir=%~dp0
set bootsdir=%bootsdir:~0,-1%
call "%bootsdir%\GetParentDir.bat" userbatdir "%bootsdir%"
set _vspgINDENTS=%_vspgINDENTS%.

call "%bootsdir%\SearchAndExecSubbat.bat" Greedy0 VSPU-CopyOrClean.bat 0 %SubbatSearchDirsNarrowToWide%

if errorlevel 1 exit /b 4

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

:EchoAndExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
  call %*
exit /b %ERRORLEVEL%

:EchoVar
  setlocal & set Varname=%~1
  call echo %_vspgINDENTS%[%batfilenam%] %Varname% = %%%Varname%%%
exit /b 0

:SetErrorlevel
  REM Usage example:
  REM call :SetErrorlevel 4
exit /b %1

