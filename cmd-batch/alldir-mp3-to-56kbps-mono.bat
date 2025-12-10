@echo off
REM Program parameter assigns target KBps (default to 56)
REM -ac 1 : audio channel is one (mono)

set KBps=%~1

if not defined Kbps (
	set Kbps=56
)

mkdir outdir
for %%f in (*.mp3) do (
    call :EchoAndExec ffmpeg -i "%%~f" -ab "%KBps%" -ac 1 -f mp3 "outdir\%%~nf.mp3"
)

exit /b 0

:EchoAndExec
  echo EXEC: %*
  call %*
exit /b %ERRORLEVEL%
