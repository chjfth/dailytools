REM This bat is a function.

REM setlocal EnableDelayedExpansion // Don't do this now, do it later! The Subbat may need to export arbitrary env-vars to outer env.
REM Chj memo: It seems I will inevitably leaks some vars to the environment, _vspg_SubbatParams etc.

set _tmp_batfilenam=%~n0%~x0
set _tmp_batdir=%~dp0
set _tmp_batdir=%_tmp_batdir:~0,-1%
set _tmp_outerbatnam=%batfilenam%

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

  set batfilenam=%_tmp_batfilenam%
  set batdir=%_tmp_batdir%
  set _vspgINDENTS=%_vspgINDENTS%.

  shift
  
  set trydir=%~1
  
  if "%trydir%" == "" exit /b 0
  
  set trydirdeco=[*%trydir%*]
  
  if defined _vspg_SearchedDirs (
    call "%batdir%\IsSubStr.bat" isfound "%_vspg_SearchedDirs%" "%trydirdeco%"
    if "%isfound%" == "1" goto :endlocal_GoNextLoop
  )

  set _vspg_SearchedDirs=%_vspg_SearchedDirs%%trydirdeco%

  set trybat=%trydir%\%_vspg_SubbatFilenam%

  if not exist "%trybat%" goto :endlocal_GoNextLoop
  
  REM [Shortcut1] Just replace "" with " ; that is enough to pass VSproj's packed params to %trybat%.
  endlocal & ( call "%trybat%" %_vspg_SubbatParams:""="% )

  set _vspg_LastError=%ERRORLEVEL%

  REM Enter local context again (N2).
  setlocal EnableDelayedExpansion

  set batfilenam=%_tmp_batfilenam%
  set batdir=%_tmp_batdir%
  set _vspgINDENTS=%_vspgINDENTS%.
  
  if not "%_vspg_LastError%" == "0" (
REM call :Echos [[[[%_tmp_outerbatnam%]]]] GOT [ERRORLEVEL=%_vspg_LastError%] from Subbat.
    exit /b 4
  )

REM  call :Echos [[[[%_tmp_outerbatnam%]]]] Successfully called Subbat.

  if "%_tmp_greedy%" == "1" (
    goto :endlocal_GoNextLoop
  )
  
  exit /b 0

:endlocal_GoNextLoop
  endlocal
  goto :loop_SearchAndExecSubbat


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
