@echo off
setlocal EnableDelayedExpansion

set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set batdir2=%batdir:\=\\%

set ThisWslDistributionName=%~1
set UnixUser=%~2

REM =========== BAT PARAMETER CHECKING ==============

if not defined ThisWslDistributionName (
	echo. Need a DistributionName as parameter.
	echo. This .bat is compatible with Win10.21H2 or above.
	echo. Examples:
	echo.     WSL-start.bat Ubuntu-22.04
	echo.     WSL-start.bat Ubuntu-22.04 bob
	exit /b 4
)

if not "" == "%~3" (
	call :Echos Error: Only one or two parameter should be given.
	exit /b 4
)

call :HasSpaceChar %ThisWslDistributionName%
if %ERRORLEVEL%==1 (
	call :Echos Error: DistributionName must NOT have space char in it.
	exit /b 4
)

if defined UnixUser (
	call :HasSpaceChar %UnixUser%
	if %ERRORLEVEL%==1 (
		call :Echos Error: UnixUser must NOT have space char in it.
		exit /b 4
	)
	set _u_param=-u %UnixUser%
) else (
	set _u_param=
)

REM =========== ENV CHECKING ==============

set unix_checkdir=%batdir%\rootfs\root
if not exist "%unix_checkdir%" (
	call :Echos Error: The folder "%unix_checkdir%" does not exist.
	call :Echos You must have a Linux file-system inside "%batdir%\rootfs" for this bat to work.
	exit /b 4
)

REM =========== PREPARE .REG FILE ==============

REM We need to generate a GUID for current DistributionName,
REM We can ensure it being unique by calcuating %ThisWslDistributionName%.
echo %ThisWslDistributionName% > wslname.tmp

call :GetFileMD5 wslname.tmp md5

if not defined md5 (
	call :Echos Error: Cannot generate a GUID for ThisWslDistributionName.
	exit /b 4
)

set md5guid=%md5:~0,8%-%md5:~8,4%-%md5:~12,4%-%md5:~16,4%-%md5:~20,12%
rem For "Ubuntu-22.04", md5guid is
rem     2b0c8e79-b4a5-1bfc-49cd-4f344cbfe444

set regfile=%batdir%\WSL-green.reg

echo Windows Registry Editor Version 5.00> "%regfile%"
echo.>> "%regfile%"
echo [HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Lxss\{%md5guid%}]>> "%regfile%"

type "%batdir%\WSL-green.reg.0" >> "%regfile%" 
echo "BasePath"="%batdir2%" >> "%regfile%"
echo "DistributionName"="%ThisWslDistributionName%" >> "%regfile%"


REM =========== IMPORTING REGISTRY ==============

call :Echos Registering WSL instance using DistributionName: %ThisWslDistributionName%

call :EchoAndExec reg import "%regfile%"
if not %ERRORLEVEL%==0 (
    call :Echos Import registry file fail: %regfile%
    pause
    exit /b 4
)

title WSL - %ThisWslDistributionName%

call :EchoAndExec wsl -d %ThisWslDistributionName% %_u_param% --cd ~
if not %ERRORLEVEL%==0 (
    echo.
    echo "wsl -d" execution fail. Please check your bat file or Windows environment.
    echo.
    pause
    exit /b 4
)

exit /b 0


REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  REM This function preserves %ERRORLEVEL% for the caller,
  REM and, LastError does NOT pollute the caller.
  setlocal & set LastError=%ERRORLEVEL%
  echo [%batfilenam%] %*
exit /b %LastError%

:EchoAndExec
  echo [%batfilenam%] EXEC: %*
  call %*
exit /b %ERRORLEVEL%

:HasSpaceChar
  REM Check if the input params has space-char in it.
  REM
  REM call :HasSpaceChar param1 param2
  REM call :HasSpaceChar "param1 param2"
  REM      -- ERRORLEVEL above will be 1, means yes.
  REM
  REM call :HasSpaceChar ans param1-param2
  REM      -- ERRORLEVEL above will be 0, means no.
  REM
  REM If no input param, consider it "have" spacechar.
  
  setlocal & set input=%*
  if not defined input exit /b 1
  
  set input=%input:"=#%
  set count=0
  for %%i in (%input%) do (
    set /A count=count+1
  )
  if %count% == 1 ( 
    exit /b 0 
  ) else (
    exit /b 1
  )

:GetFileMD5
REM Thanks to: https://github.com/npocmaka/batch.scripts/blob/master/fileUtils/md5.bat
REM Usage:
REM   call :GetFileMD5 <file> <outvar>
  set "md5="
  for /f "skip=1 tokens=* delims=" %%# in ('certutil -hashfile "%~f1" MD5') do (
	if not defined md5 (
		for %%Z in (%%#) do set "md5=!md5!%%Z"
    )
  )
  endlocal && (set "%~2=%md5%")
