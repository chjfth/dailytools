@echo off

for %%f in (*.ogg) do (
    call :EchoAndExec ffmpeg -i "%%~f" -ab 128k -f mp3 "%%~nf.mp3"
)

exit /b 0

:EchoAndExec
  echo EXEC: %*
  call %*
exit /b %ERRORLEVEL%
