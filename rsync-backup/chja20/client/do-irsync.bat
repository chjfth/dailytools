@echo off
@setlocal
set batdir=%~dp0
set batdir=%batdir:~0,-1%
pushd %batdir%

PATH=C:\cygwin64\bin;%PATH%

bash -c './do-irsync.sh'
