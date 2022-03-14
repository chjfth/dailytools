@echo off

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.


: DelaySeconds

  call :Echos Delay %~1 seconds...
  ping 127.0.0.1 -n %~1 -w 1000 > nul
  ping 127.0.0.1 -n 2 -w 1000 > nul

exit /b 0



REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  echo %_vspgINDENTS%[%batfilenam%] %*
exit /b 0

:EchoAndExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
  %*
exit /b %ERRORLEVEL%

