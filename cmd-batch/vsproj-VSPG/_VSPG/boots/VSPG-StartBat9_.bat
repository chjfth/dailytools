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


call :EchoVar SubworkBatpath
call :EchoVar FeedbackFile

if not "%FeedbackFile%"=="" (
  if not exist "%FeedbackFile%" (
	call :Echos [VSPG-ERROR] Not-existing feedback file: "%FeedbackFile%"
	exit /b 4
  )
)

if not exist "%SubworkBatpath%" (
  call :Echos [VSPG-ERROR] SubworkBatpath NOT found: "%SubworkBatpath%"
  call :SetErrorlevel 4
  exit /b 4
)

set VSPG_VSIDE_ParamsDiscrete="%SolutionDir%" "%ProjectDir%" "%BuildConf%" "%PlatformName%" "%TargetDir%" "%TargetFilenam%" "%TargetName%" "%IntrmDir%"
call "%bootsdir%\DQescape_NoTBS.bat" %VSPG_VSIDE_ParamsDiscrete%
set VSPG_VSIDE_ParamsPack=%DQescape_NoTBS_Output%

call "%bootsdir%\SearchAndExecSubbat.bat" "%SubworkBatfile%" %VSPG_VSIDE_ParamsPack% "%bootsdir%"

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

:SplitDir
  REM Param1: C:\dir1\file1.txt
  REM Param2: Output varname, receive: C:\dir1
  for %%a in (%1) do set "_retdir_=%%~dpa"
  set %2=%_retdir_:~0,-1%
exit /b

:Touch
	REM Touch updates a file's modification time to current.
	REM NOTE: No way to check for success/fail here. So, call it only when 
	REM you have decided to fail the whole bat.
	
	copy /b "%~1"+,, "%~1" >NUL 2>&1
exit /b

goto :END


:END
rem echo [%batfilenam%] END for %ProjectDir%
