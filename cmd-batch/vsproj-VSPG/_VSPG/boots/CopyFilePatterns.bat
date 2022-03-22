@echo off
setlocal EnableDelayedExpansion

: This is a function :

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.

if "%VSPG_COPYFILE_DO_DELETE%" == "1" (
	call :Echos See VSPG_COPYFILE_DO_DELETE=1, run in delete mode.
)

:CopyFilePatterns
REM Copy files of various patterns to destination directory.
REM We need this bcz Windows copy cmd only accepts one wildcard pattern per execution.
REM Param1: One source folder.
REM Param2: One destination folder.
REM Params remain: Each one is a pattern, like: *.exe *.dll .
REM Memo for pattern: 
REM * If a pattern contains no backslash, then these files are considered from source folder.
REM * This function currently does not resurce into subdirectory for source files.
REM * If a pattern contains a colon, for example,
REM      d:\test\foo.exe 
REM      d:\test\*.dll 
REM then it is considered absolute path, and Param1 is not userd.
REM 
REM [Env-var input]
REM If env-var VSPG_COPYFILE_DO_DELETE=1, target file is actually deleted.
REM This feature can be used in VSPU-CleanProject.bat .
REM If a file pattern contains wildcard(* or ?), then the wildcard is matched
REM against source folder instead of the target folder.

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
  call "%bootsdir%\IsSubStr.bat" hasWildcard "%hasAsterisk%%hasQuesmark%" 1
  if "%hasWildcard%" == "1" (
    if "%VSPG_COPYFILE_DO_DELETE%" == "1" (
      call :Echos Deleting files matching pattern "%pattern%" ...
    ) else (
      call :Echos Copying files matching pattern "%pattern%" ...
    )
  )

  REM If %pattern% has no : in it, prepend %DirSrc% to make a pattern with dir-prefix.
  REM If %pattern% has : in it, we consider it an absolute path, so don't prepend dir-prefix.
  call "%bootsdir%\IsSubStr.bat" hasColon "%pattern%" :
  if "%hasColon%" == "1" (
    set dirpfx_pattern=%pattern%
  ) else (
    set dirpfx_pattern=%DirSrc%\%pattern%
  )

  set seefile=
  for %%g in ("%dirpfx_pattern%") do (

    set seefile=%%~g
    call "%bootsdir%\PathSplit.bat" "!seefile!" __thisdir thisfilenam
    
    if "%VSPG_COPYFILE_DO_DELETE%" == "1" (
      call :EchoAndExec del "%DirDst%\!thisfilenam!"
      REM For simplicity, ignore deleting error.
    ) else (
      REM ---- call :EchoAndExec copy "%%g" "%DirDst%"
      REM ---- Use following instead:
      call "%bootsdir%\LoopExecUntilSucc.bat" #5# "%bootsdir%\vspg_copy1file.bat" "!seefile!" "%DirDst%\!thisfilenam!"
      REM
      if errorlevel 1 (
        call :Echos [ERROR] Copy file failed after multiple retries!
        exit /b 4
      )
    )
  )
  
  if "%seefile%" == "" (
    if "%VSPG_COPYFILE_DO_DELETE%" == "1" (
      call :Echos No files matching "%dirpfx_pattern%", no file deleted.
    ) else (
      call :Echos No files matching "%dirpfx_pattern%", no file copied.
    )
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

:EchoVar
  setlocal & set Varname=%~1
  call echo %_vspgINDENTS%[%batfilenam%]%~2 %Varname% = %%%Varname%%%
exit /b 0
