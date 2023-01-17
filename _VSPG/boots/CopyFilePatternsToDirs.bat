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

if "%3" == "" (
	call :Echos [ERROR] Missing parameters.
	exit /b 4
)

:CopyFilePatternsToDirs
REM Copy files of various patterns to multiple destination directories.
REM This function is a cycled wrapper of CopyFilePatterns.bat, to suppor multiple dirs.
REM 
REM Usage:
REM 	CopyFilePatternsToDirs <SrcDir> <Varname-SrcPatterns> <Varname-DstRootDirs> [DstSubdir]
REM 
REM <SrcDir>              : One source directory.
REM <Varname-SrcPatterns> : The varname that contains source-patterns.
REM <Varname-DstRootDirs> : The varname that contains destination dirs.
REM [DstSubdir]           : (optional) Extra subdir inside a <DstRootDir>, example: "x64\Debug"
REM 
REM See CopyFilePatterns.bat for other memos.

set srcdir=%~1
set varname_src=%~2
set varname_dst=%~3
set subdirpart=%~4

call set src_patterns=%%%varname_src%%%
call set dst_dirs=%%%varname_dst%%%

if not defined src_patterns (
	call :Echos [ERROR] The varname '%varname_src%' refers to empty list of patterns.
	exit /b 4
)

if not defined dst_dirs (
	call :Echos [ERROR] The varname '%varname_dst%' refers to empty list of directories.
	exit /b 4
)

if defined subdirpart (
	set subdirpart=\%subdirpart%
)


for %%d in (%dst_dirs%) do (

	set d_final=%%~d%subdirpart%
	if not exist "!d_final!" ( 
		call :EchoAndExec mkdir "!d_final!"
		if errorlevel 1 (
		  REM This can happen if user assigns a dir with non-existing driver letter.
		  call :Echos [ERROR] Cannot create directory "!d_final!" .
		  exit /b 4
		)
	)
	
	call "%batdir%\CopyFilePatterns.bat" "%srcdir%" "!d_final!" %src_patterns%
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
