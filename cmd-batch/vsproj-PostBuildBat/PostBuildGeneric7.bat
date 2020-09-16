@echo off
REM ==== boilerplate code >>>
REM
set batfilenam=%~n0%~x0
REM PostBuildGeneric7.bat $(SolutionDir) $(ProjectDir) $(Configuration) $(PlatformName) $(TargetDir) $(TargetFileName) $(TargetName)
set SolutionDir=%1
set SolutionDir=%SolutionDir:~0,-1%
set ProjectDir=%2
set ProjectDir=%ProjectDir:~0,-1%
REM BuildConf : Debug | Release
set BuildConf=%3
REM PlatformName : Win32 | x64
set PlatformName=%4
REM TargetDir is the EXE/DLL output directory
set TargetDir=%5
set TargetDir=%TargetDir:~0,-1%
REM TargetFilenam is the EXE/DLL output name (varname chopping trailing 'e', means "no path prefix")
set TargetFilenam=%6
set TargetName=%7
REM
rem call :Echos START for %ProjectDir%
REM
REM ==== boilerplate code <<<<


call :Echos called with params: 
call :Echos   SolutionDir = %SolutionDir%
call :EchoVar ProjectDir
call :EchoVar BuildConf
call :EchoVar PlatformName
call :EchoVar TargetDir
call :EchoVar TargetFilenam
call :EchoVar TargetName

REM Call PostBuildSyncOutput4.bat only if that file exist. If you need it, just copy it from the .template aside.
REM We check two places for that .bat, first in %ProjectDir% then in %SolutionDir% .
if exist %ProjectDir%\PostBuildSyncOutput4.bat (
	call :Echos Now exec [ProjectDir]\PostBuildSyncOutput4.bat 
	call %ProjectDir%\PostBuildSyncOutput4.bat %BuildConf% %PlatformName% %TargetDir% %TargetName%
	if errorlevel 1 exit /b 4
	goto :BAT4_DONE
)
REM.
if exist %SolutionDir%\PostBuildSyncOutput4.bat (
	call :Echos Now exec [SolutionDir]\PostBuildSyncOutput4.bat 
	call %SolutionDir%\PostBuildSyncOutput4.bat %BuildConf% %PlatformName% %TargetDir% %TargetName%
	if errorlevel 1 exit /b 4
)

:BAT4_DONE

goto :END

REM =============================
REM ==== Main Procedure Done ====
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

:END
rem echo [%batfilenam%] END for %ProjectDir%
