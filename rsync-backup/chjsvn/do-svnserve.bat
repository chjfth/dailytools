@echo off
REM After the svn repos have been synced to this folder, I can double-click this do-svnserve.bat
REM to start up an fast svn server for PCs on my home LAN.
setlocal
set batdir=%~dp0%
set batdir=%batdir:~0,-1%

echo SVN client hint:   
echo   svn://10.22.3.84/software_vmwin
echo   svn://10.22.3.84/chjbooks/trunk
echo.

echo SVN server starting...
set command=svnserve -d -r %batdir%
echo   %command%
%command%
