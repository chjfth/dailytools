@echo off
@setlocal
REM [2020-08-15] On chja20 Win10, I create a start-up shortcut to run this rsync daemon after logon to my desktop.

set batdir=%~dp0
set batdir=%batdir:~0,-1%
set batdir_fs=%batdir:\=/%
pushd %batdir%

set MYCMD=C:\Windows\System32\wsl.exe rsync --daemon --port=1873 --config=wsl-rsyncd.conf --no-detach
echo %MYCMD%
%MYCMD%

