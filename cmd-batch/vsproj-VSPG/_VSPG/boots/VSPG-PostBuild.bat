@echo off
setlocal EnableDelayedExpansion

set batfilenam=%~n0%~x0
set bootsdir=%~dp0
set bootsdir=%bootsdir:~0,-1%
call "%bootsdir%\GetParentDir.bat" userbatdir "%bootsdir%"
set _vspgINDENTS=%_vspgINDENTS%.

if defined vspg_DO_SHOW_VERBOSE (
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
)

REM Try to call some PostBuild bat-s  from one of five predefined directories,
REM whichever is encountered first. But if none found, just do nothing.
REM We do non-greedy calling of these bat-s, from narrow(project level) to
REM wide(global level), because user (probably) wants to override 
REM outer env's sub-work with his own one.
REM But if user wants outer bat-s as well, he should call the outer bat-s explicitly.


REM ==== Call Team-Postbuild.bat if exist. ====
call "%bootsdir%\SearchAndExecSubbat.bat" Greedy0 Team-PostBuild.bat "" %SubbatSearchDirsNarrowToWide%
if errorlevel 1 exit /b 4

REM ==== Call Personal-Postbuild.bat if exist. ====
call "%bootsdir%\SearchAndExecSubbat.bat" Greedy0 Personal-PostBuild.bat "" %SubbatSearchDirsNarrowToWide%
if errorlevel 1 exit /b 4

REM ==== Call VSPU-CopyOrClean.bat if exist. ====
REM If you need this bat, just copy it from ..\samples\VSPU-CopyOrClean.bat.sample,
REM and tune some variables there to meet your need..

call "%bootsdir%\SearchAndExecSubbat.bat" Greedy0 VSPU-CopyOrClean.bat 1 %SubbatSearchDirsNarrowToWide%
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

