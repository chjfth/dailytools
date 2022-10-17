@echo off
setlocal EnableDelayedExpansion

set batfilenam=%~n0%~x0
set bootsdir=%~dp0
set bootsdir=%bootsdir:~0,-1%
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


REM ==== Call Team-Prebuild.bat if exist. ====
call "%bootsdir%\SearchAndExecSubbat.bat" Greedy0 Team-PreBuild.bat "" %SubbatSearchDirsNarrowToWide%
if errorlevel 1 exit /b 4

REM ==== Call Personal-Prebuild.bat if exist. ====
call "%bootsdir%\SearchAndExecSubbat.bat" Greedy0 Personal-PreBuild.bat "" %SubbatSearchDirsNarrowToWide%
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

