@echo off
@setlocal

REM Use this .bat files to backup chja20 Windows PC.
REM Backup archive is generated in the directory of this bat file.

set batdir=%~dp0
set batdir=%batdir:~0,-1%
set batdir_fs=%batdir:\=/%

pushd %batdir%

@echo on
wsl.exe './do-irsync.sh'
