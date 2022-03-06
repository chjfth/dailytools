setlocal EnableDelayedExpansion

: This is a function.
: It delete a file and correctly reports ERRORLEVEL
: Thanks to: https://stackoverflow.com/a/33403497/151453

:DeleteOneFile

: Param1: the filepath to delete

if not exist "%~1" exit /b 0

> nul ver & for /F "tokens=*" %%# in ('del /Q "%~1" 2^>^&1 1^> nul') do (2> nul set =)

exit /b %ERRORLEVEL%
