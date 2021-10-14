@echo off
REM VSPG-PreBuild7.bat $(SolutionDir) $(ProjectDir) $(Configuration) $(PlatformName) $(TargetDir) $(TargetFileName) $(TargetName)
REM ==== boilerplate code >>>
REM
set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
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
call :EchoVar batdir
call :EchoVar SolutionDir
call :EchoVar ProjectDir
call :EchoVar BuildConf
call :EchoVar PlatformName
call :EchoVar TargetDir
call :EchoVar TargetFilenam
call :EchoVar TargetName

REM Try to call PreBuild-SubWCRev1.bat from one of three predefined directories,
REM whichever is encountered first. But if none found, just do nothing.
REM If you need this PreBuild-SubWCRev1.bat to run, just copy and tune it from
REM PreBuild-SubWCRev1.bat.sample .

call :SearchAndExecSubbat PreBuild-SubWCRev1.bat^
  "%ProjectDir%"^
  "%ProjectDir%\_VSPG"^
  "%SolutionDir%\_VSPG"^
  "%batdir%"
if errorlevel 1 exit /b 4


REM ==== Call Team-Prebuild7.bat if exist. ====
call :SearchAndExecSubbat Team-PreBuild7.bat^
  "%SolutionDir% %ProjectDir% %BuildConf% %PlatformName% %TargetDir% %TargetFilenam% %TargetName%"^
  "%ProjectDir%\_VSPG"^
  "%SolutionDir%\_VSPG"^
  "%batdir%"
if errorlevel 1 exit /b 4

REM ==== Call Personal-Prebuild7.bat if exist. ====
call :SearchAndExecSubbat Personal-PreBuild7.bat^
  "%SolutionDir% %ProjectDir% %BuildConf% %PlatformName% %TargetDir% %TargetFilenam% %TargetName%"^
  "%ProjectDir%\_VSPG"^
  "%SolutionDir%\_VSPG"^
  "%batdir%"
if errorlevel 1 exit /b 4


goto :END

REM =============================
REM ====== Functions Below ======
REM =============================

:SetErrorlevel
exit /b %1

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

:SearchAndExecSubbat
REM Search for a series of dirs passed in as parameters, and call Subbat from 
REM one of those dirs, whichever is found first.
REM Param1: Subbat filenam (without dir prefix).
REM Param2: All params passed to Subbat.
REM         (when pass in, surrounded by quotes, when calling Subbat, quotes stripped)
REM Params remain: Each param is a directory to search for Subbat.
  setlocal
  Set SubbatFilenam=%~1
  shift
  Set SubbatParams=%~1
  shift
  
:loop_SearchAndExecSubbat  
  
  set trydir=%~1
  
  if "%trydir%" == "" (
    endlocal
    exit /b 0
  )
  
  set trybat=%trydir%\%SubbatFilenam%
  if exist "%trybat%" (
    call :Echos Now exec: "%trybat%" %SubbatParams%
    call "%trybat%" %SubbatParams%
    if errorlevel 1 (
      endlocal
      exit /b 4
    )
  )
  
  shift
  goto :loop_SearchAndExecSubbat

REM -- End of :SearchAndExecSubbat

:END
