@echo off
setlocal EnableDelayedExpansion
REM Called as this:
REM <this>.bat $(SolutionDir) $(ProjectDir) $(BuildConf) $(PlatformName) $(TargetDir) $(TargetFileNam) $(TargetName) $(IntrmDir)
REM ==== boilerplate code >>>
REM
set batfilenam=%~n0%~x0
set bootsdir=%~dp0
set bootsdir=%bootsdir:~0,-1%
call "%bootsdir%\GetParentDir.bat" userbatdir "%bootsdir%"
set _vspgINDENTS=%_vspgINDENTS%.

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

REM Try to call some PostBuild bat-s  from one of five predefined directories,
REM whichever is encountered first. But if none found, just do nothing.
REM We do non-greedy calling of these bat-s, from narrow(project level) to
REM wide(global level), because user (probably) wants to override 
REM outer env's sub-work with his own one.
REM But if user wants outer bat-s as well, he should call the outer bat-s explicitly.


REM ==== Call Team-Postbuild8.bat if exist. ====
call "%bootsdir%\SearchAndExecSubbat.bat" Greedy0 Team-PostBuild8.bat %VSPG_VSIDE_ParamsPack% %SubbatSearchDirsNarrowToWide%
if errorlevel 1 exit /b 4

REM ==== Call Personal-Postbuild8.bat if exist. ====
call "%bootsdir%\SearchAndExecSubbat.bat" Greedy0 Personal-PostBuild8.bat %VSPG_VSIDE_ParamsPack% %SubbatSearchDirsNarrowToWide%
if errorlevel 1 exit /b 4

REM ==== Call PostBuild-CopyOutput4.bat if exist. ====
REM If you need this bat, just copy it from ..\samples\PostBuild-CopyOutput4.bat.sample,
REM and tune some variables there to meet your need..

call "%bootsdir%\SearchAndExecSubbat.bat" Greedy0 PostBuild-CopyOutput4.bat^
  """%BuildConf%"" ""%PlatformName%"" ""%TargetDir%"" ""%TargetName%"""^
  %SubbatSearchDirsNarrowToWide%
if errorlevel 1 exit /b 4


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

:EchoAndExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
  call %*
exit /b %ERRORLEVEL%

:EchoVar
  setlocal & set Varname=%~1
  call echo %_vspgINDENTS%[%batfilenam%] %Varname% = %%%Varname%%%
exit /b 0

:SetErrorlevel
  REM Usage example:
  REM call :SetErrorlevel 4
exit /b %1

