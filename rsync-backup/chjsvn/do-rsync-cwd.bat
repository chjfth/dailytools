@echo off
setlocal

REM ========= wsl.exe requires Win10.1607+ =========

set batdir=%~dp0
set batdir=%batdir:~0,-1%
set batdir_fs=%batdir:\=/%
pushd %batdir%

set BACKUP_TARGET_DIR=/mnt/i/auto-backups

set MYCMD=C:\Windows\System32\wsl.exe ./do-rsync.sh %BACKUP_TARGET_DIR%
echo %MYCMD%
%MYCMD%

