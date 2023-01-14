@echo off
setlocal EnableDelayedExpansion
REM Caveat: Do not echo exclamation mark(!), bcz we have EnableDelayedExpansion.

set batnen=%~n0
set batfilenam=%~nx0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set batdir2=%batdir:\=\\%

REM ======== parameter/env checking ========

call :IsExeInPath ffmpeg.exe
if not !errorlevel!==0 (
	call :Echos ffmpeg.exe is NOT found in PATH, I cannot work.
	exit /b 4
)

set inputmp4=%~1
set inputnen=%~dpn1
set inputext=%~x1
set inputext=%inputext:~1,999%
set audiokbps=%~2

if not defined audiokbps (
	call :Echos Input a video file, reduce its audio bitrate.
	call :Echos Usage:
	call :Echosi4 %batfilenam% {input-video} {audio-kbps}
	call :Echos Example:
	call :Echosi4 %batfilenam% input.mp4 64
	exit /b 4
)

REM ======== Now start your code ========

set fp_videoonly=%inputnen%.--videoonly--.%inputext%
set fp_audioonly=%inputnen%.--audioonly--.%inputext%
set fp_finaloutput=%inputnen%.%audiokbps%kbps.%inputext%

call :EchoAndExec ffmpeg -i "%inputmp4%" -an -c copy -y "%fp_videoonly%"
if not !errorlevel!==0 exit /b 4

call :EchoAndExec ffmpeg -i "%inputmp4%" -vn -ac 1 -ab %audiokbps%k -f mp3 -y "%fp_audioonly%"
if not !errorlevel!==0 exit /b 4

call :EchoAndExec ffmpeg -i "%fp_videoonly%" -i "%fp_audioonly%" -c copy -y "%fp_finaloutput%"
if not !errorlevel!==0 exit /b 4

call :Echos Convert success:
call :Echosi4 %fp_finaloutput%

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
