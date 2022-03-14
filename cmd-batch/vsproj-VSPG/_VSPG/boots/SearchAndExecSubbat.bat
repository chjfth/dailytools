REM This bat is a function.

REM setlocal EnableDelayedExpansion // Don't do this now, do it later! The Subbat may need to export arbitrary env-vars to outer env.
REM Chj memo: It seems I will inevitably leaks some vars to the environment, _vspg_SubbatParams etc.

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%

REM Search for a series of dirs passed in as parameters, and call subbat(s) from them.
REM Param1: "Greedy0" or "Greedy1". 
REM 	If Greedy0, only call the first found subbat.
REM 	If Greedy1, call each of the found subbats.
REM Param2: Subbat filenam (without dir prefix).
REM Param3: All params passed to subbat.
REM         (when pass in, surrounded by quotes, when calling subbat, quotes stripped)
REM Params remain: Each param is a directory to search for subbat.

  rem call :Echos CALLED WITH: %*
  
  set _tmp_greedy=%~1
  set _tmp_greedy=%_tmp_greedy:~-1%
  REM -- _tmp_greedy will be 0 or 1 (only need final char)
  
  shift
  set _vspg_SubbatFilenam=%~1
  
  shift
  set _vspg_SubbatParams=%~1
  REM    %1 example:
  REM
  REM    """D:\chj\AAA BBB\Chap03"" ""D:\chj\AAA BBB\Chap03\HelloWinD"" ""Debug"" "x64" ""D:\chj\AAA BBB\Chap03\x64\Debug"" ""HelloWin.exe"" ""HelloWin"" ""x64\Debug"""
  REM
  REM    Special Note: All directory subparam above must NOT end with a backslash, otherwise, [Shortcut1] will not work.
  
  set _vspg_SearchedDirs=
  REM -- Each searched dir will be appended to the var, with minor decoration, like this:
  REM -- [*c:\dir1*][*c:\dir2*]
  
:loop_SearchAndExecSubbat  
  
  setlocal EnableDelayedExpansion

  shift
  
  set trydir=%~1
  
  if "%trydir%" == "" exit /b 0
  
  set trydirdeco=[*%trydir%*]
  call "%batdir%\IsSubStr.bat" isfound "%_vspg_SearchedDirs%" "%trydirdeco%"
  if %isfound% == 1 goto :endlocal_GoNextLoop

  set _vspg_SearchedDirs=%_vspg_SearchedDirs%%trydirdeco%

  set trybat=%trydir%\%_vspg_SubbatFilenam%

  if not exist "%trybat%" goto :endlocal_GoNextLoop
  
  REM [Shortcut1] Just replace "" with " ; that is enough to pass VSproj's packed params to %trybat%.
  endlocal & ( call "%trybat%" %_vspg_SubbatParams:""="% )

  if errorlevel 1 exit /b 4
  
  if "%_tmp_greedy%" == "1" goto :loop_SearchAndExecSubbat
  
  exit /b 0

:endlocal_GoNextLoop
  endlocal
  goto :loop_SearchAndExecSubbat


REM =============================
REM ====== Functions Below ======
REM =============================

REM %~n0%~x0 is batfilenam
:Echos
  echo [%~n0%~x0] %*
exit /b 0

:EchoAndExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
  call %*
exit /b %ERRORLEVEL%

:EchoVar
  REM Env-var double expansion trick from: https://stackoverflow.com/a/1200871/151453
  set _Varname=%1
  for /F %%i in ('echo %_Varname%') do echo [%batfilenam%] %_Varname% = !%%i!
exit /b 0
