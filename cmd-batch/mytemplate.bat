@echo off
setlocal EnableDelayedExpansion
REM Caveat: Do not echo exclamation mark(!), bcz we have EnableDelayedExpansion.

set batstem=%~n0
set batfilenam=%~nx0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set batdir2=%batdir:\=\\%

REM ======== parameter checking ========

set param1=%~1
set param1=%~2

if not defined param1 (
	call :Echos Missing parameter.
	exit /b 4
)

REM ======== Now start your work ========

call :EchoAndExec yourexe param1 param2

if not !errorlevel!==0 (
	call :Echos [ERROR] yourexe execution fail.
    exit /b 4
)

exit /b 0



REM =============================
REM ====== Functions Below ======
REM =============================

rem More from D:\gitw\dailytools\_VSPG\samples\vspg_functions.bat.sample

:Echos
  REM This function preserves %ERRORLEVEL% for the caller,
  REM and, LastError does NOT pollute the caller.
  setlocal & set LastError=%ERRORLEVEL%
  echo [%batfilenam%] %*
exit /b %LastError%

:Echosi4
  REM Like Echos, but with 4 spaces of indent.
  setlocal & set LastError=%ERRORLEVEL%
  echo [%batfilenam%]     %*
exit /b %LastError%

:EchoAndExec
  echo [%batfilenam%] EXEC: %*
  call %*
exit /b %ERRORLEVEL%

:SetErrorlevel
  REM Usage example:
  REM call :SetErrorlevel 4
exit /b %1

:IsExeInPath
  REM Param1: The exe filename, must contain .exe suffix .
  REM Return: ERRORLEVEL=0 if true.
  setlocal
  for %%p in ("%~1") do set exepath=%%~$PATH:p
  if defined exepath (exit /b 0) else (exit /b 4)
exit /b 0

:FilepathSplit
 REM Usage: 
 REM call :FilepathSplit "c:\program files\d2\d3.txt" dname fname
 REM Output dname=c:\program files\d2
 REM Output fname=d3.txt
  
  setlocal
  For %%A in ("%~1") do (
    set Folder=%%~dpA
    set Name=%%~nxA
  )
  endlocal & (
    set "%~2=%Folder:~0,-1%"
    set "%~3=%Name%"
  )
exit /b %ERRORLEVEL%

:FilenamSplit
  REM Usage: 
  REM call :FilenamSplit "file1.txt" stemname extname
  REM Output stemname=file1
  REM Output extname=txt
  setlocal
  for %%f in ("%~1") do (
    set stemname=%%~nf
    set _extname=%%~xf
  )
  ECHO _extname=%_extname%
  endlocal & (
    set "%~2=%stemname%"
    set "%~3=%_extname:~1,999%"
  )
exit /b 0
