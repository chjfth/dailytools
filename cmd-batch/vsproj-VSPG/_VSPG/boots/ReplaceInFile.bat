setlocal EnableDelayedExpansion

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.

:ReplaceInFile
REM Thanks to:
REM https://stackoverflow.com/questions/23075953/batch-script-to-find-and-replace-a-string-in-text-file-without-creating-an-extra/23076141
REM https://ss64.com/nt/for_f.html

: This is a function, replace a string in a file, output written to a new file.
:
: Param1: Input filepath
: Param2: Output filepath
: Param3: Old word to find (must not contain whitespace)
: Param4: Replace with this new word (must not contain whitespace)
: Param5: Stampinf extra params, like "-a AMD64 -k 1.9 -v 1.0.0.1"
: 
: Issues: 
: * Blank lines will not be copied.
: * Whether filepath/filename is space tolerant, not verified.
: * File creation and writing error report is not 100% accurate.

  echo off
  set "search=%~1"
  set "replace=%~2"
  set "oldfile=%~3"
  set "newfile=%~4"
  
  if not exist "%oldfile%" (
    call :Echos [ERROR] Source file not exist: "%oldfile%"
    exit /b 4
  )
  
  call "%batdir%\DelOneFile.bat" "%newfile%"
  if errorlevel 1 (
    call :Echos [ERROR] Cannot delete stale file:"%newfile%"
    exit /b 4
  )
  
  (for /f delims^=^ eol^= %%i in (%oldfile%) do (
    set "line=%%i"
    setlocal enabledelayedexpansion
    set "line=!line:%search%=%replace%!"
    echo !line!
    endlocal
  ))>"%newfile%"

  if not exist "%newfile%" (
    call :Echos [ERROR] Target file not generated: "%newfile%"
    exit /b 4
  )
  
exit /b %ERRORLEVEL%


REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  echo %_vspgINDENTS%[%batfilenam%] %*
exit /b 0

:EchoExec
  echo %_vspgINDENTS%[%batfilenam%] EXEC: %*
exit /b 0
