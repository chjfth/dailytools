@echo off
setlocal EnableDelayedExpansion

:Function: RegQuerySz

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set dirbatfunc=%batdir%
set _vspgINDENTS=%_vspgINDENTS%.

REM Query(Read) a registry item, from 32-bit process view.
REM Usage: 
REM call RegQuerySz.bat varResult "HKLM\SOFTWARE\Microsoft\MSBuild\ToolsVersions\4.0" "MSBuildToolsPath"
REM
REM Output:
REM 	varResult=$(MSBuildExtensionsPath32)\Microsoft.Cpp\v4.0\
REM or
REM		varResult=
REM 
REM If the request regkey or regitem does not exist, varResult is empty.
REM If some error occurs, varResult is empty as well.

if "%~1" == "" (
	call :Echos [ERROR] Missing 1st parameter: varResult.
	exit /b 4
)

if "%~2" == "" (
	call :Echos [ERROR] Missing 2nd parameter: an regkey path.
	exit /b 4
)

if "%~3" == "" (
	call :Echos [ERROR] Missing 3rd parameter: an regitem name.
	exit /b 4
)

set regkeypath=%~2
set regitemname=%~3
set varResult=

set execmd=reg query "%regkeypath%" /reg:32 /v "%regitemname%"

for /F "tokens=1,2,*" %%a in ('%execmd% 2^>nul') do (
	if "%%b" == "REG_SZ" (
		set varResult=%%c
	)
) 2>2.txt >1.txt

:DONE

endlocal & (
	set "%~1=%varResult%"
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

:SetErrorlevel
  REM Usage example:
  REM call :SetErrorlevel 4
exit /b %1
