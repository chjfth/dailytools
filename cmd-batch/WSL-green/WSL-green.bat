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

set wsl1_checkdir=%batdir%\rootfs\root
set wsl1_seen=
if exist "%wsl1_checkdir%" set wsl1_seen=1

set wsl2_checkfile=%batdir%\ext4.vhdx
set wsl2_seen=
if exist "%wsl2_checkfile%" set wsl2_seen=1

if "%wsl1_seen%%wsl2_seen%" == "" (
	call :Echos Error: No WSL file-system found in "%batdir%" .
	call :Echos You should either have 
	call :Echos     "%wsl1_checkdir%" ^(for WSL1^)
	call :Echos or 
	call :Echos     "%wsl2_checkfile%" ^(for WSL2^)
	exit /b 4
)

REM =========== PREPARE .REG FILE ==============

REM We need to generate a GUID for current DistributionName,
REM We can ensure it being unique by calcuating %ThisWslDistributionName%.
echo %ThisWslDistributionName%> "%batdir%\wslname.tmp"

call :GetFileMD5 "%batdir%\wslname.tmp" md5

if not defined md5 (
	call :Echos Error: Cannot generate a GUID for ThisWslDistributionName.
	exit /b 4
)

set md5guid=%md5:~0,8%-%md5:~8,4%-%md5:~12,4%-%md5:~16,4%-%md5:~20,12%
rem For "Ubuntu-22.04", md5guid is
rem     2b0c8e79-b4a5-1bfc-49cd-4f344cbfe444

set md5finalchar=%md5:~31,1%
if not defined md5finalchar (
	call :Echos Error: When generating MD5 for ThisWslDistributionName, certttil.exe returns expected result.
	call :Echos md5=%md5%
	exit /b 4
)

set badoutput=%md5:~32,1%
if defined badoutput (
	call :Echos Error: When generating MD5 for ThisWslDistributionName, certttil.exe returns expected result.
	call :Echos md5=%md5%
	exit /b 4
)

set regfile=%batdir%\WSL-green.reg

echo Windows Registry Editor Version 5.00> "%regfile%"
echo.>> "%regfile%"
echo [HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Lxss\{%md5guid%}]>> "%regfile%"

type "%batdir%\WSL-green.reg.0" >> "%regfile%" 

if defined wsl1_seen (
	echo "Flags"=dword:00000007 >> "%regfile%"
	set wslverhint=WSL1
) else if defined wsl2_seen (
	echo "Flags"=dword:0000000f >> "%regfile%"
	set wslverhint=WSL2
)

echo "BasePath"="%batdir2%" >> "%regfile%"
echo "DistributionName"="%ThisWslDistributionName%" >> "%regfile%"


REM =========== IMPORTING REGISTRY ==============

call :Echos Registering WSL instance using:
call :Echos .   WSL version      = %wslverhint%
call :Echos .   GUID             = {%md5guid%}
call :Echos .   DistributionName = %ThisWslDistributionName%

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
