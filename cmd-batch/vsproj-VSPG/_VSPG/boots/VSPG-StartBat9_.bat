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
call "%bootsdir%\PathSplit.bat" "%bootsdir%" userbatdir __temp
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
REM But if user does not like some env-var from outer env, he can clear it to empty explicitly.
REM The search order is wide to narrow.

call "%bootsdir%\SearchAndExecSubbat.bat" Greedy1 VSPG-StartEnv.bat %VSPG_VSIDE_ParamsPack%^
  "%userbatdir%"^
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


REM ======== Loading User VSPG-Prebuild8.bat or VSPG-Postbuild8.bat ======== 

REM Note for VSPG-Prebuild8.bat and VSPG-Postbuild8.bat in advance:
REM When VSPG-Prebuild8.bat and VSPG-Postbuild8.bat calls their own subbats. Those bats should do
REM non-greedy search, bcz user (probably) wants to override outer env's sub-work with his own one.
REM But if user wants outer sub-work as well, he should call the outer sub-work explicitly.
REM The search order is narrow to wide.

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
  echo %_vspgINDENTS%[%batfilenam%] %*
exit /b 0

:EchoExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
exit /b 0

:EchoVar
  REM Env-var double expansion trick from: https://stackoverflow.com/a/1200871/151453
  set _Varname=%1
  for /F %%i in ('echo %_Varname%') do echo %_vspgINDENTS%[%batfilenam%] %_Varname% = !%%i!
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

goto :END


:END
exit /b %ERRORLEVEL%
