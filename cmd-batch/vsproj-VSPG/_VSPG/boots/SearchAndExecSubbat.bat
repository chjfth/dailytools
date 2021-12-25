REM This bat is a function.

setlocal EnableDelayedExpansion

REM Search for a series of dirs passed in as parameters, and call Subbat from 
REM one of those dirs, whichever is found first.
REM Param1: Subbat filenam (without dir prefix).
REM Param2: All params passed to Subbat.
REM         (when pass in, surrounded by quotes, when calling Subbat, quotes stripped)
REM Params remain: Each param is a directory to search for Subbat.

  setlocal
  Set SubbatFilenam=%~1
  shift
  Set _SubbatParams_=%1
  REM -- %_SubbatParams_% example:
  REM
  REM    """D:\chj\AAA BBB\Chap03"" ""D:\chj\AAA BBB\Chap03\HelloWinD"" ""Debug"" "x64" ""D:\chj\AAA BBB\Chap03\x64\Debug"" ""HelloWin.exe"" ""HelloWin"" ""x64\Debug"""
  REM
  REM    Special Note: All directory subparam above must NOT end with a backslash, otherwise, [Shortcut1] will not work.
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
    REM [Shortcut1] Just replace "" with " ; that is enough to pass VSproj params to %trybat%
    call "%trybat%" !SubbatParams:""="!
    if errorlevel 1 (
      endlocal
      exit /b 4
    )
  )
  
  shift
  goto :loop_SearchAndExecSubbat

