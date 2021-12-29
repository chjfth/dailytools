@echo off
setlocal EnableDelayedExpansion
REM This bat file boots actual VSPG bat files. It should be kept minimalist. 
REM This file exists to ease VSPG developer to temporarily switch local [boots] dir to [boots-dev].
REM so that a new VSPG version can be developed in side-by-side with the actual project using VSPG.
REM
REM Operation hint: Create a soft link boots-dev pointing to VSPG original source.
REM
REM 	cd D:\some\big work\_VSPG
REM 	if exists ".\boots\VSPG-Boots.bat" (echo Condition OK.)
REM 	mklink /j boots-dev "D:\gitw\dailytools\cmd-batch\vsproj-VSPG\_VSPG\boots"
REM 	
REM Now, D:\some\big work\_VSPG\boots\VSPG-Boots.bat will call content in
REM      D:\some\big work\_VSPG\boots-dev\VSPG-Boots.bat instead.

REM set batfilenam to .bat filename(no directory prefix)
set batfilenam=%~n0%~x0
set _vspg_bootsdir=%~dp0
set _vspg_bootsdir=%_vspg_bootsdir:~0,-1%
REM

call "%_vspg_bootsdir%\PathSplit.bat" "%_vspg_bootsdir%" ParentDir Subdir

set "_vspg_userbatdir=%ParentDir%"


if exist "%ParentDir%\boots-dev\VSPG-StartBat9_.bat" (
	REM Override _vspg_bootsdir to be the -dev one.
	set "_vspg_bootsdir=%ParentDir%\boots-dev"
)

call "%_vspg_bootsdir%\VSPG-StartBat9_.bat" %*
