@echo off
@setlocal

REM ~~~~~~~~~~~~~~~~~~~~~~~~~~
REM Use this .bat files to backup and chja20 Windows PC.
REM At client side, even on Windows 10 with WSL, using cygwin is suggested.
REM And I suggest launching it via dosched0.bat via Windows Task Scheduler.
REM ~~~~~~~~~~~~~~~~~~~~~~~~~~

set batdir=%~dp0
set batdir=%batdir:~0,-1%
pushd %batdir%

PATH=C:\cygwin64\bin;%PATH%

bash -c './do-irsync.sh'
