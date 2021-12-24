@echo off
setlocal EnableDelayedExpansion
REM Usage: This .bat is to be called from Visual Studio project Pre-build-commands and/or Post-build-commands,
REM so that we can write complex batch programs from dedicated .bat files, instead of tucking them in 
REM those crowded .vcxproj or .csproj .
REM This .bat is now internally to VSPG. User does not need to care for its code detail.


REM set batfilenam to .bat filename(no directory prefix)
set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
REM  
set SubworkBat=%batdir%\%1
shift
set FeedbackFile=%1
shift
call :EchoVar SubworkBat
call :EchoVar FeedbackFile

if not exist %FeedbackFile% (
	call :Echos [VSPG-ERROR] Not-existing feedback file: %FeedbackFile%
	exit /b 4
)

set ALL_PARAMS="%~1" "%~2" "%~3" "%~4" "%~5" "%~6" "%~7" "%~8"
if exist %SubworkBat% (
  cmd /c %SubworkBat% %ALL_PARAMS%
) else (
  call :Echos [VSPG-ERROR] SubworkBat NOT found: %SubworkBat%
  call :SetErrorlevel 4
)
if errorlevel 1 ( call :Touch %FeedbackFile% && exit /b 4 )

exit /b 0


REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  echo [%batfilenam%] %*
exit /b

:EchoExec
  echo [%batfilenam%] EXEC: %*
exit /b

:EchoVar
  REM Env-var double expansion trick from: https://stackoverflow.com/a/1200871/151453
  set _Varname=%1
  for /F %%i in ('echo %_Varname%') do echo [%batfilenam%] %_Varname% = !%%i!
exit /b

:SetErrorlevel
  REM Usage example:
  REM call :SetErrorlevel 4
exit /b %1

:Touch
	REM Touch updates a file's modification time to current.
	REM NOTE: No way to check for success/fail here. So, call it only when 
	REM you have decided to fail the whole bat.
	
	copy /b "%~1"+,, "%~1" >NUL 2>&1
exit /b

goto :END


:END
rem echo [%batfilenam%] END for %ProjectDir%
