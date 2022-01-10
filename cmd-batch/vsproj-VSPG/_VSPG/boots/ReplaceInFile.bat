
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
: Issue: Whether filepath/filename is space tolerant, not verified.

  echo off
  set "search=%1"
  set "replace=%2"
  set "oldfile=%3"
  set "newfile=%4"
  (for /f delims^=^ eol^= %%i in (%oldfile%) do (
    set "line=%%i"
    setlocal enabledelayedexpansion
    set "line=!line:%search%=%replace%!"
    echo !line!
    endlocal
  ))>"%newfile%"

exit /b
