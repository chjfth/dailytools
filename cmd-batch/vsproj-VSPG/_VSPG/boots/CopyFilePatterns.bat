@echo off
setlocal EnableDelayedExpansion

: This is a function :

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.

:CopyFilePatterns
REM Copy files of various patterns to destination directory.
REM We need this bcz Windows copy cmd only accepts one wildcard pattern per execution.
REM Param1: One source folder.
REM Param2: One destination folder.
REM Params remain: Each one is a pattern, like: *.exe *.dll .
REM Memo for pattern: 
REM * If a pattern contains no backslash, then these files are considered from source folder.
REM * This function currently does not resurce into subdirectory for source files.
REM * If a pattern contains backslash, for example,
REM      d:\test\foo.exe 
REM      d:\test\*.dll 
REM then it is considered absolute path, and Param1 is not userd..

  setlocal
  set AllPatterns=
  set isFileMet=false

  set DirSrc=%~1
  shift
  
  set DirDst=%~1
  shift
  
  if not exist "%DirDst%" mkdir "%DirDst"
  
:loop_CopyFilePatterns
  set pattern=%~1
  set AllPatterns=%AllPatterns% %pattern%
  
  if "%pattern%" == "" (
    REM All patterns finished. Do we really copy any files? If none, assert error.
    
    if "%isFileMet%" == "false" (
      call :Echos [VSPG-Error] No files are found by your patterns: %AllPatterns%
      exit /b 4
    ) else (
      exit /b 0
    )
  )
  
:process_pattern
  REM Prompt the user the currently processing pattern
  call "%bootsdir%\IsSubStr.bat" hasAsterisk "%pattern%" *
  call "%bootsdir%\IsSubStr.bat" hasQuesmark "%pattern%" ?
  call "%bootsdir%\IsSubStr.bat" has1 "%hasAsterisk%%hasQuesmark%" 1
  if "%has1%" == "1" (
    call :Echos Copying files matching pattern "%pattern%" ...
  )

  REM If %pattern% has no backslash in it, prepend %DirSrc% to make a pattern with dir-prefix.
  call "%bootsdir%\IsSubStr.bat" hasBkSlash "%pattern%" \
  if "%hasBkSlash%" == "1" (
    set dirpfx_pattern=%pattern%
  ) else (
    set dirpfx_pattern=%DirSrc%\%pattern%
  )
  
  set seefile=
  for %%g in ("%dirpfx_pattern%") do (
    set seefile=%%~g
    
    REM ---- call :EchoAndExec copy "%%g" "%DirDst%"
    REM ---- Use following instead:
    call "%bootsdir%\PathSplit.bat" "!seefile!" __thisdir thisfilenam
    call "%bootsdir%\LoopExecUntilSucc.bat" #5# "%bootsdir%\vspg_copy1file.bat" "!seefile!" "%DirDst%\!thisfilenam!"
    REM
    if errorlevel 1 (
      call :Echos [ERROR] Copy file failed after multiple retries!
      exit /b 4
    )
  )
  
  if "%seefile%" == "" (
    call :Echos No files matching "%dirpfx_pattern%", no file copied.
  ) else (
    set isFileMet=true
  )
  
  shift
  goto :loop_CopyFilePatterns

REM -- End of :CopyFilePatterns

echo [VSPG-INTERNAL-ERROR] SHOULD NOT REACH HERE.
exit /b 444


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
