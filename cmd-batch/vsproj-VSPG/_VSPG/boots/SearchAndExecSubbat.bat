REM This bat is a function.

setlocal EnableDelayedExpansion

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%

REM Search for a series of dirs passed in as parameters, and call Subbat from 
REM one of those dirs, whichever is found first.
REM Param1: Subbat filenam (without dir prefix).
REM Param2: All params passed to Subbat.
REM         (when pass in, surrounded by quotes, when calling Subbat, quotes stripped)
REM Params remain: Each param is a directory to search for Subbat.

  setlocal
  Set SubbatFilenam=%~1
rem call :Echos CALLED WITH: %*
  shift
  Set _SubbatParams_=%1
  REM -- %_SubbatParams_% example:
  REM
  REM    """D:\chj\AAA BBB\Chap03"" ""D:\chj\AAA BBB\Chap03\HelloWinD"" ""Debug"" "x64" ""D:\chj\AAA BBB\Chap03\x64\Debug"" ""HelloWin.exe"" ""HelloWin"" ""x64\Debug"""
  REM
  REM    Special Note: All directory subparam above must NOT end with a backslash, otherwise, [Shortcut1] will not work.
  Set SubbatParams=%~1
  shift
  
  set SearchedDirs=
  REM -- Each searched dir will be appended to the var, with minor decoration, like this:
  REM -- [*c:\dir1*][*c:\dir2*]
  
:loop_SearchAndExecSubbat  
  
  set trydir=%~1
  
  if "%trydir%" == "" (
    endlocal
    exit /b 0
  )
  
  set trydirdeco=[*%trydir%*]
  call "%batdir%\IsSubStr.bat" isfound "%SearchedDirs%" "%trydirdeco%"
  if %isfound% == 1 (
    shift
    goto :loop_SearchAndExecSubbat
  )
  set SearchedDirs=%SearchedDirs%%trydirdeco%

  set trybat=%trydir%\%SubbatFilenam%

  if exist "%trybat%" (
    REM [Shortcut1] Just replace "" with " ; that is enough to pass VSproj params to %trybat%
    call "%trybat%" !SubbatParams:""="!
    if errorlevel 1 (
      endlocal
      exit /b 4
    )
  )
  
  shift
  goto :loop_SearchAndExecSubbat

exit /b 444

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
