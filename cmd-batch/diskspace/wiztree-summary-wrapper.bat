@echo OFF
setlocal EnableDelayedExpansion
set batfilenam=%~nx0
set batdir=%~dp0
set batdir=%batdir:~0,-1%


REM ===== user params to customize: =====

rem Add dirs to PATH so that these EXEs can be found: cat.exe, sed.exe, WizTree.exe
PATH=C:\Program Files\Git\usr\bin;%PATH%

rem Dark yellow background, dark blue text
color 61

REM ===== customization done =====


call "%batdir%\wiztree-summary.bat"

