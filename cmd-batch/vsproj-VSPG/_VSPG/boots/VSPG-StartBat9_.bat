@echo off
setlocal EnableDelayedExpansion
REM Usage: This .bat is to be called from Visual Studio project Pre-build-commands and/or Post-build-commands,
REM so that we can write complex batch programs from dedicated .bat files, instead of tucking them in 
REM those crowded .vcxproj or .csproj .
REM This .bat is now internally to VSPG. User does not need to care for its code detail.


REM set batfilenam to .bat filename(no directory prefix)
set batfilenam=%~n0%~x0
set bootsdir=%~dp0
set bootsdir=%bootsdir:~0,-1%
REM Use PathSplit to get parent directory of bootsdir.
call "%bootsdir%\GetParentDir.bat" userbatdir "%bootsdir%"
set _vspgINDENTS=%_vspgINDENTS%.
REM
set SubworkBatfile=%~1
set SubworkBatpath=%bootsdir%\%SubworkBatfile%
shift
set FeedbackFile=%~1
shift
set SolutionDir=%~1
set SolutionDir=%SolutionDir:~0,-1%
set ProjectDir=%~2
set ProjectDir=%ProjectDir:~0,-1%
REM BuildConf : Debug or Release
set BuildConf=%~3
REM PlatformName : Win32 or x64
set PlatformName=%~4
REM TargetDir is the EXE/DLL output directory
set TargetDir=%~5
set TargetDir=%TargetDir:~0,-1%
REM TargetFilenam is the EXE/DLL output name (varname chopping trailing 'e', means "no path prefix")
set TargetFilenam=%~6
REM And I set mundane TargetFilename as well:
set TargetFilename=%TargetFilenam%
set TargetName=%~7
set IntrmDir=%~8
set IntrmDir=%IntrmDir:~0,-1%

call "%bootsdir%\VSPG-version.bat" vspgver
call :Echos [VSPG version %vspgver%] started as: "%bootsdir%\%batfilenam%"


set VSPG_VSIDE_ParamsDiscrete="%SolutionDir%" "%ProjectDir%" "%BuildConf%" "%PlatformName%" "%TargetDir%" "%TargetFilenam%" "%TargetName%" "%IntrmDir%"
call "%bootsdir%\DQescape_NoTBS.bat" %VSPG_VSIDE_ParamsDiscrete%
set VSPG_VSIDE_ParamsPack=%DQescape_NoTBS_Output%
REM -- Note: when expanding VSPG_VSIDE_ParamsPack, do NOT surround extra double-quotes on %VSPG_VSIDE_ParamsPack% .


call :EchoVar SubworkBatpath
call :EchoVar FeedbackFile

if not "%FeedbackFile%"=="" (
  if not exist "%FeedbackFile%" (
	call :Echos [VSPG-ERROR] Not-existing feedback file: "%FeedbackFile%"
	exit /b 4
  )
)

if not exist "%SubworkBatpath%" (
  call :Echos [INTERNAL-ERROR] SubworkBatpath NOT found: "%SubworkBatpath%"
  call :SetErrorlevel 4
  exit /b 4
)

REM ======== Loading User Env-vars ======== 

REM This is a greedy search, bcz user may want to accumulate env-vars from outer env.
REM But if user does not like some env-var from outer env, he can override it(or clear it) 
REM from inner env explicitly.
REM In one word, the search order is from wide to narrow.

call "%bootsdir%\SearchAndExecSubbat.bat" Greedy1 VSPG-StartEnv.bat %VSPG_VSIDE_ParamsPack%^
  "%userbatdir%"^
  "%SolutionDir%\.."^
  "%SolutionDir%\_VSPG"^
  "%SolutionDir%"^
  "%ProjectDir%\_VSPG"^
  "%ProjectDir%"
if errorlevel 1 (
  if not "%FeedbackFile%"=="" (
    call :Echos VSPG execution fail. Touching "%FeedbackFile%" .
    call :Touch "%FeedbackFile%" 
  )
  exit /b 4
)


REM ======== call VSPG-Prebuild8.bat or VSPG-Postbuild8.bat ======== 
REM ====== which one to call is determined by SubworkBatfile =======

call "%bootsdir%\SearchAndExecSubbat.bat" Greedy0 "%SubworkBatfile%" %VSPG_VSIDE_ParamsPack% "%bootsdir%"

if errorlevel 1 ( 
  if not "%FeedbackFile%"=="" (
    call :Echos VSPG execution fail. Touching "%FeedbackFile%" .
    call :Touch "%FeedbackFile%" 
  )
  exit /b 4
)

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

:Touch
	REM Touch updates a file's modification time to current.
	REM NOTE: No way to check for success/fail here. So, call it only when 
	REM you have decided to fail the whole bat.
	
	copy /b "%~1"+,, "%~1" >NUL 2>&1
exit /b %ERRORLEVEL%
