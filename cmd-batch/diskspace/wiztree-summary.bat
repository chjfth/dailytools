@echo off
setlocal EnableDelayedExpansion

set batfilenam=%~nx0
set batdir=%~dp0
set batdir=%batdir:~0,-1%

REM Add my usual exe paths for cat.exe, sed.exe, WizTree.exe
PATH=%PATH%;C:\Program Files\Git\usr\bin;C:\Git\usr\bin
PATH=%PATH%;C:\Program Files\WizTree;D:\software_vmwin\wintools\WizTree


REM If current dir happens to be C:\WINDOWS\system32, switch to %batdir% .
REM This avoids generating summary file into C:\WINDOWS\system32.
if /i "%CD%" == "%windir%\system32" (
	call :Echos Note: Current directory is %CD%, which is not desired, now change it to your bat files directory: "%batdir%"
	pushd "%batdir%"
)

REM 
REM First parameter: the drive-letter to scan, C: D: etc.

set DvLetter=%~1
if not defined DvLetter (
	set DvLetter=C
)

REM Intercept the first letter, so that user can assign C or C:
set DvLetter=%DvLetter:~0,1%

call :NowTimeAsFilename curTimestamp

call :IsRunAsAdmin
if not !errorlevel!==0 (
    call :Echos This bat must be run as Administrator.
    exit /b 4
)

if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
	set exeWizTree=WizTree64.exe
) else if "%PROCESSOR_ARCHITECTURE%"=="ARM64" (
	set exeWizTree=WizTree64a.exe
) else (
	set exeWizTree=WizTree.exe
)

call :FindExeFullPath fpWiztree %exeWizTree%
if not defined fpWiztree (
	call :Echos [ERROR] %exeWizTree% is not found in your PATH.
	call :Echos You may get WizTree version 4.13+ from https://diskanalyzer.com/guide
	exit /b 4
)
call :Echos Found %exeWizTree% at: "%fpWiztree%"

call :FindExeFullPath fpCat cat.exe
if not defined fpCat (
	call :Echos [ERROR] cat.exe is not found in your PATH.
	call :Echos You may get cat.exe from Git for Windows https://gitforwindows.org/ .
	exit /b 4
)
call :Echos Found cat.exe at: "%fpCat%"

call :FindExeFullPath fpSed sed.exe
if not defined fpSed (
	call :Echos [ERROR] sed.exe is not found in your PATH.
	call :Echos You may get sed.exe from Git for Windows https://gitforwindows.org/ .
	exit /b 4
)
call :Echos Found sed.exe at: "%fpSed%"

set csvFilename=%curTimestamp%-%DvLetter%.csv
set sumFilename=%curTimestamp%-%DvLetter%.summary.csv

call :Echos Calling wiztree.exe to create folder summary of drive-letter %DvLetter%:

call :EchoAndExecWait "%fpWiztree%" "%DvLetter%:" /export="%csvFilename%" /admin=1 /exportfolders=1 /exportfiles=0

if not !errorlevel!==0 (
	call :Echos ERROR: Fail executing %fpWiztree%
	exit /b 4
)

REM WizTree 4.13 may still return 0 on ERROR(buggy), e.g., passing a non-existing drive-letter to it.
REM So we need to further check whether the csv is generated.
if not exist "%csvFilename%" (
	call :Echos ERROR: Desired csv file "%csvFilename%" is not generated, so WizTree exe actually fails.
	exit /b 4
)

call :Echos Generating: %CD%\%sumFilename% 

@echo on
cat "%csvFilename%" | sed -rn 's/(".+"),([0-9]{1,})[0-9]{8},.+$/\200+ MB,\1/p' > "%sumFilename%"

@echo off

if not !errorlevel!==0 (
	exit /b 4
)

REM Keep a copy of WizTree's original csv file.

set csvWizTreeLastTime=WizTree.lasttime-%DvLetter%.csv

if exist "%csvWizTreeLastTime%" (
	del "%csvWizTreeLastTime%"
	if not !errorlevel!==0 (
		call :Echos ERROR: Delete last-time "%csvWizTreeLastTime%"
		exit /b 4
	)
)

call :EchoAndExec ren "%csvFilename%" "%csvWizTreeLastTime%"
if not !errorlevel!==0 (
	call :Echos ERROR: File renaming fail.
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
  REM Git-for-Windows have same-same EXEs, so I use Abs-path for whoami.exe etc.
  %windir%\System32\whoami.exe /groups | %windir%\System32\findstr.exe "S-1-16-12288" > NUL
exit /b %ERRORLEVEL%

:FindExeFullPath
  REM Example:
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

:NowTimeAsFilename
  REM Example:
  REM 
  REM   call :NowTimeAsFilename varTimestr
  REM 
  REM On return, caller's varTimestr var will contain a string representing
  REM current time, like: "04-04-2023 Tue 20.53.32"
  REM Note: The output string deliberately has space in it, so caller 
  REM must be able to deal with space-char in filename. This is a situation
  REM that the user MUST cope with, bcz, the %DATE% expanded string 
  REM may have already contained space-chars, under some user-locale selection
  REM (in intl.cpl).

  setlocal
  REM We create the string from env-var DATE and TIME, and at the sametime,
  REM replacing invalid file-system chars to valid ones.
  set timestr=%DATE:/=-% %TIME::=.%
  REM 
  REM Remove "percent-second" tail from %TIME%
  set timestr=%timestr:~0,-3%
  endlocal & ( set "%~1=%timestr%" )
exit /b 0
