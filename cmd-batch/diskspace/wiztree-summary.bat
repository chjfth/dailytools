@echo off
setlocal EnableDelayedExpansion

set batfilenam=%~nx0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set batdir2=%batdir:\=\\%

REM 
REM First parameter: the drive-letter to scan, C: D: etc.

set DvLetter=%~1
if not defined DvLetter (
	set DvLetter=C
)

REM Intercept the first letter, so that user can
set DvLetter=%DvLetter:~0,1%

set curTimestamp=%DATE:/=-% %TIME::=.%
set curTimestamp=%curTimestamp:~0,-3%

call :IsRunAsAdmin
if not !errorlevel!==0 (
    call :Echos This bat must be run as Administrator.
    exit /b 4
)

if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
	set exefile=WizTree64.exe
) else if "%PROCESSOR_ARCHITECTURE%"=="ARM64" (
	set exefile=WizTree64a.exe
) else (
	set exefile=WizTree.exe
)

call :FindExeFullPath fpWiztree %exefile%
if not defined fpWiztree (
	call :Echos [ERROR] %exefile% is not found in your PATH.
	call :Echos You may get WizTree version 4.13+ from https://diskanalyzer.com/guide
	exit /b 4
)

call :FindExeFullPath fpCat cat.exe
if not defined fpCat (
	call :Echos [ERROR] cat.exe is not found in your PATH.
	call :Echos You may get cat.exe from Git for Windows https://gitforwindows.org/ .
	exit /b 4
)

call :FindExeFullPath fpSed sed.exe
if not defined fpSed (
	call :Echos [ERROR] sed.exe is not found in your PATH.
	call :Echos You may get sed.exe from Git for Windows https://gitforwindows.org/ .
	exit /b 4
)

set csvFilename=%curTimestamp%-%DvLetter%.csv
set sumFilename=%curTimestamp%-%DvLetter%.summary.csv

call :Echos Calling wiztree.exe to create folder summary of drive-letter %DvLetter%:

call :EchoAndExecWait "%fpWiztree%" "%DvLetter%:" /export="%csvFilename%" /admin=1 /exportfolders=1 /exportfiles=0

if not !errorlevel!==0 (
	call :Echos ERROR: Fail executing %fpWiztree%
	exit /b 4
)

call :Echos Generating: %CD%\%sumFilename% 

@echo on
cat "%csvFilename%" | sed -rn 's/^(".+\\"),([0-9]{1,})[0-9]{8},.+$/\200+ MB,\1/p' > "%sumFilename%"

@echo off

if not !errorlevel!==0 (
	exit /b 4
)

call :Echos Done: %CD%\%sumFilename% 

call :Echos Success.
exit /b 0


REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  REM This function preserves %ERRORLEVEL% for the caller,
  REM and, LastError does NOT pollute the caller.
  setlocal & set LastError=%ERRORLEVEL%
  echo [%batfilenam%] %*
exit /b %LastError%

:EchoAndExec
  echo [%batfilenam%] EXEC: %*
  call %*
exit /b %ERRORLEVEL%

:EchoAndExecWait
  echo [%batfilenam%] EXEC-WAIT: %*
  start "" /wait %*
exit /b %ERRORLEVEL%

:IsRunAsAdmin
  REM Tips from https://www.robvanderwoude.com/clevertricks.php
  REM Exitcode=0 means yes.
  %windir%\System32\whoami.exe /groups | %windir%\System32\findstr.exe "S-1-16-12288" > NUL
exit /b %ERRORLEVEL%

:FindExeFullPath
  REM Example
  REM
  REM   call :FindExeFullPath outvarFullPath "notepad.exe"
  REM 
  REM Return:
  REM 
  REM   outvarFullPath=c:\program files\d2
  setlocal
  if "%~1"=="" exit /b 4
  if "%~2"=="" exit /b 4
  for %%g in ("%~2") do set fullpath=%%~$PATH:g
  endlocal & ( set "%~1=%fullpath%" )
exit /b 0


:ExitIfExeNotFound
  setlocal
  set exename=%~1
  
  call :FindExeFullPath exefullpath "%exename%
  if "%exefullpath%" == "" (
    call :Echos ERROR: Cannot find %exename% in your PATH env.
    exit /b 4
  )
exit /b 0

