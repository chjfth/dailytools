@echo off
REM ==== boilerplate code >>>
REM
set batfilenam=%~n0%~x0
REM VSPG-Start9.bat $(ProjectDir)_VSPG\VSPG-PostBuild7.bat $(ProjectDir)Program.cs $(SolutionDir) $(ProjectDir) $(Configuration) $(PlatformName) $(TargetDir) $(TargetFileName) $(TargetName)
set SubworkBat=%1
shift
set TouchFile=%1
shift
call :EchoVar SubworkBat
call :EchoVar TouchFile
REM ==== boilerplate code <<<<

set ALL_PARAMS=%1 %2 %3 %4 %5 %6 %7
if exist %SubworkBat% (
  cmd /c %SubworkBat% %ALL_PARAMS%
) else (
  call :Echos [ERROR] SubworkBat NOT found: %SubworkBat%
  call :SetErrorlevel 4
)
if errorlevel 1 ( call Touch %TouchFile% && exit /b 4 )

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
  REM Env-var double expansion trick from: https://stackoverflow.com/a/1202562/151453
  set _Varname=%1
  for /F %%i in ('echo %%%_Varname%%%') do echo [%batfilenam%] %_Varname% = %%i
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
