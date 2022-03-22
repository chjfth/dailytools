@echo off
setlocal EnableDelayedExpansion
set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set _vspgINDENTS=%_vspgINDENTS%.
call :Echos START from %batdir%

: This is a function :

:AgileCopyOrDelete
REM Usage example:
REM 
REM   call AgileCopy.bat <SourceDir> <SourcePatterns> <DestDirs> <DestSubdir>
REM 
REM Param1: <SourceDir>
REM   The directory containing source files.
REM 
REM Param2: <SourcePatterns> 
REM   Filename or wildcard-patterns relative to <SourceDir>, separated by spaces.
REM   Each filename/pattern can have extra sub-dir prefix, e.g. foodir\*.exe .
REM   If multiple filenames/patterns given, you(the caller) need to enclose all of them 
REM   in one pair of double quotes, for example:
REM      "myprog.exe subdir\helper.dll"
REM 
REM   And, if given filename/pattern itself contains spaces, you(the caller) should.
REM   apply extra quoting, like this:
REM      "myprog.exe ""sub dir with spaces\helper.dll"" readme.txt"
REM 
REM Param3: <DestDirs>
REM   List of destination directories, separated by spaces. Also try to avoid
REM   have spaces in the directory itself. 
REM   Source files will be copied to each directory.
REM 
REM Param4: <DestSubdir> (optional)
REM   This is extra subdir beneath each <DestDirs>. This help you place 
REM   source files from different BuildConf/Platform into different subdirs.

  if "%~1" == "" (
    call :Echos [ERROR] Missing parameter 1: SourceDir .
    exit /b 4
  )
  if "%~2" == "" (
    call :Echos [ERROR] Missing parameter 2: SourcePatterns .
    exit /b 4
  )
  if "%~3" == "" (
    call :Echos [ERROR] Missing parameter 3: DestDirs .
    exit /b 4
  )

  call :UnpackDoubleQuotes SourceDir "%~1"
  call :UnpackDoubleQuotes SourcePatterns "%~2"
  call :UnpackDoubleQuotes DestDirs "%~3"
  call :UnpackDoubleQuotes DestSubdir "%~4"

  if "%DestSubdir%" == "" (
    call :Echos [ERROR] Calling AgileCopy function, missing parameters, 4 parameters required.
    exit /b 4
  )


  for %%d in (%DestDirs%) do (

	if "%DestSubdir%" == "" (
	  set d_final=%%~d
	) else (
	  set d_final=%%~d\%DestSubdir%
	)

	if not exist "!d_final!" (
		mkdir "!d_final!"
		if errorlevel 1 (
		  REM This can happen if user assigns a dir with non-existing driver letter.
		  call :Echos [ERROR] Cannot create directory "!d_final!" .
		  exit /b 4
		)
	)
	

	call "%bootsdir%\CopyFilePatterns.bat" "%SourceDir%" "!d_final!" %SourcePatterns%
	if errorlevel 1 ( 
		call :Echos [ERROR] Error occurred when copying file to "!d_final!" .
		exit /b 4 
	)
  )

exit /b 0


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

:SetErrorlevel
  REM Usage example:
  REM call :SetErrorlevel 4
exit /b %1

:PackDoubleQuotes
  REM Take whole %* as input, replace each " with "" and return the result string.
  REM The return value is put in global var _vspg_dqpacked .
  set allparams=%*
  set _vspg_dqpacked=%allparams:"=""%
exit /b 0

:UnpackDoubleQuotes
  setlocal & set allparams=%~2
  set unpacked=%allparams:""="%
  endlocal & (set %~1=%unpacked%)
exit /b 0

