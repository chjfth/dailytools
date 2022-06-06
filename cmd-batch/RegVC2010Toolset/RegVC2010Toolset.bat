@echo off
setlocal EnableDelayedExpansion

REM ===USAGE====
echo This .bat registers MSBuild 4.0, VC++ 2010 compiler and Win7SDK into your system,
echo so that VS2019 IDE can recognize VC2010 vcxproj(s) and invoke VC2010 compiler for them.
echo To use it, you need to prepare a RegVC2010Toolset.ini telling related directory locations.
echo.                                        -- presented by Jimm Chen, 2022.05

REM set batfilenam to .bat filename(no directory prefix)
set batfilenam=%~n0%~x0
set batdir=%~dp0
set batdir=%batdir:~0,-1%
set dirfunc=%batdir%\batfunc

call "%dirfunc%\GetParentDir.bat" dirNLS "%batdir%"
set _vspgINDENTS=

set isWarnExist=

REM ==== Set some const vars

set regkeyVCTargetsPath=HKLM\SOFTWARE\Microsoft\MSBuild\ToolsVersions\4.0
set regitmVCTargetsPath=VCTargetsPath
set regvalVCTargetsPath=%batdir%\v4.0
rem
set regkeyVCProductDir=HKLM\SOFTWARE\Microsoft\VisualStudio\10.0\Setup\VC
set regitmVCProductDir=ProductDir
set regvalVCProductDir=%dirNLS%\compilers\VS2010\VC
rem
set regkeyVSProductDir=HKLM\SOFTWARE\Microsoft\VisualStudio\10.0\Setup\VS
set regitmVSProductDir=ProductDir
set regvalVSProductDir=%dirNLS%\compilers\VS2010
rem
set regkeyWinsdkInstallationFolder=HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.0A
set regitmWinsdkInstallationFolder=InstallationFolder
set regvalWinsdkInstallationFolder=%dirNLS%\compilers\VS2010
rem
set regkeyWinsdkCurrentInstallFolder=HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows
set regitmWinsdkCurrentInstallFolder=CurrentInstallFolder
set regvalWinsdkCurrentInstallFolder=%dirNLS%\compilers\VS2010
rem
rem == vc90(2008) below
set regkeyVCProductDir90=HKLM\SOFTWARE\Microsoft\VisualStudio\9.0\Setup\VC
set regvalVCProductDir90=%dirNLS%\compilers\VS2008\VC
rem
set regkeyVSProductDir90=HKLM\SOFTWARE\Microsoft\VisualStudio\9.0\Setup\VS
set regvalVSProductDir90=%dirNLS%\compilers\VS2008
rem
set regkeyWinsdkInstallationFolder90=HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v6.0A
set regvalWinsdkInstallationFolder90=%dirNLS%\compilers\MsSDK2008

set gacutilexe=%regvalWinsdkCurrentInstallFolder%\Bin\NETFX 4.0 Tools\gacutil.exe


REM ==== Reading related regitems, so to tell whether the "Register" operation had been applied.

call :CheckOneRegItem VCTargetsPath "%regkeyVCTargetsPath%" "%regitmVCTargetsPath%"
call :CheckOneRegItem VCProductDir  "%regkeyVCProductDir%"  "%regitmVCProductDir%"
call :CheckOneRegItem VSProductDir  "%regkeyVSProductDir%"  "%regitmVSProductDir%"
call :CheckOneRegItem WinsdkInstallationFolder    "%regkeyWinsdkInstallationFolder%"    "%regitmWinsdkInstallationFolder%"
rem
call :CheckOneRegItem WinsdkCurrentInstallFolder  "%regkeyWinsdkCurrentInstallFolder%"  "%regitmWinsdkCurrentInstallFolder%"
rem
call :CheckOneRegItem VCProductDir90 "%regkeyVCProductDir90%" "%regitmVCProductDir%"
call :CheckOneRegItem VSProductDir90 "%regkeyVSProductDir90%" "%regitmVSProductDir%"
call :CheckOneRegItem WinsdkInstallationFolder90  "%regkeyWinsdkInstallationFolder90%" "%regitmWinsdkInstallationFolder%"

echo.

if defined isWarnExist (
	call :Echos According to regitems^(32bit-view^) above, MSBuild 4.0 seems to have been registered.
	choice /m "Would you like to redo/overwrite it?"
	if "!ERRORLEVEL!"=="2" exit /b 2
)

REM ==== If run from Chj's NLS-Build-Env, just diretly goto TAKE_ACTION.
set isChjCase=
if exist "%batdir%\v4.0\Microsoft.Build.CPPTasks.Common.dll" (
	set isChjCase=1
	goto :TAKE_ACTION
)

REM ---- Now the general case, read those directory info from INI.
set inifilenam=%batfilenam:.bat=.ini%
set inifilepath=%batdir%\%inifilenam%

if not exist "%inifilepath%" (
	call :Echos [ERROR] Not found "%inifilepath%". VC2010 related directories should be configured in that ini.
	call :Echos You can created one from %inifilenam%.sample along-side this .bat file.
	exit /b 4
)

call :GetCfgDir regvalVCTargetsPath            msbuildv4 Microsoft.Build.CPPTasks.Common.dll
if errorlevel 1 exit /b 4

call :GetCfgDir regvalVSProductDir             vs2010 "VC\bin\cl.exe"
if errorlevel 1 exit /b 4

set regvalVCProductDir=%regvalVSProductDir%\VC

call :GetCfgDir regvalWinsdkInstallationFolder winsdk7 "include\Windows.h"
if errorlevel 1 exit /b 4

set regvalWinsdkCurrentInstallFolder=%regvalWinsdkInstallationFolder%

set gacutilexe=%regvalWinsdkInstallationFolder%\Bin\NETFX 4.0 Tools\gacutil.exe
if not exist "%gacutilexe%" (
	call :Echos [ERROR] Not found executable: "%gacutilexe%"
	exit /b 4
)

REM ---- Check that msvcr100.dll and msvcp100.dll have been along side with cl.exe . ----
set isMissingDLL=
call :CheckMSVC100DLL msvcr100.dll
if errorlevel 1 set isMissingDLL=1
call :CheckMSVC100DLL msvcp100.dll
if errorlevel 1 set isMissingDLL=1

if defined isMissingDLL (
	call :Echos [ERROR] The above mentioned DLL^(s^) is missing, you should copy them from a normally-installed VS2010 machine so that CL.EXE can run on any machine greenly. The can be found there in C:\Windows\SysWOW64 .
	exit /b 4
)


REM ~~~~~~ Env checking done. Do real action below ~~~~~~

:TAKE_ACTION

call "%dirfunc%\IsRunAsAdmin.bat"
if errorlevel 1 (
	call :Echos [ERROR] To continue, %batfilenam% must be run as Administrator.
	exit /b 4
)

call :WriteOneRegItem "%regkeyVCTargetsPath%" "%regitmVCTargetsPath%" "%regvalVCTargetsPath%\"
if errorlevel 1 exit /b 4

call :WriteOneRegItem "%regkeyVCProductDir%"  "%regitmVCProductDir%"  "%regvalVCProductDir%\"
if errorlevel 1 exit /b 4

call :WriteOneRegItem "%regkeyVSProductDir%"  "%regitmVSProductDir%"  "%regvalVSProductDir%\"
if errorlevel 1 exit /b 4

call :WriteOneRegItem "%regkeyWinsdkInstallationFolder%"    "%regitmWinsdkInstallationFolder%"    "%regvalWinsdkInstallationFolder%\"
if errorlevel 1 exit /b 4

call :WriteOneRegItem "%regkeyWinsdkCurrentInstallFolder%"  "%regitmWinsdkCurrentInstallFolder%"  "%regvalWinsdkCurrentInstallFolder%\"
if errorlevel 1 exit /b 4

if defined isChjCase (
	REM Do VC2008 related stuff for chj NLS-Build-env only.
	
	call :WriteOneRegItem "%regkeyVCProductDir90%" "%regitmVCProductDir%" "%regvalVCProductDir90%\"
	if errorlevel 1 exit /b 4

	call :WriteOneRegItem "%regkeyVSProductDir90%" "%regitmVSProductDir%" "%regvalVSProductDir90%\"
	if errorlevel 1 exit /b 4

	call :WriteOneRegItem "%regkeyWinsdkInstallationFolder90%" "%regitmWinsdkInstallationFolder%" "%regvalWinsdkInstallationFolder90%\"
	if errorlevel 1 exit /b 4
)

echo.
REM ==== Set Envvar VCTargetsPath10 so to avoid error exeucting MSBuild.exe from within VS2019 IDE.

call :EchoAndExec setx VCTargetsPath10 "%regvalVCTargetsPath%\\"
if errorlevel 1 (
	call :Echos [ERROR] %cmdexe%
	exit /b 4
)

REM ==== Register 3 dll into GAC

call :EchoAndExec "%gacutilexe%" /nologo /i "%regvalVCTargetsPath%\Microsoft.Build.CPPTasks.Common.dll"
if errorlevel 1 exit /b 4

call :EchoAndExec "%gacutilexe%" /nologo /i "%regvalVCTargetsPath%\Platforms\Win32\Microsoft.Build.CPPTasks.Win32.dll"
if errorlevel 1 exit /b 4

if exist "%regvalVCTargetsPath%\Platforms\x64\Microsoft.Build.CPPTasks.x64.dll" (
	REM VC2010 Express does not have x64 compiler.
	call :EchoAndExec "%gacutilexe%" /nologo /i "%regvalVCTargetsPath%\Platforms\x64\Microsoft.Build.CPPTasks.x64.dll"
	if errorlevel 1 exit /b 4
)

echo.
echo [[[[ SUCCESS ]]]]

exit /b 0


REM =============================
REM ====== Functions Below ======
REM =============================

:Echos
  REM This function preserves %ERRORLEVEL% for the caller,
  REM and, LastError does NOT pollute the caller.
  setlocal & set LastError=%ERRORLEVEL%
  echo.%*
exit /b %LastError%

:EchoAndExec
  echo.EXEC: %*
  call %*
exit /b %ERRORLEVEL%

:SetErrorlevel
  REM Usage example:
  REM call :SetErrorlevel 4
exit /b %1


:CheckOneRegItem
REM Params: same spec as RegQuerySz.bat
	call "%dirfunc%\RegQuerySz.bat" %~1 "%~2" "%~3"
	if defined %~1 (
		set isWarnExist=1
		call :Echos [%~2]
		call :Echos .   %~1 = !%~1!
	)
exit /b 0

:WriteOneRegItem
REM Usage example:
REM call :WriteOneRegItem <regkey> <regitemname> <regitemvalue>
REM ERRORLEVEL tells success or not.
	setlocal EnableDelayedExpansion
	
	call :Echos [%~1%]
	call :Echos .   %~2 = %~3
	
	REM Special: If trailing char is \ , we must double it.
	set regval=%~3
	if "%regval:~-1%" == "\" (
		set regval=!regval!\
	)
	
	reg add "%~1" /v "%~2" /t REG_SZ /d "%regval%" /f /reg:32 > nul
	if errorlevel 1 (
		call :Echos .   ^(Regitem Write ERROR!^)
		exit /b 4
	)
	call :Echos .   ^(Regitem Write OK.^)
exit /b 0


:GetCfgDir
REM Usage: 
REM 	call :GetCfgDir <outputvar> <ini-item-name> <subpath-to-check-exist>
REM
REM Example:
REM 	call :GetCfgDir dirVarname msbuildv4 Microsoft.Build.CPPTasks.Common.dll
REM 
REM dirVarname returns the dirpath in INI.
	setlocal EnableDelayedExpansion
	for /f "delims=" %%a in ('call "%dirfunc%\paxreadini.bat" "%inifilepath%" dirs %~2') do (
    	set retdir=%%a
	)
	
	if "%retdir%"=="" (
		call :Echos [ERROR] "%inifilepath%" missing [dirs] %~2= .
		exit /b 4
	)
	
	if not exist "%retdir%\%~3" (
		call :Echos [ERROR] "%~2=" is assigned a bad dir, bcz "%retdir%\%~3" does not exist.
		exit /b 4
	)

	endlocal & (
		set "%~1=%retdir%"
	)
exit /b 0


:CheckMSVC100DLL 
REM Param: DLL filename to check
REM exit 0 mean success, otherwise fail
	setlocal EnableDelayedExpansion
	set dllfilepath=%regvalVCProductDir%\bin\%~1
	if not exist "%dllfilepath%" (
		call :Echos Not found DLL: "%dllfilepath%"
		exit /b 4
	)
exit /b 0
