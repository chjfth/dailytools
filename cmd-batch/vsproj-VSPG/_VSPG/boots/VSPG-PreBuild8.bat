@echo off
setlocal EnableDelayedExpansion
REM Called as this:
REM <this>.bat $(SolutionDir) $(ProjectDir) $(BuildConf) $(PlatformName) $(TargetDir) $(TargetFileNam) $(TargetName) $(IntrmDir)
REM ==== boilerplate code >>>
REM
set batfilenam=%~n0%~x0
set bootsdir=%~dp0
set bootsdir=%bootsdir:~0,-1%
call "%bootsdir%\PathSplit.bat" "%bootsdir%" userbatdir __temp
set SolutionDir=%~1
set ProjectDir=%~2
REM BuildConf : Debug | Release
set BuildConf=%~3
set _BuildConf_=%3
REM PlatformName : Win32 | x64
set PlatformName=%~4
REM TargetDir is the EXE/DLL output directory
set TargetDir=%~5
set _TargetDir_=%5
REM TargetFilenam is the EXE/DLL output name (varname chopping trailing 'e', means "no path prefix")
set TargetFilenam=%~6
set TargetName=%~7
set IntrmDir=%~8
REM ==== boilerplate code <<<<


call :Echos called with params: 
call :EchoVar bootsdir
call :EchoVar SolutionDir
call :EchoVar ProjectDir
call :EchoVar BuildConf
call :EchoVar PlatformName
call :EchoVar IntrmDir
call :EchoVar TargetDir
call :EchoVar TargetFilenam
call :EchoVar TargetName

REM Try to call PreBuild-SubWCRev1.bat etc from one of five predefined directories,
REM whichever is encountered first. But if none found, just do nothing.
REM If you need this PreBuild-SubWCRev1.bat to run, just copy and tune it from
REM PreBuild-SubWCRev1.bat.sample .

set SubbatSearchDirs=^
  "%ProjectDir%"^
  "%ProjectDir%\_VSPG"^
  "%SolutionDir%"^
  "%SolutionDir%\_VSPG"^
  "%userbatdir%"

call "%bootsdir%\SearchAndExecSubbat.bat" PreBuild-SubWCRev1.bat %SubbatSearchDirs%
if errorlevel 1 exit /b 4

REM ==== Call Team-Prebuild8.bat if exist. ====
call "%bootsdir%\SearchAndExecSubbat.bat" Team-PreBuild8.bat %VSPG_VSIDE_ParamsPack% %SubbatSearchDirs%
if errorlevel 1 exit /b 4

REM ==== Call Personal-Prebuild8.bat if exist. ====
call "%bootsdir%\SearchAndExecSubbat.bat" Personal-PreBuild8.bat %VSPG_VSIDE_ParamsPack% %SubbatSearchDirs%
if errorlevel 1 exit /b 4


goto :END

REM =============================
REM ====== Functions Below ======
REM =============================

REM %~n0%~x0 is batfilenam
:Echos
  echo [%~n0%~x0] %*
exit /b

:EchoExec
  echo [%~n0%~x0] EXEC: %*
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


:END
