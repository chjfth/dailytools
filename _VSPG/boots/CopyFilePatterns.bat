@echo off
setlocal EnableDelayedExpansion

: This is a function :

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.

if "%vspg_COPYORCLEAN_DO_CLEAN%" == "1" (
	call :EchosV1 See vspg_COPYORCLEAN_DO_CLEAN=1, run in delete mode.
)

if "%~3" == "" (
	call :Echos [ERROR] Missing parameters.
	exit /b 4
)

:CopyFilePatterns
REM Copy files of various patterns to destination directory.
REM We need this bcz Windows copy cmd only accepts one wildcard pattern per execution.
REM Param1: One source folder.
REM Param2: One destination folder.
REM Params remain: Each one is a pattern, like: *.exe *.dll .
REM Memo for pattern: 
REM * If a pattern contains no backslash, then these files are considered from source folder.
REM * For a wildcard pattern, this function currently does not recurse into subdirectory.
REM * If a pattern contains a colon, for example,
REM      d:\test\foo.exe 
REM      d:\test\*.dll 
REM   then it is considered absolute path, and Param1 is ignored for this pattern.
REM * For a non-wildcard pattern, a new destination filename can be assigned, using '#'.
REM   Example:
REM     foo.exe#foo-x64.exe
REM   In destination dir, ther will be foo-x64.exe .
REM 
REM [Env-var input]
REM If env-var vspg_COPYORCLEAN_DO_CLEAN=1, destination file is actually deleted,
REM -- but source-file is not deleted.
REM This feature is used by VSPU-CopyOrClean.bat .
REM If a file pattern contains wildcard(* or ?), then the wildcard is matched
REM against source folder instead of the target folder.

  set AllPatterns=
  set isFileMet=false

  set DirSrc=%~1
  shift
  
  set DirDst=%~1
  shift
  
  if not exist "%DirDst%" call :EchoAndExec mkdir "%DirDst%"
  
:loop_CopyFilePatterns
  set pattern=%~1
  set AllPatterns=%AllPatterns% %pattern%
  
  if "%pattern%" == "" (
    REM All patterns finished. Do we really copy any files? If none, assert error.
    
    if "%isFileMet%" == "false" if not defined vspg_COPYORCLEAN_DO_CLEAN (
      call :Echos [VSPG-Error] No files are found by your patterns: %AllPatterns%
      exit /b 4
    )

    exit /b 0
  )
  
:process_pattern
  REM Prompt the user the currently processing pattern
  call "%batdir%\IsSubStr.bat" hasAsterisk "%pattern%" *
  call "%batdir%\IsSubStr.bat" hasQuesmark "%pattern%" ?
  call "%batdir%\IsSubStr.bat" hasWildcard "%hasAsterisk%%hasQuesmark%" 1
  if "%hasWildcard%" == "1" (
    if "%vspg_COPYORCLEAN_DO_CLEAN%" == "1" (
      call :Echos Deleting files matching pattern "%pattern%" ...
    ) else (
      call :Echos Copying files matching pattern "%pattern%" ...
    )
  )

  REM If %pattern% has no ':' in it, prepend %DirSrc% to make a pattern with dir-prefix.
  REM If %pattern% has ':' in it, we consider it an absolute path, so don't prepend dir-prefix.
  call "%batdir%\IsSubStr.bat" hasColon "%pattern%" :
  if "%hasColon%" == "1" (
    set srcpath_pattern=%pattern%
  ) else (
    set srcpath_pattern=%DirSrc%\%pattern%
  )

  REM Example of a srcpath_pattern: 
  REM 	d:\myproj\bin-v100\x64\Debug\*.exe
  REM 	d:\myproj\bin-v100\x64\Debug\foo.exe
  REM 	d:\myproj\bin-v100\x64\Debug\foo.exe#foo-x64.exe

  REM Check if srcpath_pattern contains '#', if so, split it.
  set newfilenam=
  call :SplitBySharp "%srcpath_pattern%" oldfilepath newfilenam
  if defined newfilenam (
    REM Has '#', tweak srcpath_pattern to be a normal filepath
    set srcpath_pattern=%oldfilepath%
  )

  REM Expand the wildcard patten into individual filepaths, using CMD `for` feature.
  set seefile=
  for %%g in ("%srcpath_pattern%") do (

    REM Now %g is a concrete filepath.

    set seefile=%%~g
    
    if not defined newfilenam (
      call "%batdir%\PathSplit.bat" "!seefile!" __nouse_dir newfilenam
    )
    
    set curdstpath=%DirDst%\!newfilenam!

    if "%vspg_COPYORCLEAN_DO_CLEAN%" == "1" (
      if exist "!curdstpath!" (
        call :EchoAndExec del "!curdstpath!"
        REM For simplicity, ignore deleting error.
      ) else (
        if defined vspg_DO_SHOW_VERBOSE (
          call :Echos Already deleted "!curdstpath!"
        )
      )
    ) else (
      REM ---- call :EchoAndExec copy "%%g" "%DirDst%"
      REM ---- Use following instead:
      call "%batdir%\LoopExecUntilSucc.bat" #5# "%batdir%\vspg_copy1file.bat" "!seefile!" "!curdstpath!"
      REM
      if errorlevel 1 (
        call :Echos [ERROR] Copy file failed after multiple retries!
        exit /b 4
      )
    )
  )
  
  if "%seefile%" == "" (
    if "%vspg_COPYORCLEAN_DO_CLEAN%" == "1" (
      call :Echos No files matching "%srcpath_pattern%", no file deleted.
    ) else (
      call :Echos No files matching "%srcpath_pattern%", no file copied.
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

:EchosV1
  REM echo %* only when vspg_DO_SHOW_VERBOSE=1 .
  setlocal & set LastError=%ERRORLEVEL%
  if not defined vspg_DO_SHOW_VERBOSE goto :_EchosV1_done
  echo %_vspgINDENTS%[%batfilenam%]# %*
:_EchosV1_done
exit /b %LastError%

:EchoAndExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
  call %*
exit /b %ERRORLEVEL%

:EchoVar
  setlocal & set Varname=%~1
  call echo %_vspgINDENTS%[%batfilenam%]%~2 %Varname% = %%%Varname%%%
exit /b 0

:SplitBySharp
  REM Split a string into two substrings, using # as separator.
  REM Param1: Input string to split.
  REM Param2: [out] varname to store first substring.
  REM Param3: [out] varname to store second substring.
  setlocal
  set inputstr=%~1
  for /F "delims=# tokens=1,*" %%a in ("%inputstr%") do (
    set sub1=%%a
    set sub2=%%b
  )
  
  endlocal & (
    set "%~2=%sub1%"
    set "%~3=%sub2%"
  )
exit /b 0
