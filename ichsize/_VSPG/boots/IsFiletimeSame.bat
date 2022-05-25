@echo off
setlocal EnableDelayedExpansion
set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.

: This is a function.

:IsFiletimeSame
REM Usage: 
REM call IsFiletimeSame.bat filepath1 filepath2
REM
REM 
REM 
REM exit 0 if filetime same, otherwise, non-zero exit-code.
REM
REM Implementation note: 
REM * forfiles can report file modification time as accurate as a second.
REM * forfiles can check only file/file-pattern in current directory, so we need to pushd first.

set filepath1=%~1
set filepath2=%~2

if "%filepath1%" == "" (
	call :Echos [Error] Missing parameter 1
	exit /b 4
)

if "%filepath2%" == "" (
	call :Echos [Error] Missing parameter 2
	exit /b 4
)

if not exist "%filepath1%" (
	REM [2022-03-15] Chj: If user, in PostBuild-CopyOutput4.bat, wants to copy wildcard source,
	REM (e.g. *.exe *.dll) some single source-file may not exist here, -- bcz, in a parellel 
	REM build action, an exe file matching *.exe may have been deleted by another build thread.
	REM 
	REM If we ignore this "source file not exist" fact and go on, the 'forfiles' below will 
	REM output an stderr line like:
	REM
	REM		ERROR: Files of type "XXX.exe" not found.
	REM 
	REM And, this "ERROR:" signature WILL cause MSBuild to fail, fail even if .bat exist code
	REM is zero.
	exit /b 4
)

if not exist "%filepath2%" (
	exit /b 4
)

call "%batdir%\PathSplit.bat" "%filepath1%" dir1 filenam1
pushd "%dir1%"
for /F "tokens=*" %%a in ('forfiles /m "%filenam1%" /c "cmd.exe /c echo @fdate @ftime"') do set filetime1=%%a
if errorlevel 1 exit /b 4
popd

call "%batdir%\PathSplit.bat" "%filepath2%" dir2 filenam2
pushd "%dir2%"
for /F "tokens=*" %%a in ('forfiles /m "%filenam2%" /c "cmd.exe /c echo @fdate @ftime"') do set filetime2=%%a
if errorlevel 1 exit /b 4
popd

if "%filetime1%" == "" (
	call :Echos [UNEXPECT] filetime1 is empty string. ^("%filepath1%"^)
	exit /b 4
)

if "%filetime2%" == "" (
	call :Echos [UNEXPECT] filetime2 is empty string. ^("%filepath2%"^)
	exit /b 4
)

if "%filetime1%" == "%filetime2%" (
	exit /b 0
) else (
	exit /b 4
)

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
