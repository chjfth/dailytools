@echo off
setlocal EnableDelayedExpansion
set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.

: This is a function.
: Just like CMD's internal `copy`, but it does one extra thing:
: Check file modification time first, if source-time equals dest-time, the copy is 
: considered success already.
:
: Usage scenario: When building a whole VSIDE solution with PostBuild-CopyOutput4.bat 
: in each vcxproj, two parallel build threads can copy the same source file to the 
: same destination and one thread will fail the copy. We know that is NOT a true fail,
: if the final timestamps are the same.
: So calling this .bat wrapped inside LoopExecUntilSucc.bat is a good workaround.

:vspg_copy1file
REM Usage: 
REM call vspg_copy1file.bat sourcefile destfile
REM
REM Note: The passed-in destfile MUST be a filepath, not a dirpath,
REM       bcz, this .bat need to know the exact filepath so to check its timestamp.

set srcfile=%~1
set dstfile=%~2

call "%batdir%\GetParentDir.bat" dstdir "%dstfile%"
if not exist "%dstdir%" (
	mkdir "%dstdir%"
	if errorlevel 1 exit /b 4
)

call "%batdir%\IsFiletimeSame.bat" "%srcfile%" "%dstfile%"

if not errorlevel 1 (
	call :Echos Already same: "%srcfile%" and "%dstfile%"
	exit /b 0
)

call copy "%srcfile%" "%dstfile%"

exit /b %ERRORLEVEL%


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
  %*
exit /b %ERRORLEVEL%

