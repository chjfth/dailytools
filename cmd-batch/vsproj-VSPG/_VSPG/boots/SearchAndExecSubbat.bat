REM This bat is a function.

setlocal EnableDelayedExpansion

REM Search for a series of dirs passed in as parameters, and call Subbat from 
REM one of those dirs, whichever is found first.
REM Param1: Subbat filenam (without dir prefix).
REM Param2: All params passed to Subbat.
REM         (when pass in, surrounded by quotes, when calling Subbat, quotes stripped)
REM Params remain: Each param is a directory to search for Subbat.

  if "%bootsdir%"=="" (
    echo [ERROR] Required bat-var "bootsdir" not defined.
    exit /b 3
  )

  REM Put vspg-flatten-args.exe into PATH. Otherwise, I cannot cope with the situation of 
  REM %bootsdir% containing whitespace(s). The
  REM    for ... IN (`D:\some dir\some.exe ...`) DO ...
  rem would fail if the exe's path contains any whitespace.
  PATH=%bootsdir%;%PATH%

  setlocal
  Set SubbatFilenam=%~1
  shift
  Set _SubbatParams_=%1
  REM -- %_SubbatParams_% example:
  REM
  REM    """D:\chj\AAA BBB\Chap03"" ""D:\chj\AAA BBB\Chap03\HelloWinD"" ""Debug"" "x64" ""D:\chj\AAA BBB\Chap03\x64\Debug"" ""HelloWin.exe"" ""HelloWin"" ""x64\Debug"""
  REM
  REM    Special Note: All directory subparam above must NOT end with a bkslash, otherwise, vspg-flatten-args fails.
  shift
  
:loop_SearchAndExecSubbat  
  
  set trydir=%~1
  
  if "%trydir%" == "" (
    endlocal
    exit /b 0
  )
  
  set trybat=%trydir%\%SubbatFilenam%

  if exist "%trybat%" (
	for /F "usebackq delims=" %%i IN (`vspg-flatten-args %_SubbatParams_%`) DO set SubbatParams=%%i
    "%trybat%" !SubbatParams!
    if errorlevel 1 (
      endlocal
      exit /b 4
    )
  )
  
  shift
  goto :loop_SearchAndExecSubbat

