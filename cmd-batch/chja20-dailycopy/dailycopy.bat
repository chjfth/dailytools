@echo off
setlocal EnableDelayedExpansion
REM Caveat: Do not echo exclamation mark(!), bcz we have EnableDelayedExpansion.

set batstem=%~n0
set batfilenam=%~nx0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set batdir2=%batdir:\=\\%
set _vspgINDENTS=%_vspgINDENTS%.
call :Echos START from %batdir%

call :Echos Now time %DATE% %TIME%
echo.


set timeoutsec=3600

REM User needs to answer 'y' to go on daily-copy/daily-backup.
REM If timeout, it means user is not at the computer, and the backup will not be carried out.
CHOICE /T %timeoutsec% /C yn /CS /D n /M "(Timeout %timeoutsec% seconds) Run dailycopy now? "

if !errorlevel!==2 (
	call :Echos User answers no. Task quit.
	exit /b 0
)

if not !errorlevel!==1 (
	call :Echos No user keyboard input. Task quit.
	exit /b 0
)

set dirOutRoot=I:\auto-backups

call :EchoAndExec py.exe "D:\gitw\dailytools\pytools\dailycopy.py" -v -d "%dirOutRoot%" "%batdir%\dailycopy.ini" %*
REM -- dailycopy.ini tells preserve_days=60, so backups older than 60 days will be deleted by .py automatically.

if not !errorlevel!==0 (
	call :Echos [ERROR] myexe execution fail.
    exit /b 4
)


REM
REM Now zipping chjfth.exb .
REM

call :Echos 
call :Echos Now zipping chjfth.exb ...

rem First check that %DATE% is in YYYY-MM-DD format.

if not "%DATE:~4,1%.%DATE:~7,1%" == "-.-" (
	call :Echos [ERROR] You have to set Window's CRT locale to have env-var DATE in format YYYY-MM-DD .
	exit /b 4
)

set yyyymmdd=%DATE:-=%
set zipfilename=chjfth-evernote-exb-%yyyymmdd%.7z

set zipoutdir=%dirOutRoot%\chja20-daily\%yyyymmdd%.dailycopy
set exbinput=%zipoutdir%\chj-Evernote\Databases\chjfth.exb

call :EchoAndExec "C:\program Files\7-Zip\7z.exe" a -mmt8  "%zipoutdir%\%zipfilename%"  "%exbinput%"
if not !errorlevel!==0 (
	call :Echos [ERROR] 7-zip command line execution fail.
    exit /b 4
)

call :Echos 
call :Echos Removing the big chjfth.exb ...

call :EchoAndExec rd /s /q "%zipoutdir%\chj-Evernote"
if not !errorlevel!==0 (
	call :Echos [ERROR] RD, removing dir, execution fail.
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
  echo %_vspgINDENTS%[%batfilenam%] %*
exit /b %LastError%

:Echos
  REM This function preserves %ERRORLEVEL% for the caller,
  REM and, LastError does NOT pollute the caller.
  setlocal & set LastError=%ERRORLEVEL%
  echo %_vspgINDENTS%[%batfilenam%]     %*
exit /b %LastError%

:EchoAndExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
  call %*
exit /b %ERRORLEVEL%

:EchoVar
  setlocal & set Varname=%~1
  call echo %_vspgINDENTS%[%batfilenam%]%~2 %Varname% = %%%Varname%%%
exit /b 0

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


:GetAbsPath
  REM Get absolute-path from a relative-path(relative to %CD%). 
  REM If already absolute, return as is.
  REM Note: If your input dir is relative to current .bat, use GetAbsPathRelaToMe instead.
  REM
  REM Param1: Var name to receive output.
  REM Param2: The input path.
  REM
  REM Feature 1: this function removes redundant . and .. from input path.
  REM I
  REM For example:
  REM     call :GetAbsPath outvar C:\abc\.\def\..\123
  REM Returns outvar:
  REM     C:\abc\123
  REM
  REM Feature 2: It's pure string manipulation, so the input path doesn't have to 
  REM actually exist on the file-system.
  REM
  if "%~1"=="" exit /b 4
  if "%~2"=="" exit /b 4
  for %%a in ("%~2") do set "%~1=%%~fa"
exit /b 0


:GetAbsPathRelaToMe
  REM Get absolute-path from a relative-path(relative to current .bat's dir). 
  REM If input is already an absolute path, return as is.
  REM Param1: Var name to receive output.
  REM Param2: The input path, can be filepath or dirpath, can contain ".." .
  REM
  REM Note: Since we use `pushd`, the input path should actually exist on the 
  REM file-system to make it work correctly.
  REM
  setlocal & pushd "%~dp0"
  if "%~1"=="" exit /b 4
  if "%~2"=="" exit /b 4
  for %%a in ("%~2") do set "parent_dir=%%~fa"
  endlocal & ( set "%~1=%parent_dir%" )
exit /b 0
