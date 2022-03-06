@echo off
copy nosuchfile.txt 0.txt
if ERRORLEVEL 1 goto :END 

exit /b 0

:END
