
:PathSplit
REM Usage: 
REM call PathSplit.bat "c:\program files\d2\d3.txt" dname fname
REM Output dname=c:\program files\d2
REM Output fname=d3.txt
  
  setlocal
  REM set local ensures setting `path` does not overwrite caller env's PATH.
  set path=%~1
  For %%A in ("%path%") do (
    set Folder=%%~dpA
    set Name=%%~nxA
  )
  endlocal & (
    set "%~2=%Folder:~0,-1%"
    set "%~3=%Name%"
  )
exit /b %ERRORLEVEL%
